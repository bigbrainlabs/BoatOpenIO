// ============================================================
// BoatOpenIO – Firmware v2.2
// Universal Marine IO System
// github.com/bigbrainlabs/BoatOpenIO
// ============================================================

#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <esp_task_wdt.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADS1X15.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <Preferences.h>

// ── PINS ────────────────────────────────────────────────────
#define MUX_S0           14
#define MUX_S1           27
#define MUX_S2           26
#define MUX_S3           25
#define SDA_PIN          21
#define SCL_PIN          22
#define STATUS_LED_PIN    2
#define RESET_BUTTON_PIN  0

// ── KONSTANTEN ──────────────────────────────────────────────
#define CONFIG_FILE      "/config.json"
#define AP_DEFAULT_SSID  "BoatOpenIO-Setup"
#define AP_DEFAULT_PASS  "boatopenio"
#define MQTT_CLIENT_ID   "BoatOpenIO"
#define MQTT_TOPIC_BASE  "boat/io/"
#define MQTT_STATUS      "boatopenio/status"
#define READ_INTERVAL    2000
#define IMU_INTERVAL      200
#define STATUS_INTERVAL  10000
#define WDT_TIMEOUT        10

// ── NETZWERK CONFIG ─────────────────────────────────────────
char wifi_ssid[40]   = "";
char wifi_pass[64]   = "";
char mqtt_server[40] = "192.168.1.100";
char mqtt_port[6]    = "1883";
char mqtt_user[40]   = "";
char mqtt_pass[40]   = "";
// AP & Portal-Credentials (in NVS gespeichert, beim ersten Boot leer)
char ap_ssid[32]     = AP_DEFAULT_SSID;
char ap_pass[32]     = AP_DEFAULT_PASS;
char portal_user[32] = "admin";
char portal_pass[64] = "";   // leer = Ersteinrichtung noch nicht abgeschlossen

// ── OBJEKTE ─────────────────────────────────────────────────
Adafruit_ADS1115 ads[4];   // ads[0]=0x48 … ads[3]=0x4B
Adafruit_MPU6050 mpu;
WiFiClient       wifiClient;
PubSubClient     mqtt(wifiClient);
WebServer        webServer(80);
Preferences      preferences;

// ── KONFIGURATION ───────────────────────────────────────────
struct KanalConfig {
  uint8_t klemme;
  uint8_t ads;
  uint8_t pin;
  String  sensor;
  String  einheit;
  float   faktor;
  float   offset;
  bool    aktiv;
};
KanalConfig kanaele[16];
bool testMode = true;
bool mpuOK    = false;
bool adsOK[4] = {false, false, false, false};

// ── IMU / IMPACT ─────────────────────────────────────────────
float         pitch = 0.0f, roll = 0.0f;
unsigned long lastIMUMicros  = 0;
float         impactPeakG    = 0.0f;
unsigned long lastImpactTime = 0;
bool          impactActive   = false;

const float        COMP_ALPHA       = 0.98f;
const float        IMPACT_THRESHOLD = 0.5f;
const unsigned long IMPACT_DECAY   = 8000;

// Gyro-Bias (automatisch beim Boot gemessen)
float gyroBiasX = 0.0f, gyroBiasY = 0.0f, gyroBiasZ = 0.0f;
// Montage-Offset (via Web-UI kalibriert, in NVS gespeichert)
float pitch_offset = 0.0f, roll_offset = 0.0f;

// ── TIMING ──────────────────────────────────────────────────
unsigned long lastChannelRead = 0;
unsigned long lastIMURead     = 0;
unsigned long lastStatus      = 0;

// ── MUX ─────────────────────────────────────────────────────
void selectChannel(uint8_t ch) {
  digitalWrite(MUX_S0,  ch        & 0x01);
  digitalWrite(MUX_S1, (ch >> 1)  & 0x01);
  digitalWrite(MUX_S2, (ch >> 2)  & 0x01);
  digitalWrite(MUX_S3, (ch >> 3)  & 0x01);
  delayMicroseconds(10);
}

