// BoatOpenIO – W-Klemme Simulator v2
// D1 Mini / NodeMCU – simuliert Lichtmaschine W-Klemme
// PWM mit variablem Duty Cycle – 0 RPM = 0%, MAX_RPM = 100%
// 8 Impulse pro Umdrehung

#define PIN_OUT    D2
#define IMPULSE_PRO_UMDREHUNG 8
#define MAX_RPM    4000
#define PWM_FREQ   1000  // 1kHz PWM Frequenz

int zielRPM = 1500;

void setup() {
  Serial.begin(115200);
  analogWriteFreq(PWM_FREQ);
  analogWriteRange(1023);
  pinMode(PIN_OUT, OUTPUT);
  analogWrite(PIN_OUT, 0);
  Serial.println("W-Klemme Simulator v2 gestartet");
  Serial.print("Ziel RPM: ");
  Serial.println(zielRPM);
}

void loop() {
  // Duty Cycle proportional zu RPM
  int dutyCycle = (int)((float)zielRPM / MAX_RPM * 1023);
  if (dutyCycle > 1023) dutyCycle = 1023;
  if (dutyCycle < 0)    dutyCycle = 0;
  analogWrite(PIN_OUT, dutyCycle);

  // RPM per Serial ändern
  if (Serial.available()) {
    int neueRPM = Serial.parseInt();
    if (neueRPM >= 0 && neueRPM <= MAX_RPM) {
      zielRPM = neueRPM;
      Serial.print("Neues Ziel RPM: ");
      Serial.println(zielRPM);
    }
  }

  delay(100);
}
