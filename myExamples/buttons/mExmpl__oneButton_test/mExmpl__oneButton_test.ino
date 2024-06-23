#include <OneButton.h>
#define BUTTON_PIN 2
bool isLight = 0;

OneButton btn = OneButton(BUTTON_PIN, true, true);
static void btn_click() {
  isLight = !isLight;
  Serial.println("Clicked");
}

void setup() {
  Serial.begin(9600);
  btn.attachClick(btn_click);
}

void loop() {
  btn.tick();

  if(isLight) digitalWrite(13,HIGH);    
  else digitalWrite(13,LOW);    
}
