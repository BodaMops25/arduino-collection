#include <Adafruit_NeoPixel.h>

#define LED_PIN 13
#define LED_COUNT 16
#define BRIGHTNESS 50

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);

void setup() {

  strip.begin();
  strip.show();
  strip.setBrightness(BRIGHTNESS);
}

void loop() {
   for(int i = 0; i < strip.numPixels(); i++) {
     strip.setPixelColor(i, strip.Color(0, 0, 255));
   }
   strip.show();
}
