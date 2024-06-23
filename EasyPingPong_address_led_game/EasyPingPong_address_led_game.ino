// LIBRARIES

#include <Adafruit_NeoPixel.h>
#include <OneButton.h>

// PINS

#define ADDRESS_LED_PIN 12 // D12
#define BTN_L_PIN 2 // D2
#define BTN_R_PIN 3 // D3
#define BUZZ_PIN 13 // D13

// ADDRESS LED

#define LED_NUM 84
#define LED_BRIGHTNESS 10

// GAME

#define goalStep 10
#define goalsForWin 5
#define speedUpStep .01

float ballPos = LED_NUM / 2;
float ballSpeed = .05;
bool ballDirection;

int goalsLeft = 0;
int goalsRight = 0;

// OTHER

OneButton btn_L = OneButton(BTN_L_PIN, true, true);
OneButton btn_R = OneButton(BTN_R_PIN, true, true);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_NUM, ADDRESS_LED_PIN, NEO_GRB + NEO_KHZ800);

unsigned long timer = 0;
unsigned int buzzTune = 400;
bool isBuzz = 0;
bool isFinalBuzz = 0;
bool isGame = 0;

void setup() {
  // Serial.begin(9600);

  btn_L.attachClick(btn_L_click);
  btn_R.attachClick(btn_R_click);

  strip.begin();
  strip.setBrightness(LED_BRIGHTNESS);
}

void loop() {
  btn_L.tick();
  btn_R.tick();

  buzzTick();

  if(isGame) {
    if(goalsLeft < 5 && goalsRight < 5) ballMove();
    else if(!isFinalBuzz) {
      makeOneBuzz(800, 500);
      isFinalBuzz = 1;
    }
  }

  render();

  // Serial.print("LeftGoals: ");Serial.print(goalsLeft);Serial.print("; ");
  // Serial.print("RightGoals: ");Serial.print(goalsRight);Serial.print("; ");
  // Serial.print("BallPos: ");Serial.println(ballPos);
}

void btn_L_click() {
  if(!isGame) isGame = 1;

  if(ballPos <= goalStep) {
    ballDirection = 0;
    ballSpeed += speedUpStep;
  }
  
  makeOneBuzz(50, 400);
  // Serial.println("Left click!");
}
void btn_R_click() {
  if(!isGame) isGame = 1;  

  if(LED_NUM - goalStep <= ballPos) {
    ballDirection = 1;
    ballSpeed += speedUpStep;
  }

  makeOneBuzz(50, 400);
  // Serial.println("Right click!");
}

// BUZZER STUFF

void buzzTick() {
  if(millis() >= timer) {isBuzz = 0;}

  if(isBuzz) tone(BUZZ_PIN, buzzTune);
  else noTone(BUZZ_PIN);
}

void makeOneBuzz(int time_MS, unsigned int tune) {
  buzzTune = tune;
  timer = millis() + time_MS;
  isBuzz = 1;
  tone(BUZZ_PIN, buzzTune);
}

// BALL LOGIC

void ballMove() {
  if(ballDirection) ballPos -= ballSpeed;
  else ballPos += ballSpeed;

  if(ballPos <= 0) {
    goalsLeft++;
    ballPos = 0;
    ballDirection = 0;
  }
  else if(LED_NUM <= ballPos) {
    goalsRight++;
    ballPos = LED_NUM;
    ballDirection = 1;
  }
}

void render() {
  strip.clear();

  strip.setPixelColor(goalStep - 1, strip.Color(0, 0, 255));
  strip.setPixelColor(LED_NUM - goalStep, strip.Color(255, 0, 0));

  for(int i = 0; i < goalsLeft; i++) {
    strip.setPixelColor(LED_NUM / 2 + i, strip.Color(255, 0, 0));
  }
  for(int i = 0; i < goalsRight; i++) {
    strip.setPixelColor(LED_NUM / 2 - 1 - i, strip.Color(0, 0, 255));
  }

  strip.setPixelColor(ballPos, strip.Color(255, 255, 0));

  strip.show();
}