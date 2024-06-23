#define AP_SSID "NETGEAR82"
#define AP_PASS "notsmoothpanda742"

// NTP

#define NTP_SERVER "in.pool.ntp.org"
#define UTS_OFFSET_SEC 3600
#define DAYLIGHT_OFFSET_SEC 0

#include <WiFi.h>
#include "time.h"

unsigned long timestampSec = 0;
// unsigned long timestampSec = 23*3600 + 59*60 + 40;
struct Time {
  byte hours, minutes, seconds;
  Time(byte hours, byte minutes, byte seconds) : hours(hours), minutes(minutes), seconds(seconds) {}
};

void updateTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)) return;

  timestampSec = timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60 + timeinfo.tm_sec;
}

Time getTime() {
  unsigned long time = (timestampSec + (millis() / 1000)) % (3600 * 24);

  byte hours = time / 3600,
        minutes = (time - hours * 3600) / 60,
        seconds = (time - hours * 3600 - minutes * 60);

  return Time(hours, minutes, seconds);
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(AP_SSID, AP_PASS);

  Serial.print("Connecting to WiFI...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  configTime(UTS_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
  updateTime();
}

void loop() {
  delay(1000);
  Time tm = getTime();
  Serial.printf("%d:%d:%d\n", tm.hours, tm.minutes, tm.seconds);
}