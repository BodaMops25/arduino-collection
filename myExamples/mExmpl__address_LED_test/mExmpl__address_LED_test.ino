#include <Adafruit_NeoPixel.h>
#define ADDRESS_LED_PIN 12
#define PONT_RED 0 // A0
#define PONT_GREEN 1 // A1
#define PONT_BLUE 2 // A2
#define NUMPIXELS 210

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, ADDRESS_LED_PIN, NEO_GRB + NEO_KHZ800);

#define DELAYVAL 250 // Time (in milliseconds) to pause between pixels

void setup() {
  Serial.begin(9600);
  pinMode(A0,INPUT);
  strip.begin();
}

void loop() {
  strip.clear();
// int r = 200;
// int g = 0;
// int b = 0;

  int r = (float)analogRead(PONT_RED) / 950 * 255;
  int g = (float)analogRead(PONT_GREEN) / 950 * 255;
  int b = (float)analogRead(PONT_BLUE) / 950 * 255;

  // int r = random(0,255);
  // int g = random(0,255);
  // int b = random(0,255);

  Serial.print(r);Serial.print(" -> "); Serial.print(g);Serial.print(" -> "); Serial.println(b);
  
  for(int i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}
