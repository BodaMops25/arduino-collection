#define RX_PIN 2
#define TX_PIN 3

#define MIC_PIN A0
#define LED_PIN 12

#define LOW_THRESHOLD_ITERATIONS 500
#define IS_AUTO_THRESHOLD 1

#define LED_AM 298
// #define LED_AM 228
#define LED_SIDE_AM LED_AM / 2

#include <stdlib.h>
#include <string.h>
#include "mString.h"
#include "SoftwareSerial.h"

SoftwareSerial BTSerial(RX_PIN, TX_PIN);

mString<64> DBuffer;
bool isBTTransmitted = false;

#include "FastLED.h"
CRGB leds[LED_AM];

#include "VolAnalyzer.h"
VolAnalyzer sound(MIC_PIN);

byte brightness = 255;
float smothness_k = .5;
float exp_k = 1.4;
bool power = true;

byte mode = 0;
byte colorMode = 1;

byte color = 28;
int colorStep = ceil((float)LED_AM / 255);
int dynamicRainbowSpeed = 1;
int mic_delta = 270;                              // разница потенциала микрофона между самым тихим и самым громким звуком
bool isSymmetrical = true;

// mode 1 variables

int pixel_speed = 1;

// program variables

short int lowThreshold = 200;
float volFiltered = 0;

void setup() {
  Serial.begin(9600);
  BTSerial.begin(9600);

  FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, LED_AM).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(brightness);

  sound.setTrsh(80);

  // настройки анализатора звука
  sound.setVolK(15);        // снизим фильтрацию громкости (макс. 31)
  sound.setVolMax(255); // выход громкости 0-255
  sound.setPulseMax(200);   // сигнал пульса
  sound.setPulseMin(150);   // перезагрузка пульса

  if(IS_AUTO_THRESHOLD) autoLowThreshold();
}

int colorShift = 0;

