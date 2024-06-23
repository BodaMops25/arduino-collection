// ASSEMBLY JUST WITH MICROPHONE, NOTHING MORE

#define SOUND_PIN A0    // пин звука

#include "VolAnalyzer.h"
VolAnalyzer sound(SOUND_PIN);

void setup() {

  sound.setTrsh(80);

  // настройки анализатора звука
  sound.setVolK(15);        // снизим фильтрацию громкости (макс. 31)
  sound.setVolMax(100);     // выход громкости 0-100
  sound.setPulseMax(200);   // сигнал пульса
  sound.setPulseMin(150);   // перезагрузка пульса

  Serial.begin(9600);
}

void loop() {

  if(sound.tick()) {
    Serial.print(sound.getVol());Serial.print(',');
  }
}