// ── ADS LESEN ───────────────────────────────────────────────
float readADS(uint8_t adsNr, uint8_t pin) {
  if (adsNr < 1 || adsNr > 4 || !adsOK[adsNr - 1]) return 0.0f;
  int16_t raw = ads[adsNr - 1].readADC_SingleEnded(pin);
  return ads[adsNr - 1].computeVolts(raw);
}

// ── KANÄLE LESEN & PUBLISHEN ─────────────────────────────────
void readAllChannels() {
  for (int i = 0; i < 16; i++) {
    if (!kanaele[i].aktiv) continue;
    float voltage;
    if (testMode) {
      voltage = 2.5f + sinf(millis() / 3000.0f + i) * 0.3f;
    } else {
      selectChannel(kanaele[i].klemme - 1);
      delayMicroseconds(500);
      voltage = readADS(kanaele[i].ads, kanaele[i].pin);
    }
    float wert = voltage * kanaele[i].faktor + kanaele[i].offset;
    char buf[16];
    dtostrf(wert, 6, 2, buf);
    mqtt.publish((String(MQTT_TOPIC_BASE) + kanaele[i].sensor).c_str(), buf);
  }
}

// ── IMPACT ───────────────────────────────────────────────────
const char* impactSeverity(float g) {
  if (g < 1.0f) return "LEICHT";
  if (g < 3.0f) return "MITTEL";
  if (g < 6.0f) return "STARK";
  return "KRITISCH";
}

void publishImpact() {
  static unsigned long lastPub = 0;
  if (millis() - lastPub < 1000) return;
  lastPub = millis();
  char buf[10];
  dtostrf(impactPeakG, 5, 2, buf);
  mqtt.publish("boat/alarm/impact_g",        buf, true);
  mqtt.publish("boat/alarm/impact_severity", impactSeverity(impactPeakG), true);
  mqtt.publish("boat/alarm/impact_active",   "true", true);
  Serial.printf("AUFPRALL: %.2fg -> %s\n", impactPeakG, impactSeverity(impactPeakG));
}

