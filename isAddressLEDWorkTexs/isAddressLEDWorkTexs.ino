#include <Adafruit_NeoPixel.h>

#define LED_NUM 292
#define ADDRESS_LED_PIN 23

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_NUM, ADDRESS_LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();
  strip.setBrightness(100);
  pinMode(3, OUTPUT);
  pinMode(2, OUTPUT);

  digitalWrite(3, HIGH);
  digitalWrite(2, LOW);
}

void loop() {
  strip.clear();
  for(int i = 0; i < LED_NUM; i++) strip.setPixelColor(i, strip.Color(255, 100, 0));
  strip.show();
}
