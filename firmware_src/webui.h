// ============================================================
// BoatOpenIO – Web-UI & Captive Portal (v2.3 – modern)
// ============================================================
#pragma once

#include <WebServer.h>
#include <DNSServer.h>

static DNSServer dnsServer;

static void startCaptivePortal() {
  dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
}

static void sendRedirect(const char* url) {
  webServer.sendHeader("Location", url, true);
  webServer.send(302, "text/plain", "");
}

static bool setupRequired() {
  return strlen(portal_pass) == 0;
}

static bool requireAuth() {
  if (!webServer.authenticate(portal_user, portal_pass)) {
    webServer.requestAuthentication(BASIC_AUTH, "BoatOpenIO");
    return false;
  }
  return true;
}

static bool isDefaultApPass() {
  return strcmp(ap_pass, AP_DEFAULT_PASS) == 0;
}

// ── ERSTEINRICHTUNG ──────────────────────────────────────────
void handleSetup() {
  webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  webServer.send(200, "text/html", "");
  webServer.sendContent(
    "<!DOCTYPE html><html lang='de'><head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>BoatOpenIO Setup</title>"
    "<style>"
    ":root{--bg:#080e1a;--c1:#0f1929;--acc:#06b6d4;--ok:#10b981;--warn:#f59e0b;--err:#ef4444;--txt:#e2e8f0;--sub:#94a3b8;--r:12px}"
    "*{box-sizing:border-box;margin:0;padding:0}"
    "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',sans-serif;background:var(--bg);"
    "color:var(--txt);min-height:100vh;display:flex;align-items:center;justify-content:center;padding:20px;"
    "background-image:radial-gradient(ellipse 60% 50% at 30% 20%,rgba(6,182,212,.12) 0%,transparent 60%)}"
    ".wrap{width:100%;max-width:440px}"
    ".logo{text-align:center;margin-bottom:28px}"
    ".logo h1{font-size:2rem;font-weight:800;background:linear-gradient(120deg,#06b6d4,#818cf8);"
    "-webkit-background-clip:text;-webkit-text-fill-color:transparent;background-clip:text}"
    ".logo p{color:var(--sub);font-size:.88rem;margin-top:6px}"
    ".card{background:var(--c1);border:1px solid rgba(255,255,255,.08);border-radius:var(--r);"
    "padding:24px;box-shadow:0 8px 40px rgba(0,0,0,.6),inset 0 1px 0 rgba(255,255,255,.04)}"
    ".warn{background:rgba(245,158,11,.1);border:1px solid rgba(245,158,11,.3);padding:12px 16px;"
    "border-radius:8px;color:var(--warn);margin-bottom:20px;font-size:.85rem;line-height:1.55}"
    "h2{font-size:.75rem;font-weight:700;color:var(--acc);text-transform:uppercase;letter-spacing:.1em;"
    "margin:20px 0 10px;display:flex;align-items:center;gap:7px}"
    "h2::before{content:'';display:block;width:3px;height:13px;background:var(--acc);border-radius:2px;flex-shrink:0}"
    "h2:first-of-type{margin-top:0}"
    "label{display:block;font-size:.8rem;color:var(--sub);margin:12px 0 4px;font-weight:500}"
    "input{background:#060c18;color:var(--txt);border:1px solid rgba(255,255,255,.1);padding:10px 12px;"
    "width:100%;border-radius:7px;font-size:.9rem;outline:none;transition:border-color .2s,box-shadow .2s}"
    "input:focus{border-color:var(--acc);box-shadow:0 0 0 3px rgba(6,182,212,.15)}"
    ".sep{border:none;border-top:1px solid rgba(255,255,255,.07);margin:20px 0}"
    ".btn{padding:12px;margin-top:16px;width:100%;border:none;border-radius:8px;"
    "background:linear-gradient(135deg,#047857,#10b981);color:#fff;font-size:.95rem;"
    "font-weight:600;cursor:pointer;transition:opacity .15s,transform .1s}"
    ".btn:hover{opacity:.88}.btn:active{transform:scale(.98)}"
    "</style></head><body>"
    "<div class='wrap'>"
    "<div class='logo'><h1>&#9875; BoatOpenIO</h1>"
    "<p>Universal Marine IO System &mdash; v2.2</p></div>"
    "<div class='card'>"
    "<div class='warn'>&#9888;&ensp;First-time setup required. Set a portal password to secure the "
    "configuration interface.<br>Ersteinrichtung erforderlich. Bitte ein Portal-Passwort setzen.</div>"
    "<form method='POST' action='/dosetup'>"
    "<h2>Portal Access</h2>"
    "<label>User / Benutzer</label>"
    "<input name='pu' value='admin' required autocomplete='username'>"
    "<label>Password / Passwort</label>"
    "<input type='password' name='pp' required minlength='6' placeholder='min. 6 Zeichen' autocomplete='new-password'>"
    "<label>Confirm / Best&auml;tigen</label>"
    "<input type='password' name='pp2' required minlength='6' placeholder='Wiederholen' autocomplete='new-password'>"
    "<hr class='sep'>"
    "<h2>AP Credentials</h2>"
    "<label>AP Name (SSID)</label>"
    "<input name='as' value='"
  );
  webServer.sendContent(ap_ssid);
  webServer.sendContent(
    "' required>"
    "<label>AP Password (min. 8 Zeichen)</label>"
    "<input type='password' name='ap' value='"
  );
  webServer.sendContent(ap_pass);
  webServer.sendContent(
    "' required minlength='8' autocomplete='new-password'>"
    "<button class='btn' type='submit'>Setup abschlie&szlig;en &rarr;</button>"
    "</form></div></div></body></html>"
  );
}

void handleDoSetup() {
  String pu  = webServer.arg("pu");
  String pp  = webServer.arg("pp");
  String pp2 = webServer.arg("pp2");
  String as_ = webServer.arg("as");
  String ap_ = webServer.arg("ap");

  if (pp != pp2 || pp.length() < 6 || as_.length() < 1 || ap_.length() < 8) {
    webServer.send(400, "text/html",
      "<html><head><meta charset='UTF-8'>"
      "<meta http-equiv='refresh' content='2;url=/setup'></head>"
      "<body style='background:#080e1a;color:#ef4444;font-family:sans-serif;padding:20px'>"
      "<p>Invalid input / Ung&uuml;ltige Eingabe.</p></body></html>");
    return;
  }
  pu.toCharArray(portal_user, sizeof(portal_user));
  pp.toCharArray(portal_pass, sizeof(portal_pass));
  as_.toCharArray(ap_ssid,    sizeof(ap_ssid));
  ap_.toCharArray(ap_pass,    sizeof(ap_pass));
  saveNetConfig();
  webServer.send(200, "text/html",
    "<html><head><meta charset='UTF-8'></head>"
    "<body style='background:#080e1a;color:#10b981;font-family:sans-serif;padding:20px'>"
    "<p>Setup complete. Restarting...</p></body></html>");
  delay(1000);
  ESP.restart();
}