void loop() {

  while(Serial.available()) {

    byte symbol = Serial.read();

    if(symbol == 59 || symbol == 13 || symbol == 10 || symbol == 0) {
      isBTTransmitted = true;
      Serial.print("Command received: ");Serial.print(DBuffer.buf);Serial.println(";");
      break;
    }

    if(32 <= symbol && symbol <= 126) DBuffer += (char)symbol;
  }

  // RECEIVING BL DATA

  while(BTSerial.available()) {

    byte symbol = BTSerial.read();

    if(symbol == 59 || symbol == 13 || symbol == 10 || symbol == 0) {
      isBTTransmitted = true;
      Serial.print("Command received: ");Serial.print(DBuffer.buf);Serial.println(";");
      BTSerial.print("Command received: ");BTSerial.print(DBuffer.buf);BTSerial.println(";");
      break;
    }

    if(32 <= symbol && symbol <= 126) DBuffer += (char)symbol;
  }

  // LOGIC BY PARSED BL DATA

  if(isBTTransmitted) {

    char* commands[16];
    byte commandAmount = DBuffer.split(commands, ',');

    bool isPowerStop = false;

    for(byte i = 0; i < commandAmount; i++) {
      mString<64> command;
      command = commands[i];

      if(command == "l13") {
        if(atoi(commands[i+1]) == 1) digitalWrite(13, HIGH);
        else digitalWrite(13, LOW);
      }
      else if(command == "mode") {
        mode = atoi(commands[i+1]);
      }
      else if(command == "color-mode") {
        colorMode = atoi(commands[i+1]);
      }
      else if(command == "color") {
        color = constrain(atoi(commands[i+1]), 0, 255);
      }
      else if(command == "color-step") {
        colorStep = atoi(commands[i+1]);
      }
      else if(command == "brightness") {
        brightness = constrain(atoi(commands[i+1]), 0, 255);
      }
      else if(command == "mic-delta") {
        mic_delta = atoi(commands[i+1]);
      }
      else if(command == "power") {
        if(atoi(commands[i+1]) == 1) power = true;
        else {
          power = false;
          isPowerStop = true;
        }
      }
      else if(command == "main-threshold") {
        lowThreshold = atoi(commands[i+1]);
      }
      else if(command == "sound-threshold") {
        sound.setTrsh(atoi(commands[i+1]));
      }
      else if(command == "rainbow-speed") {
        dynamicRainbowSpeed = atoi(commands[i+1]);
      }
      else if(command == "smoothness-k") {
        smothness_k = atof(commands[i+1]);
      }
      else if(command == "exp-k") {
        exp_k = atof(commands[i+1]);
      }
      if(command == "symmetrical") {
        if(atoi(commands[i+1]) == 1) isSymmetrical = true;
        else isSymmetrical = false;
      }
    }

    isBTTransmitted = false;
    DBuffer = "";
  }

  if(power) {

    colorShift += dynamicRainbowSpeed;

    int pixels_am = LED_AM;
    if(isSymmetrical) pixels_am = LED_SIDE_AM;

    switch(mode) {
      case 0: {
        FastLED.clear();

        int curVol = volMax(50);
        int maxRawVol = lowThreshold + mic_delta;

        curVol = map(curVol, lowThreshold, maxRawVol, 0, maxRawVol); // обработка по нижнему порогу
        curVol = constrain(curVol, 0, maxRawVol); // ограничение по нижнему порогу

        curVol = pow(curVol, exp_k); // в степень для чёткости
      
        volFiltered = curVol * smothness_k + volFiltered * (1 - smothness_k); // сглаживание

        int emmitedPixels = constrain(
          map(
            volFiltered,
            0, pow(maxRawVol, exp_k),
            0, pixels_am
          ),
          0, pixels_am
        );

        switch(colorMode) {
          case 0:
            if(isSymmetrical) {
              fill_gradient(
                leds,
                LED_SIDE_AM-1,
                CHSV(0, 255, 255),
                LED_SIDE_AM-1 - emmitedPixels,
                CHSV(emmitedPixels * colorStep * 2, 255, 255),
                BACKWARD_HUES
              );

              fill_gradient(
                leds,
                LED_SIDE_AM,
                CHSV(0, 255, 255),
                LED_SIDE_AM + emmitedPixels,
                CHSV(emmitedPixels * colorStep * 2, 255, 255),
                FORWARD_HUES
              );
            }
            else fill_rainbow(leds, emmitedPixels, 0, colorStep);
            break;

          case 1:
            if(isSymmetrical) {
              fill_gradient(
                leds,
                LED_SIDE_AM-1,
                CHSV(-colorShift, 255, 255),
                LED_SIDE_AM-1 - emmitedPixels,
                CHSV(emmitedPixels * colorStep * 2 - colorShift, 255, 255),
                BACKWARD_HUES
              );

              fill_gradient(
                leds,
                LED_SIDE_AM,
                CHSV(-colorShift, 255, 255),
                LED_SIDE_AM + emmitedPixels,
                CHSV(emmitedPixels * colorStep * 2 - colorShift, 255, 255),
                FORWARD_HUES
              );
            }
            else fill_rainbow(leds, emmitedPixels, -colorShift, colorStep);
            break;
          case 2:
            fill_solid(leds, emmitedPixels, CHSV(color, 255, brightness));
            break;
        }

        FastLED.show();
        break;
      }

      case 1: {
        if(sound.tick()) {
          for (int k = 0; k < pixel_speed; k++) {
            for(int i = LED_AM - 1; i > 0; i--) leds[i] = leds[i-1];
          }
          
          // резкий звук - меняем цвет
          if (sound.pulse()) color += colorStep;
          
          // берём текущий цвет с яркостью по громкости (0-255)
          CRGB color_fin = CHSV(color, 255, sound.getVol());
          
          // красим pixel_speed первых светодиодов
          for (int i = 0; i < pixel_speed; i++) leds[i] = color_fin;

          FastLED.show();
        }
        break;
      }

      case 2: {
        FastLED.clear();

        switch(colorMode) {
          case 0:
            if(isSymmetrical) {
              fill_gradient(
                leds,
                LED_SIDE_AM-1,
                CHSV(0, 255, 255),
                0,
                CHSV((LED_SIDE_AM-1) * colorStep * 2, 255, 255),
                BACKWARD_HUES
              );

              fill_gradient(
                leds,
                LED_SIDE_AM,
                CHSV(0, 255, 255),
                LED_AM-1,
                CHSV((LED_SIDE_AM-1) * colorStep * 2, 255, 255),
                FORWARD_HUES
              );
            }
            else fill_rainbow(leds, LED_AM, 0, colorStep);
            break;

          case 1:
            if(isSymmetrical) {
              fill_gradient(
                leds,
                LED_SIDE_AM-1,
                CHSV(-colorShift, 255, 255),
                0,
                CHSV((LED_SIDE_AM-1) * colorStep * 2 - colorShift, 255, 255),
                BACKWARD_HUES
              );

              fill_gradient(
                leds,
                LED_SIDE_AM,
                CHSV(-colorShift, 255, 255),
                LED_AM-1,
                CHSV((LED_SIDE_AM-1) * colorStep * 2 - colorShift, 255, 255),
                FORWARD_HUES
              );
            }
            else fill_rainbow(leds, LED_AM, -colorShift, colorStep);
            break;

          case 2:
            fill_solid(leds, LED_AM, CHSV(color, 255, brightness));
            break;
        }

        FastLED.show();
        break;
      }
    }
  }
}

int volMax(int iterations) {
  int maxVol = 0;

  for(int i = 0; i < iterations; i++) {
    int curVol = analogRead(MIC_PIN);
    if(curVol > maxVol) maxVol = curVol;
  }

  return maxVol;
}

void autoLowThreshold() {
  lowThreshold = volMax(LOW_THRESHOLD_ITERATIONS);
  Serial.print("Low Threshold set to: ");Serial.print(lowThreshold);Serial.println(';');

  fill_solid(leds, LED_AM, CRGB::Blue);
  FastLED.show();
  delay(500);
  FastLED.clear();
  FastLED.show();
  delay(500);

  fill_solid(leds, LED_AM, CRGB::Blue);
  FastLED.show();
  delay(500);
  FastLED.clear();
  FastLED.show();
}