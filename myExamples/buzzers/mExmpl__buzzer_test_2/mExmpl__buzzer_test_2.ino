#include <OneButton.h>

#define BUZZ_PIN 13 // D13
#define BTN_PIN 2 // D2
#define PONT_PIN 0 // A0

OneButton btn = OneButton(BTN_PIN, true, true);

bool isBuzz = 0;
void btn_click() {isBuzz = !isBuzz;}

void setup() {
  Serial.begin(9600);

  pinMode(BUZZ_PIN, OUTPUT);
  pinMode(PONT_PIN, INPUT);

  btn.attachClick(btn_click);
}

void loop() {
  btn.tick();

  if(isBuzz) {tone(BUZZ_PIN, analogRead(PONT_PIN));}
  else noTone(BUZZ_PIN);

  Serial.print("isBuzz: ");Serial.print(isBuzz);Serial.print(';');Serial.print("Pont: ");Serial.println(analogRead(PONT_PIN));
}