#define PIN 2
bool isLight = 0;

void setup() {
  Serial.begin(9600);
  pinMode(PIN,INPUT_PULLUP);
  pinMode(13,OUTPUT);
}

void loop() {
  bool button = digitalRead(PIN);

  if(button == 1 && isLight == 0) {
    isLight = 1;
    digitalWrite(13,HIGH);
    Serial.println("pressed");
  }
  if(button == 0 && isLight == 1) {
    isLight = 0;
    digitalWrite(13,LOW);
    Serial.println("relissed");
  }
}