// ── ROOT ─────────────────────────────────────────────────────
void handleRoot() {
  if (setupRequired()) { sendRedirect("/setup"); return; }
  if (!requireAuth())  return;

  webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  webServer.send(200, "text/html", "");

  // ── HEAD + CSS ──────────────────────────────────────────────
  webServer.sendContent(
    "<!DOCTYPE html><html lang='de'><head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>BoatOpenIO</title>"
    "<style>"
    ":root{--bg:#080e1a;--c1:#0f1929;--c2:#141e35;--acc:#06b6d4;--ok:#10b981;--warn:#f59e0b;--err:#ef4444;--txt:#e2e8f0;--sub:#94a3b8;--r:12px}"
    "*{box-sizing:border-box}"
    "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',sans-serif;background:var(--bg);color:var(--txt);"
    "max-width:960px;margin:0 auto;padding:12px 16px 48px;"
    "background-image:radial-gradient(ellipse 70% 50% at 0% 0%,rgba(6,182,212,.09) 0%,transparent 60%),"
    "radial-gradient(ellipse 60% 50% at 100% 100%,rgba(124,58,237,.07) 0%,transparent 60%)}"
    "h1{font-size:1.45rem;font-weight:800;background:linear-gradient(120deg,#06b6d4,#818cf8);"
    "-webkit-background-clip:text;-webkit-text-fill-color:transparent;background-clip:text;margin:0}"
    "h2{font-size:.73rem;font-weight:700;color:var(--acc);text-transform:uppercase;letter-spacing:.1em;"
    "margin:0 0 14px;display:flex;align-items:center;gap:6px}"
    "h2::before{content:'';width:3px;height:13px;background:var(--acc);border-radius:2px;flex-shrink:0}"
    ".hdr{display:flex;justify-content:space-between;align-items:center;padding:14px 0 12px;"
    "border-bottom:1px solid rgba(255,255,255,.06);margin-bottom:12px}"
    ".card{background:var(--c1);border:1px solid rgba(255,255,255,.07);border-radius:var(--r);"
    "padding:16px 20px;margin:8px 0;box-shadow:0 2px 20px rgba(0,0,0,.45),inset 0 1px 0 rgba(255,255,255,.03)}"
    "details.card{overflow:hidden}"
    "details.card>summary{cursor:pointer;list-style:none;display:flex;justify-content:space-between;"
    "align-items:center;margin:-16px -20px;padding:16px 20px;border-radius:var(--r);user-select:none;"
    "transition:background .15s}"
    "details.card>summary:hover{background:rgba(255,255,255,.04)}"
    "details.card>summary::-webkit-details-marker{display:none}"
    "details.card[open]>summary{border-radius:var(--r) var(--r) 0 0;"
    "border-bottom:1px solid rgba(255,255,255,.07);margin-bottom:14px}"
    ".chev{color:var(--sub);font-size:1.2em;transition:transform .25s;display:inline-block;line-height:1}"
    "details[open] .chev{transform:rotate(90deg)}"
    "label{display:block;font-size:.8rem;color:var(--sub);margin:10px 0 3px;font-weight:500}"
    "input,select{background:#060c18;color:var(--txt);border:1px solid rgba(255,255,255,.1);"
    "padding:8px 10px;width:100%;border-radius:7px;font-size:.87rem;outline:none;"
    "transition:border-color .2s,box-shadow .2s}"
    "input:focus,select:focus{border-color:var(--acc);box-shadow:0 0 0 3px rgba(6,182,212,.15)}"
    "input[type=checkbox]{width:16px;height:16px;accent-color:var(--acc);cursor:pointer}"
    "table{width:100%;border-collapse:collapse;font-size:.79rem}"
    "th{padding:7px 5px;text-align:left;font-size:.7rem;font-weight:600;color:var(--sub);"
    "text-transform:uppercase;letter-spacing:.04em;border-bottom:1px solid rgba(255,255,255,.09)}"
    "td{padding:4px 5px;border-bottom:1px solid rgba(255,255,255,.03)}"
    "tr:nth-child(even) td{background:rgba(255,255,255,.025)}"
    "tr:last-child td{border-bottom:none}"
    "button{padding:9px 14px;margin:4px 0;border:none;border-radius:8px;font-size:.87rem;"
    "font-weight:600;cursor:pointer;width:100%;transition:opacity .15s,transform .1s}"
    "button:hover{opacity:.85}button:active{transform:scale(.98)}"
    ".bsave{background:linear-gradient(135deg,#047857,#10b981);color:#fff}"
    ".breboot{background:linear-gradient(135deg,#991b1b,#dc2626);color:#fff}"
    ".bcal{background:linear-gradient(135deg,#1e40af,#06b6d4);color:#fff}"
    ".bsec{background:linear-gradient(135deg,#5b21b6,#8b5cf6);color:#fff}"
    ".btest{background:linear-gradient(135deg,#92400e,#f59e0b);color:#111}"
    ".blive{background:linear-gradient(135deg,#14532d,#22c55e);color:#fff}"
    ".blang{background:rgba(255,255,255,.06);border:1px solid rgba(255,255,255,.12)!important;"
    "color:var(--txt);padding:5px 16px;width:auto;font-size:.82rem;border-radius:20px}"
    ".ok{color:var(--ok)}.err{color:var(--err)}.warn{color:var(--warn)}"
    ".dot{width:8px;height:8px;border-radius:50%;display:inline-block;flex-shrink:0;vertical-align:middle}"
    ".dok{background:var(--ok);box-shadow:0 0 7px var(--ok);animation:pok 2.5s ease-in-out infinite}"
    ".derr{background:var(--err);box-shadow:0 0 5px var(--err)}"
    ".dwarn{background:var(--warn);box-shadow:0 0 5px var(--warn);animation:pwarn 1.5s ease-in-out infinite}"
    "@keyframes pok{0%,100%{opacity:1;box-shadow:0 0 5px var(--ok)}50%{opacity:.5;box-shadow:0 0 12px var(--ok)}}"
    "@keyframes pwarn{0%,100%{opacity:1}50%{opacity:.3}}"
    ".warnbox{background:rgba(245,158,11,.08);border:1px solid rgba(245,158,11,.35);padding:10px 14px;"
    "border-radius:8px;color:var(--warn);margin:8px 0;font-size:.85rem;display:flex;align-items:center;gap:10px}"
    ".stg{display:grid;grid-template-columns:1fr 1fr;gap:8px}"
    ".sti{background:var(--c2);border:1px solid rgba(255,255,255,.06);border-radius:9px;padding:11px 14px}"
    ".sl{font-size:.68rem;color:var(--sub);font-weight:600;text-transform:uppercase;letter-spacing:.06em;margin-bottom:4px}"
    ".sv{font-size:.87rem;font-weight:600;display:flex;align-items:center;gap:6px;flex-wrap:wrap}"
    ".achip{padding:2px 8px;border-radius:10px;font-size:.73rem;font-weight:700}"
    ".aok{background:rgba(16,185,129,.15);color:#10b981;border:1px solid rgba(16,185,129,.3)}"
    ".aerr{background:rgba(239,68,68,.15);color:#ef4444;border:1px solid rgba(239,68,68,.3)}"
    "#iw{display:flex;align-items:flex-start;gap:18px;margin:8px 0}"
    "#cal_status,#inv_status{font-size:.8rem;min-height:1.2em;color:var(--ok);margin-top:5px}"
    "@media(max-width:560px){.stg{grid-template-columns:1fr}#iw{flex-direction:column}}"
    "</style>"
  );

  // ── JS i18n + helpers ────────────────────────────────────────
  webServer.sendContent(
    "<script>"
    "var T={"
    "de:{"
    "h_wifi:'WiFi & MQTT',h_ch:'Kanalzuordnung',h_cal:'Kalibrierungs-Rechner',"
    "h_live:'Live-Werte',h_diag:'ADS Diagnose',"
    "h_imu:'IMU Kalibrierung',h_sec:'Sicherheit',h_act:'Aktionen',"
    "l_ssid:'WiFi SSID',l_wpass:'WiFi Passwort',l_mqtt:'MQTT Server',"
    "l_mport:'MQTT Port',l_muser:'MQTT User',l_mpass:'MQTT Passwort',"
    "save_net:'Speichern & Neustart',"
    "th_sensor:'Sensor',th_ads:'ADS',th_pin:'Pin',th_factor:'Faktor',"
    "th_offset:'Offset',th_unit:'Einheit',th_active:'Aktiv',"
    "save_ch:'Konfiguration speichern',"
    "cal_hint:'Faktor = Echter Wert / ADS-Pin-Spannung',"
    "l_real:'Echter Wert (Multimeter)',l_adsv:'ADS-Pin-Spannung [V]',l_factor:'Faktor',"
    "th_val:'Wert',loading:'Lade...',not_found:'nicht gefunden',"
    "imu_hint:'Boot auf ruhigem Wasser ausrichten, dann Null setzen.',"
    "imu_raw:'Rohwert',imu_corr:'Korrigiert',imu_off:'Offset',"
    "imu_bias:'Gyro-Bias (rad/s)',imu_btn:'Jetzt Null setzen',imu_saved:'Offset gespeichert',"
    "imu_inv_pitch:'Pitch invertieren',imu_inv_roll:'Roll invertieren',"
    "imu_inv_save:'Invertierung speichern',imu_inv_saved:'Gespeichert',"
    "sec_ap:'AP-Zugangsdaten',sec_portal:'Portal-Zugangsdaten',"
    "l_ap_ssid:'AP Name (SSID)',l_ap_pass:'AP Passwort (min. 8 Zeichen)',"
    "l_pu:'Portal Benutzer',l_pp:'Portal Passwort (min. 6 Zeichen)',"
    "l_pp2:'Passwort bestätigen',sec_save:'Speichern & Neustart',"
    "warn_defpass:'Standard-Passwort aktiv – bitte AP-Passwort ändern!',"
    "to_live:'→ LIVE (echte Sensoren)',to_test:'→ TEST (Fake-Daten)',"
    "restart:'Neustart',"
    "s_on:'verbunden',s_off:'getrennt',s_nowifi:'nicht verbunden',s_miss:'fehlt',"
    "s_uptime:'Uptime',s_mode:'Modus'"
    "},"
    "en:{"
    "h_wifi:'WiFi & MQTT',h_ch:'Channel Assignment',h_cal:'Calibration Calculator',"
    "h_live:'Live Values',h_diag:'ADS Diagnostics',"
    "h_imu:'IMU Calibration',h_sec:'Security',h_act:'Actions',"
    "l_ssid:'WiFi SSID',l_wpass:'WiFi Password',l_mqtt:'MQTT Server',"
    "l_mport:'MQTT Port',l_muser:'MQTT User',l_mpass:'MQTT Password',"
    "save_net:'Save & Restart',"
    "th_sensor:'Sensor',th_ads:'ADS',th_pin:'Pin',th_factor:'Factor',"
    "th_offset:'Offset',th_unit:'Unit',th_active:'Active',"
    "save_ch:'Save Configuration',"
    "cal_hint:'Factor = Real Value / ADS Pin Voltage',"
    "l_real:'Real Value (Multimeter)',l_adsv:'ADS Pin Voltage [V]',l_factor:'Factor',"
    "th_val:'Value',loading:'Loading...',not_found:'not found',"
    "imu_hint:'Level boat on calm water, then set zero.',"
    "imu_raw:'Raw',imu_corr:'Corrected',imu_off:'Offset',"
    "imu_bias:'Gyro Bias (rad/s)',imu_btn:'Set Zero Now',imu_saved:'Offset saved',"
    "imu_inv_pitch:'Invert Pitch',imu_inv_roll:'Invert Roll',"
    "imu_inv_save:'Save Inversion',imu_inv_saved:'Saved',"
    "sec_ap:'AP Credentials',sec_portal:'Portal Credentials',"
    "l_ap_ssid:'AP Name (SSID)',l_ap_pass:'AP Password (min. 8 chars)',"
    "l_pu:'Portal User',l_pp:'Portal Password (min. 6 chars)',"
    "l_pp2:'Confirm Password',sec_save:'Save & Restart',"
    "warn_defpass:'Default password active – please change the AP password!',"
    "to_live:'→ LIVE (real sensors)',to_test:'→ TEST (fake data)',"
    "restart:'Restart',"
    "s_on:'connected',s_off:'disconnected',s_nowifi:'not connected',s_miss:'missing',"
    "s_uptime:'Uptime',s_mode:'Mode'"
    "}"
    "};"
    "var curLang=localStorage.getItem('lang')||'de';"
    "function applyLang(l){"
    "var t=T[l];"
    "document.querySelectorAll('[data-i18n]').forEach(function(e){"
    "var k=e.getAttribute('data-i18n');"
    "if(t[k]!==undefined)e.textContent=t[k];"
    "});"
    "localStorage.setItem('lang',l);curLang=l;"
    "var b=document.getElementById('lbtn');if(b)b.textContent=l==='de'?'EN':'DE';"
    "}"
    "function toggleLang(){applyLang(curLang==='de'?'en':'de');}"
    "function updateHorizon(pitch,roll){"
    "var g=document.getElementById('imu-plane');if(!g)return;"
    "var py=Math.max(-38,Math.min(38,pitch*1.8));"
    "g.setAttribute('transform','rotate('+roll+') translate(0,'+py+')');"
    "}"
    "</script></head><body>"
  );

  // ── HEADER ───────────────────────────────────────────────────
  webServer.sendContent(
    "<div class='hdr'>"
    "<h1>&#9875; BoatOpenIO</h1>"
    "<button class='blang' id='lbtn' onclick='toggleLang()'>EN</button>"
    "</div>"
  );

  // ── DEFAULT-PASSWORT WARNUNG ──────────────────────────────────
  if (isDefaultApPass()) {
    webServer.sendContent(
      "<div class='warnbox'>&#9888;&ensp;"
      "<span data-i18n='warn_defpass'>Standard-Passwort aktiv &ndash; bitte AP-Passwort &auml;ndern!</span>"
      "</div>"
    );
  }

  // ── STATUS GRID ───────────────────────────────────────────────
  bool wifiUp = (WiFi.status() == WL_CONNECTED);
  webServer.sendContent("<div class='card'><h2>Status</h2><div class='stg'>");

  // AP
  webServer.sendContent("<div class='sti'><div class='sl'>Access Point</div><div class='sv'>");
  webServer.sendContent(ap_ssid);
  webServer.sendContent(" &rarr; 192.168.4.1</div></div>");

  // WiFi
  webServer.sendContent("<div class='sti'><div class='sl'>WiFi</div><div class='sv'>");
  if (wifiUp) {
    webServer.sendContent(
      String("<span class='dot dok'></span>") + WiFi.SSID() +
      " <small style='color:var(--sub)'>(" + WiFi.RSSI() + " dBm)</small>"
    );
  } else {
    webServer.sendContent("<span class='dot derr'></span><span class='warn' data-i18n='s_nowifi'>nicht verbunden</span>");
  }
  webServer.sendContent("</div></div>");

  // MQTT
  webServer.sendContent("<div class='sti'><div class='sl'>MQTT</div><div class='sv'>");
  if (mqtt.connected()) {
    webServer.sendContent(
      String("<span class='dot dok'></span><span class='ok' data-i18n='s_on'>verbunden</span>"
      " <small style='color:var(--sub)'>") + mqtt_server + "</small>"
    );
  } else {
    webServer.sendContent("<span class='dot derr'></span><span class='err' data-i18n='s_off'>getrennt</span>");
  }
  webServer.sendContent("</div></div>");

  // Mode
  webServer.sendContent("<div class='sti'><div class='sl' data-i18n='s_mode'>Modus</div><div class='sv'>");
  if (testMode) {
    webServer.sendContent("<span class='dot dwarn'></span><span style='color:var(--warn)'>TEST</span>");
  } else {
    webServer.sendContent("<span class='dot dok'></span><span class='ok'>LIVE</span>");
  }
  webServer.sendContent("</div></div>");

  // MPU6050
  webServer.sendContent("<div class='sti'><div class='sl'>MPU6050</div><div class='sv'>");
  if (mpuOK) {
    webServer.sendContent("<span class='dot dok'></span><span class='ok'>OK</span>");
  } else {
    webServer.sendContent("<span class='dot derr'></span><span class='err' data-i18n='s_miss'>fehlt</span>");
  }
  webServer.sendContent("</div></div>");

  // ADS
  webServer.sendContent("<div class='sti'><div class='sl'>ADS1115</div><div class='sv'>");
  for (int i = 0; i < 4; i++) {
    webServer.sendContent(
      String("<span class='achip ") + (adsOK[i] ? "aok" : "aerr") + "'>ADS" + (i + 1) + "</span>"
    );
  }
  webServer.sendContent("</div></div>");

  // Uptime
  webServer.sendContent(
    String("<div class='sti'><div class='sl' data-i18n='s_uptime'>Uptime</div>"
    "<div class='sv'>") + String(millis() / 1000) + " s</div></div>"
  );

  // Heap
  webServer.sendContent(
    String("<div class='sti'><div class='sl'>Heap</div>"
    "<div class='sv'>") + String(ESP.getFreeHeap() / 1024) + " KB free</div></div>"
  );

  webServer.sendContent("</div></div>"); // close stg + card

  // ── WIFI & MQTT (collapsible) ─────────────────────────────────
  webServer.sendContent(
    "<details class='card'>"
    "<summary><h2 data-i18n='h_wifi'>WiFi &amp; MQTT</h2><span class='chev'>&#8250;</span></summary>"
    "<form method='POST' action='/savenet'>"
  );
  webServer.sendContent(
    String("<label data-i18n='l_ssid'>WiFi SSID</label>"
    "<input name='ssid' value='") + wifi_ssid + "' autocomplete='off'>" +
    "<label data-i18n='l_wpass'>WiFi Passwort</label>"
    "<input type='password' name='wpass' placeholder='(unver&auml;ndert lassen)'>" +
    "<label data-i18n='l_mqtt'>MQTT Server</label>"
    "<input name='mqtt' value='" + mqtt_server + "'>" +
    "<label data-i18n='l_mport'>MQTT Port</label>"
    "<input name='mport' value='" + mqtt_port + "'>" +
    "<label data-i18n='l_muser'>MQTT User</label>"
    "<input name='muser' value='" + mqtt_user + "'>" +
    "<label data-i18n='l_mpass'>MQTT Passwort</label>"
    "<input type='password' name='mpass' placeholder='(unver&auml;ndert lassen)'>" +
    "<button class='bsave' type='submit' data-i18n='save_net'>Speichern &amp; Neustart</button>"
    "</form></details>"
  );

  // ── KANALZUORDNUNG (collapsible) ──────────────────────────────
  webServer.sendContent(
    "<details class='card'>"
    "<summary><h2 data-i18n='h_ch'>Kanalzuordnung</h2><span class='chev'>&#8250;</span></summary>"
    "<form method='POST' action='/save'>"
    "<div style='overflow-x:auto'>"
    "<table><tr>"
    "<th>#</th>"
    "<th data-i18n='th_sensor'>Sensor</th>"
    "<th data-i18n='th_ads'>ADS</th>"
    "<th data-i18n='th_pin'>Pin</th>"
    "<th data-i18n='th_factor'>Faktor</th>"
    "<th data-i18n='th_offset'>Offset</th>"
    "<th data-i18n='th_unit'>Einheit</th>"
    "<th data-i18n='th_active'>Aktiv</th>"
    "</tr>"
  );
  for (int i = 0; i < 16; i++) {
    String r = "<tr><td>" + String(i + 1) + "</td>";
    r += "<td><input name='s" + String(i) + "' value='" + kanaele[i].sensor + "'></td>";
    r += "<td><select name='a" + String(i) + "'>";
    for (int a = 1; a <= 4; a++)
      r += "<option value='" + String(a) + "'" + (kanaele[i].ads == a ? " selected" : "") + ">" + a + "</option>";
    r += "</select></td><td><select name='p" + String(i) + "'>";
    for (int p = 0; p <= 3; p++)
      r += "<option value='" + String(p) + "'" + (kanaele[i].pin == p ? " selected" : "") + ">A" + p + "</option>";
    r += "</select></td>";
    r += "<td><input name='f" + String(i) + "' value='" + String(kanaele[i].faktor, 4) + "' style='width:65px'></td>";
    r += "<td><input name='o" + String(i) + "' value='" + String(kanaele[i].offset, 4) + "' style='width:65px'></td>";
    r += "<td><input name='e" + String(i) + "' value='" + kanaele[i].einheit + "' style='width:45px'></td>";
    r += "<td><input type='checkbox' name='c" + String(i) + "'" + (kanaele[i].aktiv ? " checked" : "") + "></td></tr>";
    webServer.sendContent(r);
  }
  webServer.sendContent(
    "</table></div>"
    "<button class='bsave' type='submit' style='margin-top:12px' data-i18n='save_ch'>Konfiguration speichern</button>"
    "</form></details>"
  );

  // ── KALIBRIERUNGS-RECHNER (collapsible) ───────────────────────
  webServer.sendContent(
    "<details class='card'>"
    "<summary><h2 data-i18n='h_cal'>Kalibrierungs-Rechner</h2><span class='chev'>&#8250;</span></summary>"
    "<p style='font-size:.82rem;color:var(--sub);margin-bottom:12px' data-i18n='cal_hint'>"
    "Faktor = Echter Wert / ADS-Pin-Spannung</p>"
    "<label data-i18n='l_real'>Echter Wert (Multimeter)</label>"
    "<input type='number' step='0.001' id='cr' oninput='cf()'>"
    "<label data-i18n='l_adsv'>ADS-Pin-Spannung [V]</label>"
    "<input type='number' step='0.001' id='cp' oninput='cf()'>"
    "<label data-i18n='l_factor'>Faktor</label>"
    "<input id='cres' readonly>"
    "<script>"
    "function cf(){"
    "var r=parseFloat(document.getElementById('cr').value),"
    "p=parseFloat(document.getElementById('cp').value);"
    "if(!isNaN(r)&&!isNaN(p)&&p!==0)"
    "document.getElementById('cres').value=(r/p).toFixed(4);}"
    "</script></details>"
  );

  // ── LIVE-WERTE ────────────────────────────────────────────────
  webServer.sendContent(
    "<div class='card'>"
    "<h2 data-i18n='h_live'>Live-Werte</h2>"
    "<div id='vals'><small style='color:var(--sub)' data-i18n='loading'>Lade...</small></div>"
    "<script>"
    "function reloadVals(){"
    "fetch('/api/values').then(function(r){return r.json();}).then(function(d){"
    "var t=T[curLang];"
    "var s='<table><tr><th>Sensor</th><th>'+t.th_val+'</th></tr>';"
    "for(var k in d)s+='<tr><td>'+k+'</td><td>'+parseFloat(d[k]).toFixed(2)+'</td></tr>';"
    "s+='</table>';"
    "document.getElementById('vals').innerHTML=s;"
    "}).catch(function(){});}"
    "reloadVals();setInterval(reloadVals,3000);"
    "</script></div>"
  );

  // ── ADS DIAGNOSE (collapsible) ────────────────────────────────
  webServer.sendContent(
    "<details class='card'>"
    "<summary><h2 data-i18n='h_diag'>ADS Diagnose</h2><span class='chev'>&#8250;</span></summary>"
    "<div id='raw'><small style='color:var(--sub)' data-i18n='loading'>Lade...</small></div>"
    "<script>"
    "function reloadRaw(){"
    "fetch('/api/raw').then(function(r){return r.json();}).then(function(d){"
    "var t=T[curLang];"
    "var s='<table><tr><th>ADS</th><th>Addr</th><th>A0</th><th>A1</th><th>A2</th><th>A3</th></tr>';"
    "for(var i=1;i<=4;i++){"
    "var k='ads'+i,a=d[k];if(!a)continue;"
    "s+='<tr>';"
    "s+='<td>'+(a.ok?'<span class=ok>ADS'+i+'</span>':'<span class=err>ADS'+i+'</span>')+'</td>';"
    "s+='<td>'+a.addr+'</td>';"
    "if(a.ok){"
    "s+='<td>'+a.a0.toFixed(3)+'V</td>';"
    "s+='<td>'+a.a1.toFixed(3)+'V</td>';"
    "s+='<td>'+a.a2.toFixed(3)+'V</td>';"
    "s+='<td>'+a.a3.toFixed(3)+'V</td>';"
    "}else{"
    "s+='<td colspan=4 style=\"color:var(--err)\">'+t.not_found+'</td>';}"
    "s+='</tr>';}"
    "s+='</table>';"
    "document.getElementById('raw').innerHTML=s;"
    "}).catch(function(){});}"
    "reloadRaw();setInterval(reloadRaw,2000);"
    "</script></details>"
  );

  // ── IMU KALIBRIERUNG ──────────────────────────────────────────
  webServer.sendContent(
    "<div class='card'>"
    "<h2 data-i18n='h_imu'>IMU Kalibrierung</h2>"
    "<p style='font-size:.82rem;color:var(--sub);margin-bottom:12px' data-i18n='imu_hint'>"
    "Boot auf ruhigem Wasser ausrichten, dann Null setzen.</p>"
    "<div id='iw'>"
    "<svg viewBox='-50 -50 100 100' width='110' height='110' style='flex-shrink:0'>"
    "<defs><clipPath id='cc'><circle r='47'/></clipPath></defs>"
    "<g clip-path='url(#cc)'>"
    "<g id='imu-plane'>"
    "<rect x='-60' y='-80' width='120' height='80' fill='#1e3a8a' opacity='.7'/>"
    "<rect x='-60' y='0' width='120' height='80' fill='#064e3b' opacity='.7'/>"
    "<line x1='-55' y1='0' x2='55' y2='0' stroke='#06b6d4' stroke-width='2'/>"
    "</g></g>"
    "<circle r='47' fill='none' stroke='rgba(6,182,212,.45)' stroke-width='2.5'/>"
    "<line x1='-20' y1='0' x2='20' y2='0' stroke='#f59e0b' stroke-width='2.5' stroke-linecap='round'/>"
    "<line x1='0' y1='-8' x2='0' y2='8' stroke='#f59e0b' stroke-width='2' stroke-linecap='round'/>"
    "<circle r='2.5' fill='#f59e0b'/>"
    "</svg>"
    "<div id='imu_table' style='flex:1'><small style='color:var(--sub)' data-i18n='loading'>Lade...</small></div>"
    "</div>"
    "<button class='bcal' onclick='doCalibrate()' data-i18n='imu_btn'>Jetzt Null setzen</button>"
    "<div id='cal_status'></div>"
    "<div style='margin-top:14px;padding-top:12px;border-top:1px solid rgba(255,255,255,.07)'>"
    "<label style='display:flex;align-items:center;gap:8px;margin:6px 0;cursor:pointer;width:auto'>"
    "<input type='checkbox' id='pitch_inv'>"
    "<span data-i18n='imu_inv_pitch'>Pitch invertieren</span></label>"
    "<label style='display:flex;align-items:center;gap:8px;margin:6px 0;cursor:pointer;width:auto'>"
    "<input type='checkbox' id='roll_inv'>"
    "<span data-i18n='imu_inv_roll'>Roll invertieren</span></label>"
    "<button class='bsave' onclick='saveInvert()' style='margin-top:8px' data-i18n='imu_inv_save'>"
    "Invertierung speichern</button>"
    "<div id='inv_status'></div>"
    "</div>"
    "<script>"
    "var imuInvLoaded=false;"
    "function reloadIMU(){"
    "fetch('/api/imu').then(function(r){return r.json();}).then(function(d){"
    "var t=T[curLang];"
    "var s='<table>';"
    "s+='<tr><th></th><th>Pitch</th><th>Roll</th></tr>';"
    "s+='<tr><td>'+t.imu_raw+'</td>"
    "<td>'+d.pitch_raw.toFixed(2)+'&deg;</td>"
    "<td>'+d.roll_raw.toFixed(2)+'&deg;</td></tr>';"
    "s+='<tr><td>'+t.imu_corr+'</td>"
    "<td><b>'+d.pitch_corr.toFixed(2)+'&deg;</b></td>"
    "<td><b>'+d.roll_corr.toFixed(2)+'&deg;</b></td></tr>';"
    "s+='<tr><td>'+t.imu_off+'</td>"
    "<td style=\"color:var(--sub)\">'+d.pitch_offset.toFixed(2)+'&deg;</td>"
    "<td style=\"color:var(--sub)\">'+d.roll_offset.toFixed(2)+'&deg;</td></tr>';"
    "s+='</table>';"
    "s+='<small style=\"color:#4a5568;margin-top:5px;display:block\">"
    "'+t.imu_bias+': X='+d.bias_x.toFixed(4)+' Y='+d.bias_y.toFixed(4)+' Z='+d.bias_z.toFixed(4)+'</small>';"
    "document.getElementById('imu_table').innerHTML=s;"
    "updateHorizon(d.pitch_corr,d.roll_corr);"
    "if(!imuInvLoaded){"
    "var pi=document.getElementById('pitch_inv');"
    "var ri=document.getElementById('roll_inv');"
    "if(pi)pi.checked=!!d.pitch_invert;"
    "if(ri)ri.checked=!!d.roll_invert;"
    "imuInvLoaded=true;}"
    "}).catch(function(){});}"
    "function saveInvert(){"
    "var fd=new FormData();"
    "if(document.getElementById('pitch_inv').checked)fd.append('pi','1');"
    "if(document.getElementById('roll_inv').checked)fd.append('ri','1');"
    "fetch('/setinvert',{method:'POST',body:fd})"
    ".then(function(r){return r.json();})"
    ".then(function(d){"
    "if(d.ok){"
    "var s=document.getElementById('inv_status');"
    "s.textContent=T[curLang].imu_inv_saved;"
    "setTimeout(function(){s.textContent='';},3000);}}).catch(function(){});}"
    "function doCalibrate(){"
    "fetch('/calibrate',{method:'POST'}).then(function(r){return r.json();}).then(function(d){"
    "if(d.ok){"
    "document.getElementById('cal_status').textContent=T[curLang].imu_saved;"
    "setTimeout(function(){document.getElementById('cal_status').textContent='';},3000);"
    "reloadIMU();}}).catch(function(){});}"
    "reloadIMU();setInterval(reloadIMU,1000);"
    "</script></div>"
  );

  // ── SICHERHEIT (collapsible) ──────────────────────────────────
  webServer.sendContent(
    "<details class='card'>"
    "<summary><h2 data-i18n='h_sec'>Sicherheit</h2><span class='chev'>&#8250;</span></summary>"
    "<form method='POST' action='/savesec'>"
    "<p style='font-size:.82rem;color:var(--sub);margin-bottom:8px;font-weight:600' data-i18n='sec_ap'>AP-Zugangsdaten</p>"
    "<label data-i18n='l_ap_ssid'>AP Name (SSID)</label>"
    "<input name='as' value='"
  );
  webServer.sendContent(ap_ssid);
  webServer.sendContent(
    "'>"
    "<label data-i18n='l_ap_pass'>AP Passwort (min. 8 Zeichen)</label>"
    "<input type='password' name='ap' placeholder='...' minlength='8'>"
    "<p style='font-size:.82rem;color:var(--sub);margin:18px 0 8px;font-weight:600' data-i18n='sec_portal'>Portal-Zugangsdaten</p>"
    "<label data-i18n='l_pu'>Portal Benutzer</label>"
    "<input name='pu' value='"
  );
  webServer.sendContent(portal_user);
  webServer.sendContent(
    "'>"
    "<label data-i18n='l_pp'>Portal Passwort (min. 6 Zeichen)</label>"
    "<input type='password' name='pp' placeholder='...' minlength='6'>"
    "<label data-i18n='l_pp2'>Passwort best&auml;tigen</label>"
    "<input type='password' name='pp2' placeholder='...' minlength='6'>"
    "<button class='bsec' type='submit' style='margin-top:12px' data-i18n='sec_save'>Speichern &amp; Neustart</button>"
    "</form></details>"
  );

  // ── AKTIONEN ──────────────────────────────────────────────────
  webServer.sendContent(
    String("<div class='card'>"
    "<h2 data-i18n='h_act'>Aktionen</h2>"
    "<form method='POST' action='/testmode'>") +
    (testMode
      ? "<button class='blive' type='submit' data-i18n='to_live'>→ LIVE (echte Sensoren)</button>"
      : "<button class='btest' type='submit' data-i18n='to_test'>→ TEST (Fake-Daten)</button>") +
    "</form>"
    "<form method='POST' action='/reboot'>"
    "<button class='breboot' type='submit' data-i18n='restart'>Neustart</button>"
    "</form></div>"
    "<script>applyLang(curLang);</script>"
    "</body></html>"
  );
}

