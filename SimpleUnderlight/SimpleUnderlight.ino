#define LED_PIN 12
#define LED_AM 298
#define INIT_BRIGHTNESS 255

#include "FastLED.h"
CRGB leds[LED_AM];

#define CHANGING_COLOR_PERIOD_MS (unsigned long)30 * 60 * 1000
#define CHANGING_TO_RAINBOW_PERIOD_MS (unsigned long)60 * 60 * 1000
// #define CHANGING_COLOR_PERIOD_MS (unsigned long)5 * 1000
// #define CHANGING_TO_RAINBOW_PERIOD_MS (unsigned long)10 * 1000

// FLAME SETTINGS
#define HUE_GAP 25             // заброс по hue
#define FIRE_STEP 15           // шаг огня
#define MIN_SATURATION 255     // мин. насыщенность
#define MAX_SATURATION 255     // макс. насыщенность
#define MIN_VALUE 50           // мин. яркость огня
#define MAX_VALUE 255          // макс. яркость огня

#define ANIMATION_SPEED 8    // [1, 127] - adding speed, [-128, -1] - subtructing speed

#define COLORS_PATTERN_LENGTH 4

byte COLORS_PATTERN[COLORS_PATTERN_LENGTH] = {0, 85, 140, 200};

void setup() {
  FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, LED_AM).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(INIT_BRIGHTNESS);
}

byte currentColorIndex = 0;
int animationFrame = 0;
unsigned long datePoint = 0;

bool policeState = 0;

void loop() {

  if(policeState == 0) {
    for(int i = 0; i < LED_AM; i++) {
      if(i < LED_AM / 2) leds[i] = CHSV(0, 255, 255);
      else leds[i] = CHSV(160, 255, 255);
    }
  }
  else {
    for(int i = 0; i < LED_AM; i++) {
      if(i < LED_AM / 2) leds[i] = CHSV(160, 255, 255);
      else leds[i] = CHSV(0, 255, 255);
    }
  }

  policeState = !policeState;

  // if(currentColorIndex == 0) {
  //   fill_rainbow(leds, LED_AM, animationFrame/8, 1);

  //   if(millis() > datePoint + CHANGING_TO_RAINBOW_PERIOD_MS) {
  //     currentColorIndex++;
  //     datePoint = millis();
  //   }
  // }

  // else {

  //   for(int i = 0; i < LED_AM; i++) {
  //     byte value = inoise8(i*FIRE_STEP, animationFrame);
  //     leds[i] = getFlameColor(value);
  //     FastLED.show();
  //   }

  //   for(int i = 0; i < LED_AM; i++) {
  //     byte value = inoise8(i*FIRE_STEP, animationFrame);
  //     leds[i] = getFlameColor(value);
  //   }

  //   if(millis() > datePoint + CHANGING_COLOR_PERIOD_MS) {
  //     currentColorIndex++;
  //     datePoint = millis();
  //   }

  //   if(currentColorIndex >= COLORS_PATTERN_LENGTH) currentColorIndex = 0;
  // }

  // if(ANIMATION_SPEED > 0) animationFrame += ANIMATION_SPEED;
  // else if(ANIMATION_SPEED < 0 && millis() % -ANIMATION_SPEED == 0) animationFrame++;

  FastLED.show();

  delay(1);
}

CHSV getFlameColor(byte value) {
  return CHSV(
    COLORS_PATTERN[currentColorIndex] + map(value, 0, 255, 0, HUE_GAP),
    map(value, 0, 255, MAX_SATURATION, MIN_SATURATION),
    map(value, 0, 255, MIN_VALUE, MAX_VALUE)
  );
}