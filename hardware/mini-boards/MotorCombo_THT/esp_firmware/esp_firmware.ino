const int inputPin = 0;       // GPIO0: Eingang vom Optokoppler (Lima-W)
const int analogOutPin = 2;   // GPIO2: PWM-Ausgang zum MUX / ADS1115

volatile unsigned long pulsZaehler = 0;
unsigned long letzteMessung = 0;
const unsigned long messIntervall = 200; // Alle 200ms aktualisieren für schnelle Reaktion

void IRAM_ATTR pulsISR() {
  pulsZaehler++;
}

void setup() {
  pinMode(inputPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(inputPin), pulsISR, RISING);

  pinMode(analogOutPin, OUTPUT);

  // WICHTIG beim ESP8266/ESP01s: Erhöhe die PWM-Frequenz auf z.B. 5000 Hz.
  // Das macht das Filtern mit dem 1k Widerstand und Kondensator extrem effektiv.
  analogWriteFreq(5000);
}

void loop() {
  unsigned long aktuelleZeit = millis();

  if (aktuelleZeit - letzteMessung >= messIntervall) {
    noInterrupts();
    unsigned long pulse = pulsZaehler;
    pulsZaehler = 0;
    interrupts();

    letzteMessung = aktuelleZeit;

    // Frequenz der Lichtmaschine berechnen
    float frequenz = (pulse * 1000.0) / messIntervall;

    // HIER IHRER ANPASSUNG: Max. Frequenz Ihrer Lima bei Höchstdrehzahl ermitteln
    // Beispiel: 0 Hz = 0V (PWM 0) | 1500 Hz (z.B. 6000 U/min) = 3.3V (PWM 1023)
    // Hinweis: ESP8266 nutzt standardmäßig 10-Bit PWM (0 bis 1023)
    int pwmWert = map(frequenz, 0, 1500, 0, 1023);
    pwmWert = constrain(pwmWert, 0, 1023);

    // Ausgabe an das RC-Glied vor dem MUX
    analogWrite(analogOutPin, pwmWert);
  }
}