// ── SAVE KANALCONFIG ─────────────────────────────────────────
void handleSave() {
  if (!requireAuth()) return;
  for (int i = 0; i < 16; i++) {
    kanaele[i].sensor  = webServer.arg("s" + String(i));
    kanaele[i].ads     = webServer.arg("a" + String(i)).toInt();
    kanaele[i].pin     = webServer.arg("p" + String(i)).toInt();
    kanaele[i].faktor  = webServer.arg("f" + String(i)).toFloat();
    kanaele[i].offset  = webServer.arg("o" + String(i)).toFloat();
    kanaele[i].einheit = webServer.arg("e" + String(i));
    kanaele[i].aktiv   = webServer.hasArg("c" + String(i));
  }
  saveConfig();
  webServer.sendHeader("Location", "/");
  webServer.send(303);
}

// ── SAVE NETZWERK-CONFIG ──────────────────────────────────────
void handleSaveNet() {
  if (!requireAuth()) return;
  if (webServer.hasArg("ssid") && webServer.arg("ssid").length())
    strncpy(wifi_ssid,   webServer.arg("ssid").c_str(),  sizeof(wifi_ssid)   - 1);
  if (webServer.hasArg("wpass") && webServer.arg("wpass").length())
    strncpy(wifi_pass,   webServer.arg("wpass").c_str(), sizeof(wifi_pass)   - 1);
  if (webServer.hasArg("mqtt") && webServer.arg("mqtt").length())
    strncpy(mqtt_server, webServer.arg("mqtt").c_str(),  sizeof(mqtt_server) - 1);
  if (webServer.hasArg("mport") && webServer.arg("mport").length())
    strncpy(mqtt_port,   webServer.arg("mport").c_str(), sizeof(mqtt_port)   - 1);
  if (webServer.hasArg("muser"))
    strncpy(mqtt_user,   webServer.arg("muser").c_str(), sizeof(mqtt_user)   - 1);
  if (webServer.hasArg("mpass") && webServer.arg("mpass").length())
    strncpy(mqtt_pass,   webServer.arg("mpass").c_str(), sizeof(mqtt_pass)   - 1);
  saveNetConfig();
  webServer.send(200, "text/html",
    "<html><head><meta charset='UTF-8'>"
    "<meta http-equiv='refresh' content='3;url=/'></head>"
    "<body style='background:#080e1a;color:#10b981;font-family:sans-serif;padding:20px'>"
    "<p>Gespeichert. Neustart... / Saved. Restarting...</p></body></html>");
  delay(1000);
  ESP.restart();
}

