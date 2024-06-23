#define RX_PIN 5
#define TX_PIN 6

#include <stdlib.h>
#include <string.h>
// #include "GParser.h"
#include "SoftwareSerial.h"
#include "mString.h"

SoftwareSerial BTSerial(RX_PIN, TX_PIN);

// бегущие частицы 1D, версия с FastLED

#define STRIP_PIN 2     // пин ленты
#define SOUND_PIN A0    // пин звука

#define MAX_VOL 255

#define LEDS_AM 228     // количество светодиодов
// #define COLOR_STEP 151  // шаг цвета, интересные 151, 129
// #define P_SPEED 2       // скорость движения

byte COLOR_STEP = 151;
byte P_SPEED = 2;
bool POWER = true;

#include <FastLED.h>
CRGB leds[LEDS_AM];

#include "VolAnalyzer.h"
VolAnalyzer sound(SOUND_PIN);

byte curColor = 0;      // текущий цвет

void setup() {
  FastLED.addLeds<WS2812, STRIP_PIN, GRB>(leds, LEDS_AM);
  FastLED.setBrightness(10);

  sound.setTrsh(80);

  // настройки анализатора звука
  sound.setVolK(15);        // снизим фильтрацию громкости (макс. 31)
  sound.setVolMax(MAX_VOL); // выход громкости 0-255
  sound.setPulseMax(200);   // сигнал пульса
  sound.setPulseMin(150);   // перезагрузка пульса

  pinMode(RX_PIN, INPUT);
  pinMode(TX_PIN, OUTPUT);

  Serial.begin(9600);
  BTSerial.begin(9600);
}

mString<64> BTBuffer;
bool isBTTransmitted = false;
bool isPauseCommand = false;

const char* CONTROLS[] = {
  "l13",
  "power",
  "color-step",
  "speed",
  "brightness",
  "sound-threshold",
  "vol-k",
  "vol-max",
  "pulse-max",
  "pulse-min",
  "mode",
};

byte MODE = 0;

void loop() {

  while(Serial.available()) {
    if(POWER == true) POWER = false;

    byte symbol = Serial.read();

    if(symbol == 59 || symbol == 13 || symbol == 10 || symbol == 0) {
      isBTTransmitted = true;
      Serial.print("Command received: ");Serial.print(BTBuffer.buf);Serial.println(";");
      break;
    }

    if(32 <= symbol && symbol <= 126) BTBuffer += (char)symbol;
  }

  // RECEIVING BL DATA

  while(BTSerial.available()) {
    if(POWER == true) POWER = false;

    byte symbol = BTSerial.read();

    if(symbol == 59 || symbol == 13 || symbol == 10 || symbol == 0) {
      isBTTransmitted = true;
      Serial.print("Command received: ");Serial.print(BTBuffer.buf);Serial.println(";");
      BTSerial.print("Command received: ");BTSerial.print(BTBuffer.buf);BTSerial.println(";");
      break;
    }

    if(32 <= symbol && symbol <= 126) BTBuffer += (char)symbol;
  }

  // LOGIC BY PARSED BL DATA

  if(isBTTransmitted) {

    // GParser data(BTBuffer.buf, ',');
    char* commands[16];
    byte commandAmount = BTBuffer.split(commands, ',');

    for(byte i = 0; i < commandAmount; i++) {

      mString<64> command;
      command = commands[i];

      if(command == "l13") {

        if(atoi(commands[i+1]) == 1) digitalWrite(13, HIGH);
        else digitalWrite(13, LOW);
      }
      else if(command == "power") {
        if(atoi(commands[i+1]) != 0) {
          POWER = true;
          isPauseCommand = false;
        }
        else {
          POWER = false;
          FastLED.clear();
          FastLED.show();
        }
      }
      else if(command == "pause") {
        POWER = false;
        isPauseCommand = true;
      }
      else if(command == "color-step") {
        COLOR_STEP = atoi(commands[i+1]);
      }
      else if(command == "speed") {
        P_SPEED = constrain(atoi(commands[i+1]), 0, 32767);
      }
      else if(command == "brightness") {
        FastLED.setBrightness(
          constrain(atoi(commands[i+1]), 0, 255)
        );
      }
      else if(command == "sound-threshold") {
        sound.setTrsh(atoi(commands[i+1]));
      }
      else if(command == "vol-k") {
        sound.setVolK(
          constrain(atoi(commands[i+1]), 0, 31)
        );
      }
      else if(command == "vol-max") {
        sound.setVolMax(
          constrain(atoi(commands[i+1]), 0, 255)
        );
      }
      else if(command == "pulse-max") {
        sound.setPulseMax(atoi(commands[i+1]));
      }
      else if(command == "pulse-min") {
        sound.setPulseMin(atoi(commands[i+1]));
      }
      else if(command == "cur-color") {
        curColor = constrain(atoi(commands[i+1]), 0, 255);
      }
      else if(command == "mode") {
        MODE = atoi(commands[i+1]);
      }
    }

    isBTTransmitted = false;
    BTBuffer = "";
    if(!isPauseCommand) POWER = true;
  }

  // OTHER PROGRAM

  if(POWER) {

    if(sound.tick()) {

      if(MODE == 0) {   // если анализ звука завершён (~10мс)
        // перематываем массив светодиодов на P_SPEED вправо
        for (int k = 0; k < P_SPEED; k++) {
          for (int i = LEDS_AM - 1; i > 0; i--) leds[i] = leds[i - 1];
        }
        
        // резкий звук - меняем цвет
        if (sound.pulse()) curColor += COLOR_STEP;
        
        // берём текущий цвет с яркостью по громкости (0-255)
        CRGB color = CHSV(curColor, 255, sound.getVol());
        
        // красим P_SPEED первых светодиодов
        for (int i = 0; i < P_SPEED; i++) leds[i] = color;
      }

      else if(MODE == 1) {
        FastLED.clear();
        for(int i = 0; i < map(sound.getVol(), 0, LEDS_AM, 0, MAX_VOL); i++) leds[i] = CRGB::Green;
      }

      // выводим
      FastLED.show();
    }
  }
}