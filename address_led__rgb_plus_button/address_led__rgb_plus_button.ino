#include <Button.h>
#include <Adafruit_NeoPixel.h>

// #include <OneButton.h>

#define PONT_PIN 0 // A0

#define BUTTON_PIN 2 // D3
#define LED_PIN_R 3 // D4
#define LED_PIN_G 4 // D5
#define LED_PIN_B 5 // D6

#define LED_NUM 210
#define LAS_PIN 12 // D12
// LAS = LED ADDRESS STRIP

Button btn(BUTTON_PIN);
Adafruit_NeoPixel LA_strip(LED_NUM, LAS_PIN);

int LED_mode = 0;
int rgb_values[3] = {0, 0, 0};
int pont_value = 0;
int pontRaw_prev_value = 0;

void setup() {
  pinMode(PONT_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN_R, OUTPUT);
  pinMode(LED_PIN_G, OUTPUT);
  pinMode(LED_PIN_B, OUTPUT);
  pinMode(LAS_PIN, OUTPUT);

  Serial.begin(9600);

  btn.begin();
  LA_strip.begin();

  digitalWrite(LED_PIN_R, HIGH);
}

void loop() {

  if(btn.released()) {
    setLED(moveLED_mode());
  }

  pont_value = map(analogRead(PONT_PIN), 0, 1023, 0, 255);
  rgb_values[LED_mode] = pont_value;

  for(int i = 0; i < LED_NUM; i++) LA_strip.setPixelColor(i, LA_strip.Color(rgb_values[0], rgb_values[1], rgb_values[2]));
  LA_strip.show();  
  
  // Serial.print("R: ");Serial.print(rgb_values[0]);Serial.print("; ");
  // Serial.print("G: ");Serial.print(rgb_values[1]);Serial.print("; ");
  // Serial.print("B: ");Serial.print(rgb_values[2]);Serial.print("; ");Serial.println(" --- ");
  Serial.println(analogRead(PONT_PIN));

  delay(100);
}

void setLED(int number) {
  digitalWrite(LED_PIN_R, LOW);
  digitalWrite(LED_PIN_G, LOW);
  digitalWrite(LED_PIN_B, LOW);

  if(number == 0) digitalWrite(LED_PIN_R, HIGH);
  else if(number == 1) digitalWrite(LED_PIN_G, HIGH);
  else if(number == 2) digitalWrite(LED_PIN_B, HIGH);
}

int moveLED_mode() {
  if(++LED_mode > 2) LED_mode = 0;
  return LED_mode;
}