// ── SAVE SICHERHEITS-CONFIG ───────────────────────────────────
void handleSaveSec() {
  if (!requireAuth()) return;
  String as_ = webServer.arg("as");
  String ap_ = webServer.arg("ap");
  String pu  = webServer.arg("pu");
  String pp  = webServer.arg("pp");
  String pp2 = webServer.arg("pp2");

  if (as_.length() > 0)
    as_.toCharArray(ap_ssid, sizeof(ap_ssid));
  if (ap_.length() >= 8)
    ap_.toCharArray(ap_pass, sizeof(ap_pass));
  if (pu.length() > 0)
    pu.toCharArray(portal_user, sizeof(portal_user));
  if (pp.length() >= 6 && pp == pp2)
    pp.toCharArray(portal_pass, sizeof(portal_pass));

  saveNetConfig();
  webServer.send(200, "text/html",
    "<html><head><meta charset='UTF-8'>"
    "<meta http-equiv='refresh' content='3;url=/'></head>"
    "<body style='background:#080e1a;color:#10b981;font-family:sans-serif;padding:20px'>"
    "<p>Gespeichert. Neustart... / Saved. Restarting...</p></body></html>");
  delay(1000);
  ESP.restart();
}

// ── TEST/LIVE UMSCHALTEN ──────────────────────────────────────
void handleTestMode() {
  if (!requireAuth()) return;
  testMode = !testMode;
  saveNetConfig();
  webServer.send(200, "text/html",
    "<html><head><meta charset='UTF-8'>"
    "<meta http-equiv='refresh' content='2;url=/'></head>"
    "<body style='background:#080e1a;color:#e2e8f0;font-family:sans-serif;padding:20px'>"
    "<p>Modus ge&auml;ndert / Mode changed.</p></body></html>");
  delay(1000);
  ESP.restart();
}

