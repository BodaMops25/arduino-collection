// вывод гармонического шума, наложенного на громкость

#define STRIP_PIN 2     // пин ленты
#define SOUND_PIN A0    // пин звука

#define SHOW_MAX 1          // 1/0 - показывать плавающие точки максимума
#define COLOR_MULT -5      // шаг изменения цветовой палитры столбика
#define COLOR_STEP 1        // шаг движения палитры столбика (по времени)
#define COLON_SIZE 50       // высота матрицы
#define COLON_AMOUNT 6      // ширина матрицы

#define COLOR_DEBTH 3
#include <microLED.h>
microLED < COLON_SIZE * COLON_AMOUNT, STRIP_PIN, -1, LED_WS2812, ORDER_GRB, CLI_AVER > strip;

#include <FastLED.h>

#include "VolAnalyzer.h"
VolAnalyzer sound(SOUND_PIN);

byte volume[COLON_AMOUNT];
int maxs[COLON_AMOUNT];
byte colorCount = 0;
int16_t noise;

void setup() {
  strip.setBrightness(255);     // яркость ленты
  sound.setVolMax(60);
  sound.setVolK(30);
}

void loop() {
  if (sound.tick()) {   // если анализ звука завершён (~10мс)
    strip.clear();      // чистим ленту

    for (int i = 0; i < COLON_AMOUNT; i++) {
      // домножаем шум на громкость
      // также я домножил на 2, чтобы шум был более амплитудным
      int val = inoise8(noise - i * 100) * 2 * sound.getVol() / 100;
      
      // ограничиваем и масштабируем до половины высоты столбика
      val = constrain(val, 0, 255);
      val = map(val, 0, 255, 0, COLON_SIZE / 2);
      volume[i] = val;
    }

    noise += 90;    // двигаем шум (скорость бокового движения картинки)
    colorCount += COLOR_STEP;   // двигаем цвет
    
    // двигаем точки максимумов
    for (int i = 0; i < COLON_AMOUNT; i++) {
      if (maxs[i] < volume[i] * 10) maxs[i] = (volume[i] - 1) * 10;
      else maxs[i] -= 2;
      if (maxs[i] < 0) maxs[i] = 0;
    }
    
    // выводим
    for (int col = 0; col < COLON_AMOUNT; col++) {      // по столбикам
      for (int i = 0; i < volume[col]; i++) {           // для текущей громкости
        mData color = mWheel8(colorCount + i * COLOR_MULT);
        strip.leds[COLON_SIZE * col + COLON_SIZE / 2 + i] = color;      // вверх от центра
        strip.leds[COLON_SIZE * col + COLON_SIZE / 2 - 1 - i] = color;  // вниз от центра
      }
      
      // отображаем точки максимумов, если включены и больше 0
      if (SHOW_MAX && maxs[col] > 0) {
        strip.leds[COLON_SIZE * col + COLON_SIZE / 2 + maxs[col] / 10] = mGreen;      // вверх от центра
        strip.leds[COLON_SIZE * col + COLON_SIZE / 2 - 1 - maxs[col] / 10] = mGreen;  // вниз от центра
      }
    }
    
    // обновляем ленту
    strip.show();
  }
}