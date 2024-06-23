// TIME

#define BEGIN_WAKEUP_STAMP (long)6 * 3600
#define STOP_WAKEUP_STAMP (long)7 * 3600
#define OFF_WAKEUP_STAMP (long)15 * 3600

// LED

#define LED_AM 292
#define LED_PIN 2

#include <microDS3231.h>
MicroDS3231 rtc;

#include "FastLED.h"
CRGB leds[LED_AM];

void setup() {
  FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, LED_AM).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(255);
  
  // проверка наличия модуля на линии i2c
  // вызов rtc.begin() не обязателен для работы
  if (!rtc.begin()) {
    Serial.println("DS3231 not found");
    for(;;);
  }

  // rtc.setTime(BUILD_SEC, BUILD_MIN, BUILD_HOUR, BUILD_DAY, BUILD_MONTH, BUILD_YEAR);

  // Serial.begin(9600);
  // Serial.print(rtc.getHours());Serial.print('.');
  // Serial.print(rtc.getMinutes());Serial.print('.');
  // Serial.println(rtc.getSeconds());
}

byte timeDelay = 0;
byte brightness = 0;
bool isLed = false;

void loop() {

  // Serial.print(rtc.getHours());Serial.print('.');
  // Serial.print(rtc.getMinutes());Serial.print('.');
  // Serial.println(rtc.getSeconds());

  if(rtc.getSeconds() != timeDelay) {

    long timestamp = (long)rtc.getHours() * 3600 + rtc.getMinutes() * 60 + rtc.getSeconds();

    if(BEGIN_WAKEUP_STAMP <= timestamp && timestamp < STOP_WAKEUP_STAMP) {

      byte brightness;
      brightness = map(timestamp, BEGIN_WAKEUP_STAMP, STOP_WAKEUP_STAMP, 0, 255);
      brightness = constrain(brightness, 0, 255);

      fill_solid(leds, LED_AM, CHSV(20, 50, brightness));

      FastLED.show();
    }
    else if(STOP_WAKEUP_STAMP <= timestamp && timestamp < OFF_WAKEUP_STAMP && !isLed) {
      fill_solid(leds, LED_AM, CHSV(20, 50, 255));
      FastLED.show();

      isLed = true;
    }
    else if(OFF_WAKEUP_STAMP <= timestamp && isLed) {
      FastLED.clear();
      FastLED.show();

      isLed = false;
    }

    timeDelay = rtc.getSeconds();
  }
}