// ── NEUSTART ─────────────────────────────────────────────────
void handleReboot() {
  if (!requireAuth()) return;
  webServer.send(200, "text/html",
    "<html><head><meta charset='UTF-8'></head>"
    "<body style='background:#080e1a;color:#e2e8f0;font-family:sans-serif;padding:20px'>"
    "<p>Neustart / Restarting...</p></body></html>");
  delay(500);
  ESP.restart();
}

// ── LIVE-WERTE API ───────────────────────────────────────────
void handleValues() {
  StaticJsonDocument<1024> doc;
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
    doc[kanaele[i].sensor] = roundf((voltage * kanaele[i].faktor + kanaele[i].offset) * 100.f) / 100.f;
  }
  doc["pitch"] = roundf((pitch - pitch_offset) * (pitch_invert ? -1.0f : 1.0f) * 10.f) / 10.f;
  doc["roll"]  = roundf((roll  - roll_offset)  * (roll_invert  ? -1.0f : 1.0f) * 10.f) / 10.f;
  String out;
  serializeJson(doc, out);
  webServer.send(200, "application/json", out);
}

// ── RAW ADS DIAGNOSE API ─────────────────────────────────────
void handleRaw() {
  const char* addrStr[4] = {"0x48", "0x49", "0x4A", "0x4B"};
  StaticJsonDocument<512> doc;
  for (int i = 0; i < 4; i++) {
    JsonObject a = doc.createNestedObject(String("ads") + (i + 1));
    a["ok"]   = adsOK[i];
    a["addr"] = addrStr[i];
    if (adsOK[i]) {
      for (int p = 0; p < 4; p++) {
        int16_t raw = ads[i].readADC_SingleEnded(p);
        a["a" + String(p)] = roundf(ads[i].computeVolts(raw) * 1000.f) / 1000.f;
      }
    }
  }
  String out;
  serializeJson(doc, out);
  webServer.send(200, "application/json", out);
}