// ── IMU (Komplementärfilter + Impact) ────────────────────────
void readIMU() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  unsigned long nowUs = micros();
  float dt = (lastIMUMicros == 0) ? 0.01f : (nowUs - lastIMUMicros) / 1e6f;
  if (dt > 1.0f) dt = 0.01f;
  lastIMUMicros = nowUs;

  float aPitch = atan2f(a.acceleration.x,
    sqrtf(a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * 180.f / PI;
  float aRoll = atan2f(a.acceleration.y,
    sqrtf(a.acceleration.x * a.acceleration.x + a.acceleration.z * a.acceleration.z)) * 180.f / PI;

  pitch = COMP_ALPHA * (pitch + (g.gyro.y - gyroBiasY) * dt * 180.f / PI) + (1.f - COMP_ALPHA) * aPitch;
  roll  = COMP_ALPHA * (roll  + (g.gyro.x - gyroBiasX) * dt * 180.f / PI) + (1.f - COMP_ALPHA) * aRoll;

  float mag = sqrtf(a.acceleration.x * a.acceleration.x +
                    a.acceleration.y * a.acceleration.y +
                    a.acceleration.z * a.acceleration.z);
  float exG = mag / 9.81f - 1.0f;

  if (exG > IMPACT_THRESHOLD) {
    lastImpactTime = millis();
    if (!impactActive) impactActive = true;
    if (exG > impactPeakG) impactPeakG = exG;
    publishImpact();
  } else if (impactActive && millis() - lastImpactTime > IMPACT_DECAY) {
    impactActive = false;
    impactPeakG  = 0.0f;
    mqtt.publish("boat/alarm/impact_active", "false", true);
  }
}

// ── FAKE IMU ─────────────────────────────────────────────────
void fakeIMU() {
  static float pa = 0.0f, ra = 0.0f;
  pa += 0.004f; ra += 0.006f;
  pitch = sinf(pa) * 3.5f + sinf(pa * 2.1f) * 1.2f;
  roll  = sinf(ra) * 6.0f + sinf(ra * 1.8f) * 2.5f;

  static unsigned long nextFake = 45000UL;
  if (millis() > nextFake) {
    nextFake       = millis() + 30000UL + (unsigned long)random(0, 60000);
    impactPeakG    = 0.6f + random(0, 35) / 10.0f;
    impactActive   = true;
    lastImpactTime = millis();
    publishImpact();
  }
  if (impactActive && millis() - lastImpactTime > IMPACT_DECAY) {
    impactActive = false;
    impactPeakG  = 0.0f;
    mqtt.publish("boat/alarm/impact_active", "false", true);
  }
}

// ── KONFIGURATION LADEN / SPEICHERN ─────────────────────────
void loadConfig() {
  for (int i = 0; i < 16; i++) {
    kanaele[i] = {(uint8_t)(i + 1), (uint8_t)(i / 4 + 1), (uint8_t)(i % 4),
                  "kanal_" + String(i + 1), "V", 1.0f, 0.0f, false};
  }
  if (!LittleFS.exists(CONFIG_FILE)) return;
  File f = LittleFS.open(CONFIG_FILE, "r");
  StaticJsonDocument<4096> doc;
  if (deserializeJson(doc, f) != DeserializationError::Ok) { f.close(); return; }
  f.close();
  JsonArray arr = doc["kanaele"].as<JsonArray>();
  int idx = 0;
  for (JsonObject k : arr) {
    if (idx >= 16) break;
    kanaele[idx].klemme  = k["klemme"]  | (idx + 1);
    kanaele[idx].ads     = k["ads"]     | 1;
    kanaele[idx].pin     = k["pin"]     | 0;
    kanaele[idx].sensor  = k["sensor"].isNull()  ? "kanal_" + String(idx + 1) : k["sensor"].as<String>();
    kanaele[idx].einheit = k["einheit"].isNull() ? "V"                         : k["einheit"].as<String>();
    kanaele[idx].faktor  = k["faktor"]  | 1.0f;
    kanaele[idx].offset  = k["offset"]  | 0.0f;
    kanaele[idx].aktiv   = k["aktiv"]   | false;
    idx++;
  }
}

void saveConfig() {
  StaticJsonDocument<4096> doc;
  JsonArray arr = doc.createNestedArray("kanaele");
  for (int i = 0; i < 16; i++) {
    JsonObject k = arr.createNestedObject();
    k["klemme"]  = kanaele[i].klemme;
    k["ads"]     = kanaele[i].ads;
    k["pin"]     = kanaele[i].pin;
    k["sensor"]  = kanaele[i].sensor;
    k["einheit"] = kanaele[i].einheit;
    k["faktor"]  = kanaele[i].faktor;
    k["offset"]  = kanaele[i].offset;
    k["aktiv"]   = kanaele[i].aktiv;
  }
  File f = LittleFS.open(CONFIG_FILE, "w");
  serializeJson(doc, f);
  f.close();
}

void loadNetConfig() {
  preferences.begin("boatopenio", true);
  String ss  = preferences.getString("wifi_ssid",  "");
  String sp  = preferences.getString("wifi_pass",  "");
  String ms  = preferences.getString("mqtt_ip",    "192.168.1.100");
  String mp  = preferences.getString("mqtt_port",  "1883");
  String mu  = preferences.getString("mqtt_user",  "");
  String mpw = preferences.getString("mqtt_pass",  "");
  testMode     = preferences.getBool("testmode",  true);
  pitch_offset = preferences.getFloat("pitch_off", 0.0f);
  roll_offset  = preferences.getFloat("roll_off",  0.0f);
  String aps  = preferences.getString("ap_ssid",    AP_DEFAULT_SSID);
  String app  = preferences.getString("ap_pass",    AP_DEFAULT_PASS);
  String pu   = preferences.getString("portal_user","admin");
  String pp   = preferences.getString("portal_pass","");
  preferences.end();
  ss.toCharArray(wifi_ssid,   sizeof(wifi_ssid));
  sp.toCharArray(wifi_pass,   sizeof(wifi_pass));
  ms.toCharArray(mqtt_server, sizeof(mqtt_server));
  mp.toCharArray(mqtt_port,   sizeof(mqtt_port));
  mu.toCharArray(mqtt_user,   sizeof(mqtt_user));
  mpw.toCharArray(mqtt_pass,  sizeof(mqtt_pass));
  aps.toCharArray(ap_ssid,     sizeof(ap_ssid));
  app.toCharArray(ap_pass,     sizeof(ap_pass));
  pu.toCharArray(portal_user,  sizeof(portal_user));
  pp.toCharArray(portal_pass,  sizeof(portal_pass));
}

void saveNetConfig() {
  preferences.begin("boatopenio", false);
  preferences.putString("wifi_ssid",  wifi_ssid);
  preferences.putString("wifi_pass",  wifi_pass);
  preferences.putString("mqtt_ip",    mqtt_server);
  preferences.putString("mqtt_port",  mqtt_port);
  preferences.putString("mqtt_user",  mqtt_user);
  preferences.putString("mqtt_pass",  mqtt_pass);
  preferences.putBool("testmode",     testMode);
  preferences.putFloat("pitch_off",    pitch_offset);
  preferences.putFloat("roll_off",     roll_offset);
  preferences.putString("ap_ssid",     ap_ssid);
  preferences.putString("ap_pass",     ap_pass);
  preferences.putString("portal_user", portal_user);
  preferences.putString("portal_pass", portal_pass);
  preferences.end();
}

// ── IMU GYRO-KALIBRIERUNG (Boot) ─────────────────────────────
void setupIMUCalibration() {
  if (!mpuOK) return;
  Serial.print("  Gyro-Bias messen (0.5s still halten)...");
  float sx = 0, sy = 0, sz = 0;
  const int N = 100;
  for (int i = 0; i < N; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    sx += g.gyro.x; sy += g.gyro.y; sz += g.gyro.z;
    delay(5);
  }
  gyroBiasX = sx / N; gyroBiasY = sy / N; gyroBiasZ = sz / N;
  Serial.printf(" X=%.4f Y=%.4f Z=%.4f rad/s\n", gyroBiasX, gyroBiasY, gyroBiasZ);
}

// ── WIFI VERBINDEN (non-blocking mit Timeout) ────────────────
void connectWiFi() {
  if (strlen(wifi_ssid) == 0) {
    Serial.println("Kein WiFi konfiguriert – nur AP aktiv");
    return;
  }
  Serial.printf("WiFi verbinden mit: %s ...\n", wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_pass);
  unsigned long t = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t < 10000) {
    delay(300);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi verbunden: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nWiFi Verbindung fehlgeschlagen (AP bleibt aktiv)");
    WiFi.disconnect();
  }
}

