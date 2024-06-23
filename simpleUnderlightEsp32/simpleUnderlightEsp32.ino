#define GP_NO_MDNS
#define GP_NO_DNS
#define GP_NO_UPLOAD
#define GP_NO_DOWNLOAD

#define OTA_LOGIN "BodaMops25"
#define OTA_PASS "123aA456"

#define AP_SSID "NETGEAR82"
#define AP_PASS "notsmoothpanda742"

// NTP

#define NTP_SERVER "in.pool.ntp.org"
#define UTS_OFFSET_SEC 3600
#define DAYLIGHT_OFFSET_SEC 0

#include "time.h"

unsigned long timestampSec = 0;
struct Time {
  unsigned long time;
  byte hours, minutes, seconds;
  Time(unsigned long time, byte hours, byte minutes, byte seconds) : time(time), hours(hours), minutes(minutes), seconds(seconds) {}
};

#define LED_PIN 2
#define LED_BLINK_DELAY 100

#define LED_STRIP_PIN 13
#define LED_AM 292

// FLAME SETTINGS
#define HUE_GAP 25             // заброс по hue
#define FIRE_STEP 15           // шаг огня
#define MIN_SATURATION 255     // мин. насыщенность
#define MAX_SATURATION 255     // макс. насыщенность
#define MIN_VALUE 50           // мин. яркость огня
#define MAX_VALUE 255          // макс. яркость огня

#include <LittleFS.h>
#include <FileData.h>

bool power = 1;
long animationFrame = 0;

struct Settings {
  unsigned long autoPowerOnSec = 8 * 3600;
  byte mode = 0, hue = 0, brightness = 1;
  int animationSpeed = 1; // positive values add speed, negative values subtruct speed
};
Settings settings;

FileData settingsData(&LittleFS, "/settings", 'A', &settings, sizeof(settings));

#include "FastLED.h"
CRGB leds[LED_AM];

#include <GyverPortal.h>
GyverPortal portal;

void updateTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)) {
    portal.log.println("Unable to get time from NTP Server.");
    return;
  }

  timestampSec = timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60 + timeinfo.tm_sec;
}
Time getTime() {
  unsigned long time = (timestampSec + (millis() / 1000)) % (3600 * 24);

  byte hours = time / 3600,
        minutes = (time - hours * 3600) / 60,
        seconds = (time - hours * 3600 - minutes * 60);

  return Time(time, hours, minutes, seconds);
}