// ── IMU API ──────────────────────────────────────────────────
void handleIMU() {
  float pc = (pitch - pitch_offset) * (pitch_invert ? -1.0f : 1.0f);
  float rc = (roll  - roll_offset)  * (roll_invert  ? -1.0f : 1.0f);
  StaticJsonDocument<256> doc;
  doc["pitch_raw"]    = roundf(pitch * 100.f) / 100.f;
  doc["roll_raw"]     = roundf(roll  * 100.f) / 100.f;
  doc["pitch_corr"]   = roundf(pc * 100.f) / 100.f;
  doc["roll_corr"]    = roundf(rc * 100.f) / 100.f;
  doc["pitch_offset"] = roundf(pitch_offset * 100.f) / 100.f;
  doc["roll_offset"]  = roundf(roll_offset  * 100.f) / 100.f;
  doc["pitch_invert"] = pitch_invert;
  doc["roll_invert"]  = roll_invert;
  doc["bias_x"]       = roundf(gyroBiasX * 10000.f) / 10000.f;
  doc["bias_y"]       = roundf(gyroBiasY * 10000.f) / 10000.f;
  doc["bias_z"]       = roundf(gyroBiasZ * 10000.f) / 10000.f;
  String out;
  serializeJson(doc, out);
  webServer.send(200, "application/json", out);
}

