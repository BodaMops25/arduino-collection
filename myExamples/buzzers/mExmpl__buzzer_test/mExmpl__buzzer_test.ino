#include <OneButton.h>
#define BUTTON_PIN 2 // D2
#define BUZZ_PIN 3 // D3
#define PONT_PIN 0  // A0

int step = 250;
bool isBuzz = 0;
OneButton btn = OneButton(BUTTON_PIN, 1, 1);
static void btn_click() {isBuzz = !isBuzz;}

void setup() {
  Serial.begin(9600);
  pinMode(BUZZ_PIN,OUTPUT);
  pinMode(PONT_PIN,INPUT);

  btn.attachClick(btn_click);
}

unsigned long time;
bool isLow;

void loop() {
  btn.tick();
  
  
  if(isBuzz) {
    if(micros() >= time) {
      step = analogRead(PONT_PIN);
      if(!isLow) {
        digitalWrite(BUZZ_PIN,HIGH);
        isLow = 1;
      }
      else if(isLow) {
        digitalWrite(BUZZ_PIN,LOW);
        isLow = 0;
      }

      time = micros() + (unsigned long) step*step / 10;
      Serial.print(isBuzz);Serial.print("; ");Serial.println(step);
    }
  }
}