#include <GyverTimers.h>
#define pin 13
volatile byte duty;

void setup() {
  pinMode(pin,OUTPUT);
  pinMode(A0,INPUT);

  Timer2.setFrequency(125000UL);
  Timer2.enableISR();
}

void loop() {
  duty = analogRead(A0) / 4;
}

ISR(TIMER2_A) {pwmTick();}

void pwmTick() {
  static volatile byte counter = 0;
  if(counter == 0 && duty > 0) {digitalWrite(pin,HIGH);}
  else if(counter == duty) {digitalWrite(pin,LOW);}
  counter++;
}