// ── INVERTIERUNG SETZEN ───────────────────────────────────────
void handleSetInvert() {
  if (!requireAuth()) return;
  pitch_invert = webServer.hasArg("pi") && webServer.arg("pi") == "1";
  roll_invert  = webServer.hasArg("ri") && webServer.arg("ri") == "1";
  Preferences prefs;
  prefs.begin("boatopenio", false);
  prefs.putBool("pitch_inv", pitch_invert);
  prefs.putBool("roll_inv",  roll_invert);
  prefs.end();
  StaticJsonDocument<64> doc;
  doc["ok"]           = true;
  doc["pitch_invert"] = pitch_invert;
  doc["roll_invert"]  = roll_invert;
  String out;
  serializeJson(doc, out);
  webServer.send(200, "application/json", out);
}

// ── MONTAGE-OFFSET SETZEN ────────────────────────────────────
void handleCalibrate() {
  if (!requireAuth()) return;
  pitch_offset = pitch;
  roll_offset  = roll;
  Preferences prefs;
  prefs.begin("boatopenio", false);
  prefs.putFloat("pitch_off", pitch_offset);
  prefs.putFloat("roll_off",  roll_offset);
  prefs.end();
  StaticJsonDocument<128> doc;
  doc["ok"]           = true;
  doc["pitch_offset"] = roundf(pitch_offset * 100.f) / 100.f;
  doc["roll_offset"]  = roundf(roll_offset  * 100.f) / 100.f;
  String out;
  serializeJson(doc, out);
  webServer.send(200, "application/json", out);
}