void build() {
  GP.BUILD_BEGIN(GP_DARK, 500);
  GP.PAGE_TITLE("Led Strip Controls");

  GP.JS_BEGIN();
  GP.SEND("document.body.insertAdjacentHTML(\"afterbegin\",\"<style>input[type=\\\"range\\\"],input[type=\\\"number\\\"]{max-width:none!important;width:100%!important}.inlBlock{width: 100%}</style>\")");
  GP.JS_END();

  GP.BOX_BEGIN();
  GP.LABEL("Board Led");
  GP.SWITCH("swh_boardLed", 0);
  GP.BOX_END();

  GP.BOX_BEGIN();
  GP.LABEL("Power");
  GP.SWITCH("swh_power", power);
  GP.BOX_END();

  GP.BREAK();
  GP.HR();
  GP.BREAK();

  GP.BOX_BEGIN();
  GP.LABEL("Mode");
  GP.SELECT("slct_mode", "Animated Rainbow,Animated Rainbow Flame,Animated Flame,Solid Color", settings.mode);
  GP.BOX_END();

  GP.BOX_BEGIN();
  GP.LABEL("Color");
  GP.BOX_BEGIN(GP_RIGHT, "300px");
  GP.SLIDER("slr_hue", settings.hue, 0, 255);
  GP.BOX_END();
  GP.BOX_END();

  GP.BOX_BEGIN();
  GP.LABEL("Brightness");
  GP.BOX_BEGIN(GP_RIGHT, "300px");
  GP.SLIDER("slr_brightness", settings.brightness, 0, 50);
  GP.BOX_END();
  GP.BOX_END();

  GP.BOX_BEGIN();
  GP.LABEL("Animation Speed");
  GP.BOX_BEGIN(GP_RIGHT, "200px");
  GP.SPINNER("spr_animationSpeed", settings.animationSpeed, -128, 127);
  GP.BOX_END();
  GP.BOX_END();

  GP.BOX_BEGIN();
  GP.LABEL("Auto Power On in Seconds");
  GP.BOX_BEGIN(GP_RIGHT, "200px");
  GP.SPINNER("spr_autoPowerOnSec", settings.autoPowerOnSec, 0, 86400);
  GP.BOX_END();
  GP.BOX_END();

  GP.BREAK();
  GP.HR();
  GP.BREAK();

  GP.BOX_BEGIN();
  GP.BUTTON_MINI("btn_get_time", "Get Time");
  GP.BUTTON_MINI("btn_format_fs", "Reset settings");
  GP.BOX_END();

  GP.AREA_LOG(5, 1000, "100%");

  GP.BUILD_END();
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  LittleFS.begin();
  byte settingsStatus = settingsData.read();

  // Serial.begin(115200);

  FastLED.addLeds<WS2811, LED_STRIP_PIN, GRB>(leds, LED_AM).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(settings.brightness);

  WiFi.mode(WIFI_STA);
  WiFi.begin(AP_SSID, AP_PASS);

  // Serial.print("Connecting to WiFI...");
  FastLED.clear();

  for(int i = 0; WiFi.status() != WL_CONNECTED; i++) {    
    // Serial.print(".");
    if(digitalRead(LED_PIN) == LOW) digitalWrite(LED_PIN, HIGH);
    else digitalWrite(LED_PIN, LOW);

    leds[i] = CHSV(i, 255, 255);
    FastLED.show();

    if(i >= LED_AM) {
      i = 0;
      FastLED.clear();
    }

    delay(500);
  }

  FastLED.clear();
  digitalWrite(LED_PIN, LOW);

  // Serial.println();
  // Serial.println(WiFi.localIP());

  portal.start();
  portal.attachBuild(build);
  portal.attach(action);
  portal.log.start();
  portal.enableOTA(OTA_LOGIN, OTA_PASS);

  portal.log.println();
  
  portal.log.printf("Settings read status: %d\n", settingsStatus);

  configTime(UTS_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
  updateTime();

  Time tm = getTime();
  portal.log.printf("%d:%d:%d\n", tm.hours, tm.minutes, tm.seconds);
}

void action() {
  if(portal.click()) {

    if(portal.click("swh_boardLed")) {
      if(portal.getBool() == 1) digitalWrite(LED_PIN, HIGH);
      else digitalWrite(LED_PIN, LOW);
      ledBlink();
    }    
    else if(portal.click("swh_power")) {
      power = portal.getBool();
      if(power == 0) {
        FastLED.clear();
        FastLED.show();
      }
      ledBlink();
    }
    else if(portal.click("slct_mode")) {
      settings.mode = portal.getInt();
      settingsData.update();
      ledBlink();
    }
    else if(portal.click("slr_hue")) {
      settings.hue = portal.getInt();
      settingsData.update();
      ledBlink();
    }
    else if(portal.click("slr_brightness")) {
      settings.brightness = portal.getInt();
      FastLED.setBrightness(settings.brightness);
      settingsData.update();
      ledBlink();
    }
    else if(portal.click("spr_animationSpeed")) {
      settings.animationSpeed = portal.getInt();
      settingsData.update();
      ledBlink();
    }
    else if(portal.click("spr_autoPowerOnSec")) {
      settings.autoPowerOnSec = portal.getInt();
      settingsData.update();
      ledBlink();
    }
    else if(portal.click("btn_get_time")) {
      Time tm = getTime();
      portal.log.printf("%d:%d:%d = %d\n", tm.hours, tm.minutes, tm.seconds, tm.time);
    }
    else if(portal.click("btn_format_fs")) {
      if(LittleFS.format()) portal.log.println("Settings reset");
      else portal.log.println("Something went wrong");
    }
  }
}

void loop() {
  portal.tick();
  if(settingsData.tick() == FD_WRITE) portal.log.println("Settings updated");
  if(getTime().time == settings.autoPowerOnSec && power == 0) power = 1;
  if(power == 1) {
    switch(settings.mode) {
      case 0:
        fill_rainbow(leds, LED_AM, animationFrame, 1);
        moveAnimationFrame(-1);
        break;

      case 1:
        for(int i = 0; i < LED_AM; i++) {
          byte value = inoise8(i*FIRE_STEP, animationFrame);
          leds[i] = CHSV(
            value, 255,
            map(value, 0, 255, MIN_VALUE, MAX_VALUE)
          );
        }

        moveAnimationFrame(1);
        break;

      case 2:
        for(int i = 0; i < LED_AM; i++) {
          byte value = inoise8(i*FIRE_STEP, animationFrame);
          leds[i] = getFlameColor(value);
        }

        moveAnimationFrame(1);
        break;

      case 3:
        fill_solid(leds, LED_AM, CHSV(settings.hue, 255, 255));
        break;
    }

    FastLED.show();
  }
}

void moveAnimationFrame(int direction) {
  if(direction < -1 || 1 < direction) {
    portal.log.printf("Animation direction parameter %d not in range [-1, 1]\n", direction);
    return;
  }

  if(settings.animationSpeed > 0) animationFrame += settings.animationSpeed * direction;
  else if(settings.animationSpeed < 0 && millis() % -settings.animationSpeed == 0) animationFrame += direction;
}

CHSV getFlameColor(byte value) {
  return CHSV(
    settings.hue + map(value, 0, 255, 0, HUE_GAP),
    map(value, 0, 255, MAX_SATURATION, MIN_SATURATION),
    map(value, 0, 255, MIN_VALUE, MAX_VALUE)
  );
}

void ledBlink() {
  bool state = digitalRead(LED_PIN);
  digitalWrite(LED_PIN, !state);
  delay(LED_BLINK_DELAY);
  digitalWrite(LED_PIN, state);
}