// ── MQTT ────────────────────────────────────────────────────
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];

  if (String(topic) == "boatopenio/config") {
    StaticJsonDocument<4096> doc;
    if (deserializeJson(doc, msg) == DeserializationError::Ok) {
      JsonArray arr = doc["kanaele"].as<JsonArray>();
      int idx = 0;
      for (JsonObject k : arr) {
        if (idx >= 16) break;
        kanaele[idx].klemme  = k["klemme"]  | kanaele[idx].klemme;
        kanaele[idx].ads     = k["ads"]     | kanaele[idx].ads;
        kanaele[idx].pin     = k["pin"]     | kanaele[idx].pin;
        kanaele[idx].sensor  = k["sensor"].isNull()  ? kanaele[idx].sensor : k["sensor"].as<String>();
        kanaele[idx].einheit = k["einheit"].isNull() ? kanaele[idx].einheit : k["einheit"].as<String>();
        kanaele[idx].faktor  = k["faktor"]  | kanaele[idx].faktor;
        kanaele[idx].offset  = k["offset"]  | kanaele[idx].offset;
        kanaele[idx].aktiv   = k["aktiv"]   | kanaele[idx].aktiv;
        idx++;
      }
      saveConfig();
      mqtt.publish(MQTT_STATUS, "config_updated");
    }
  }
}