// ── SETUP ────────────────────────────────────────────────────
void setupWebServer() {
  startCaptivePortal();

  webServer.on("/",           HTTP_GET,  handleRoot);
  webServer.on("/setup",      HTTP_GET,  handleSetup);
  webServer.on("/dosetup",    HTTP_POST, handleDoSetup);
  webServer.on("/save",       HTTP_POST, handleSave);
  webServer.on("/savenet",    HTTP_POST, handleSaveNet);
  webServer.on("/savesec",    HTTP_POST, handleSaveSec);
  webServer.on("/testmode",   HTTP_POST, handleTestMode);
  webServer.on("/reboot",     HTTP_POST, handleReboot);
  webServer.on("/api/values", HTTP_GET,  handleValues);
  webServer.on("/api/raw",    HTTP_GET,  handleRaw);
  webServer.on("/api/imu",    HTTP_GET,  handleIMU);
  webServer.on("/calibrate",  HTTP_POST, handleCalibrate);
  webServer.on("/setinvert",  HTTP_POST, handleSetInvert);

  webServer.onNotFound([]() {
    if (setupRequired()) sendRedirect("http://192.168.4.1/setup");
    else                 sendRedirect("http://192.168.4.1/");
  });

  webServer.begin();
  Serial.println("WebServer + Captive Portal gestartet");
  Serial.println("  http://192.168.4.1  (immer, via AP)");
  if (WiFi.status() == WL_CONNECTED)
    Serial.println("  http://" + WiFi.localIP().toString() + "  (via Router)");
}

// ── LOOP ─────────────────────────────────────────────────────
void loopWebServer() {
  dnsServer.processNextRequest();
  webServer.handleClient();
}
