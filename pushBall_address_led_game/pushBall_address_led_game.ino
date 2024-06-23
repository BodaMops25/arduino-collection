#include <OneButton.h>
#include <Adafruit_NeoPixel.h>

#define BUTTON_PIN 13 // D13
#define ADDRESS_LED_PIN 12 // D12
#define LED_COUNT 210

#define GAME_SPEED 10 // Milliseconds step
#define GRAVITY -.5
#define BALLPUSH 1
unsigned long timer;
// OneButton btn = OneButton(BUTTON_PIN, true, true);
float ballPos = LED_COUNT / 2;
float ballSpeed;
bool isGame;

void btnClick() {
 if(!isGame) isGame = 1;
 ballSpeed += BALLPUSH;
}

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, ADDRESS_LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(9600);
  pinMode(ADDRESS_LED_PIN,OUTPUT);
  pinMode(BUTTON_PIN,INPUT_PULLUP);

  // btn.attachClick(btnClick);
  strip.begin();
}

void loop() {
  // btn.tick();

  if(digitalRead(BUTTON_PIN)) btnClick();

  if(!isGame) ballRender();
  else {
    if(millis() >= timer + GAME_SPEED) {
      timer = millis();

      ballMove();
      ballRender();
    }
  }

  Serial.print("isGame: ");Serial.print(isGame);Serial.print("; ");
  Serial.print("ballSpeed: ");Serial.print(ballSpeed);Serial.print("; ");
  Serial.print("ballPos: ");Serial.println(ballPos);
}

void ballMove() {
  ballSpeed += GRAVITY;

  if(ballPos + ballSpeed <= 0) {
    ballPos = 0;
    if(ballSpeed <= 0) ballSpeed = 0;
  }
  else if(ballPos + ballSpeed >= LED_COUNT) ballPos = LED_COUNT;
  else ballPos += ballSpeed;
}

void ballRender() {
    strip.clear();
    strip.setPixelColor(ballPos, strip.Color(255, 50, 0)); 
    strip.show();
}