void reconnectMQTT() {
  if (WiFi.status() != WL_CONNECTED) return;
  static unsigned long lastAttempt = 0;
  if (millis() - lastAttempt < 5000) return;
  lastAttempt = millis();
  Serial.print("MQTT... ");
  bool ok = (strlen(mqtt_user) > 0)
    ? mqtt.connect(MQTT_CLIENT_ID, mqtt_user, mqtt_pass, MQTT_STATUS, 1, true, "offline")
    : mqtt.connect(MQTT_CLIENT_ID, MQTT_STATUS, 1, true, "offline");
  if (ok) {
    mqtt.publish(MQTT_STATUS, "online", true);
    mqtt.subscribe("boatopenio/config");
    mqtt.subscribe("boatopenio/cmd/#");
    Serial.println("verbunden");
  } else {
    Serial.printf("Fehler rc=%d\n", mqtt.state());
  }
}

// ── WEB-UI & CAPTIVE PORTAL (ausgelagert) ───────────────────
#include "webui.h"

// ── OTA ─────────────────────────────────────────────────────
void setupOTA() {
  ArduinoOTA.setHostname("BoatOpenIO");
  ArduinoOTA.setPassword(strlen(portal_pass) > 0 ? portal_pass : AP_DEFAULT_PASS);
  ArduinoOTA.onStart([]()  { Serial.println("OTA Start"); });
  ArduinoOTA.onEnd([]()    { Serial.println("\nOTA fertig"); });
  ArduinoOTA.onProgress([](unsigned int p, unsigned int t) {
    Serial.printf("OTA: %u%%\r", p / (t / 100));
  });
  ArduinoOTA.onError([](ota_error_t e) {
    Serial.printf("OTA Fehler[%u]\n", e);
  });
  ArduinoOTA.begin();
}

// ── SETUP ───────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n========================================");
  Serial.println("BoatOpenIO Firmware v2.2");
  Serial.println("========================================");

  pinMode(STATUS_LED_PIN,   OUTPUT);
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
  pinMode(MUX_S0, OUTPUT); pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT); pinMode(MUX_S3, OUTPUT);

  // Factory Reset: GPIO0 beim Boot 3s gedrückt halten
  if (digitalRead(RESET_BUTTON_PIN) == LOW) {
    Serial.println("Factory Reset – 3s halten...");
    delay(3000);
    if (digitalRead(RESET_BUTTON_PIN) == LOW) {
      preferences.begin("boatopenio", false);
      preferences.clear();
      preferences.end();
      LittleFS.begin(true);
      LittleFS.remove(CONFIG_FILE);
      Serial.println("Reset! Neustart...");
      delay(1000);
      ESP.restart();
    }
  }

  Wire.begin(SDA_PIN, SCL_PIN);
  LittleFS.begin(true);
  loadNetConfig();
  loadConfig();

  // I2C Scanner – alle Geräte auf dem Bus anzeigen
  Serial.println("I2C Scan:");
  for (uint8_t addr = 0x08; addr < 0x78; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0)
      Serial.printf("  Geraet @ 0x%02X\n", addr);
  }

  // ADS1115 (4x)
  const uint8_t ADS_ADDRS[4] = {0x48, 0x49, 0x4A, 0x4B};
  for (int i = 0; i < 4; i++) {
    if (ads[i].begin(ADS_ADDRS[i])) {
      ads[i].setGain(GAIN_TWOTHIRDS);
      adsOK[i] = true;
      Serial.printf("  ADS%d @ 0x%02X OK\n", i + 1, ADS_ADDRS[i]);
    } else {
      Serial.printf("  ADS%d @ 0x%02X fehlt\n", i + 1, ADS_ADDRS[i]);
    }
  }

  // MPU6050: 0x68 (AD0=GND) oder 0x69 (AD0=VCC)
  if (mpu.begin(0x68)) {
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_250_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    mpuOK = true;
    Serial.println("  MPU6050 @ 0x68 OK");
  } else if (mpu.begin(0x69)) {
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_250_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    mpuOK = true;
    Serial.println("  MPU6050 @ 0x69 OK (AD0=VCC)");
  } else {
    Serial.println("  MPU6050 nicht gefunden – Fake-IMU aktiv");
  }
  setupIMUCalibration();

  // WiFi: AP immer sofort starten (Credentials aus NVS)
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ap_ssid, ap_pass);
  Serial.printf("AP gestartet: %s | 192.168.4.1\n", ap_ssid);
  if (strlen(portal_pass) == 0)
    Serial.println("  !! Ersteinrichtung erforderlich – http://192.168.4.1/setup");

  // STA: falls Credentials vorhanden, verbinden (max 10s)
  connectWiFi();

  // MQTT
  mqtt.setServer(mqtt_server, atoi(mqtt_port));
  mqtt.setCallback(mqttCallback);
  mqtt.setBufferSize(512);

  // mDNS + OTA (nur wenn STA verbunden)
  if (WiFi.status() == WL_CONNECTED) {
    MDNS.begin("boatopenio");
    setupOTA();
    Serial.println("mDNS: http://boatopenio.local");
  }

  setupWebServer();

  // Watchdog
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  esp_task_wdt_config_t wdt_cfg = {
    .timeout_ms     = WDT_TIMEOUT * 1000,
    .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
    .trigger_panic  = true
  };
  esp_task_wdt_init(&wdt_cfg);
