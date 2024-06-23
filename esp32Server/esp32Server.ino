#define LED_PIN 2

#define AP_SSID "NETGEAR82"
#define AP_PASS "notsmoothpanda742"

#include <GyverPortal.h>
GyverPortal portal;

byte mode = 0;

// конструктор страницы
void build() {
  GP.BUILD_BEGIN();
  GP.THEME(GP_DARK);

  GP.PAGE_TITLE("ESP32 Server");

  GP.BOX_BEGIN();

  GP.BUTTON("btn_led_off", "Turn led off");
  GP.BUTTON("btn_led_on", "Turn led on");

  GP.BOX_END();

  GP.LABEL("Mode:");

  GP.BOX_BEGIN();

  GP.SLIDER("sld_mode", mode, 0, 5);
  GP.BUTTON_MINI("btn_sld_mode", "Set", "sld_mode");

  GP.BOX_END();

  GP.BUILD_END();
}

void setup() {
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(AP_SSID, AP_PASS);

  Serial.print("Connecting to WiFI...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println(WiFi.localIP());

  portal.list.init(1);
  portal.list.add(&mode, "btn_sld_mode", T_INT);

  portal.start();
  portal.attachBuild(build);
  portal.attach(action);
}

void action() {
  if(portal.click()) {
    if(portal.click("btn_led_on")) digitalWrite(LED_PIN, HIGH);
    else if(portal.click("btn_led_off")) digitalWrite(LED_PIN, LOW);

    if(portal.click("btn_sld_mode")) Serial.printf("mode: %d\n", mode);
  }
}

void loop() {
  portal.tick();
}