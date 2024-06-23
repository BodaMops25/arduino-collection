#define MOSFET_PIN 12
#define LED_PIN 13
#define TIMING_PONT_PIN A0
#define TIMING_STEP 20

void setup() {
  Serial.begin(9600);

  pinMode(MOSFET_PIN, OUTPUT);
  pinMode(LED_PIN, INPUT);
  pinMode(TIMING_PONT_PIN, INPUT);

  digitalWrite(MOSFET_PIN, LOW);
}

void loop() {
  int timing_pont_val = analogRead(TIMING_PONT_PIN);

  Serial.println(timing_pont_val);

  digitalWrite(MOSFET_PIN, HIGH);
  delay(TIMING_STEP);
  digitalWrite(MOSFET_PIN, LOW);
  delay(timing_pont_val * 2);
}