#else
  esp_task_wdt_init(WDT_TIMEOUT, true);
#endif
  esp_task_wdt_add(NULL);

  Serial.printf("Bereit! Modus: %s\n", testMode ? "TEST" : "LIVE");
  Serial.println("Web-UI: http://192.168.4.1");
  Serial.println("========================================\n");

  for (int i = 0; i < 3; i++) {
    digitalWrite(STATUS_LED_PIN, HIGH); delay(100);
    digitalWrite(STATUS_LED_PIN, LOW);  delay(100);
  }
}

// ── LOOP ────────────────────────────────────────────────────
void loop() {
  esp_task_wdt_reset();

  if (!mqtt.connected()) reconnectMQTT();
  mqtt.loop();
  if (WiFi.status() == WL_CONNECTED) ArduinoOTA.handle();
  loopWebServer();

  // GPIO0 Langdruck (3s) im Betrieb → WiFi-Credentials löschen & Neustart
  static unsigned long btnHeld = 0;
  if (digitalRead(RESET_BUTTON_PIN) == LOW) {
    if (!btnHeld) btnHeld = millis();
    if (millis() - btnHeld > 3000) {
      btnHeld = 0;
      Serial.println("Button: WiFi-Credentials loeschen...");
      preferences.begin("boatopenio", false);
      preferences.remove("wifi_ssid");
      preferences.remove("wifi_pass");
      preferences.end();
      delay(500);
      ESP.restart();
    }
  } else {
    btnHeld = 0;
  }

  // LED blinken
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink > 1000) {
    digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
    lastBlink = millis();
  }

  unsigned long now = millis();

  // IMU (200ms)
  if (now - lastIMURead >= IMU_INTERVAL) {
    lastIMURead = now;
    if (mpuOK) readIMU();
    else        fakeIMU();
    if (mqtt.connected()) {
      char buf[10];
      dtostrf(pitch - pitch_offset, 5, 1, buf); mqtt.publish("boat/io/pitch", buf, true);
      dtostrf(roll  - roll_offset,  5, 1, buf); mqtt.publish("boat/io/roll",  buf, true);
    }
  }

  // Kanäle (2s)
  if (now - lastChannelRead >= READ_INTERVAL) {
    lastChannelRead = now;
    if (mqtt.connected()) readAllChannels();
  }

  // Heartbeat (10s)
  if (now - lastStatus >= STATUS_INTERVAL) {
    lastStatus = now;
    if (mqtt.connected()) {
      mqtt.publish(MQTT_STATUS, "online", true);
      char buf[16];
      ltoa(now / 1000, buf, 10); mqtt.publish("boatopenio/uptime", buf);
      itoa(WiFi.RSSI(),  buf, 10); mqtt.publish("boatopenio/rssi",   buf);
      mqtt.publish("boatopenio/mode", testMode ? "TEST" : "LIVE", true);
    }
    Serial.printf("[%lus] WiFi:%s  MQTT:%s  Modus:%s  Pitch:%.1f  Roll:%.1f  Heap:%d\n",
                  now / 1000,
                  WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString().c_str() : "X",
                  mqtt.connected() ? "OK" : "X",
                  testMode ? "TEST" : "LIVE",
                  pitch, roll, ESP.getFreeHeap());
  }
}
