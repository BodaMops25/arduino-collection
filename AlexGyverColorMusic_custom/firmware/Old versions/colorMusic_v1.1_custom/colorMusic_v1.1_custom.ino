/*
   Крутейшая свето-цветомузыка на Arduino и адресной светодиодной ленте WS2812b
   Управление:
    - Однократное нажатие кнопки: смена режима
    - Удержание кнопки: калибровка нижнего порога шума
   Режимы работы (переключаются кнопкой):
    - VU meter (столбик громкости): от зелёного к красному
    - VU meter (столбик громкости): плавно бегущая радуга
    - Светомузыка по частотам: 5 полос симметрично
    - Светомузыка по частотам: 3 полосы
    - Светомузыка по частотам: 1 полоса
   Особенности:
    - Плавная анимация (можно настроить)
    - Автонастройка по громкости (можно настроить)
    - Фильтр нижнего шума (можно настроить)
    - Автокалибровка шума при запуске (можно настроить)
    - Поддержка стерео и моно звука (можно настроить)

   НАСТРОЙКА НИЖНЕГО ПОРОГА ШУМА (строки 65-71)
    - Ручная: выключаем AUTO_LOW_PASS и EEPROM_LOW_PASS, настраиваем LOW_PASS и SPEKTR_LOW_PASS вручную
    - При запуске: включаем AUTO_LOW_PASS. При подаче питания музыка должна стоять на паузе!
    - По кнопке: при удерживании кнопки 1 секунду настраивается нижний порог шума (музыку на паузу!)
    - Из памяти (ЛУЧШИЙ ВАРИАНТ): выключаем AUTO_LOW_PASS и включаем EEPROM_LOW_PASS
      - Включаем систему
      - Ставим музыку на паузу
      - Удерживаем кнопку 1 секунду
      Значения шумов будут записаны в память и САМИ загружаться при последующем запуске! Всё!

   **************************************************
   Разработано: AlexGyver
   Страница проекта: http://alexgyver.ru/colormusic/
   GitHub: https://github.com/AlexGyver/ColorMusic
*/

// --------------------------- НАСТРОЙКИ ---------------------------
// лента
#define NUM_LEDS 210        // количество светодиодов
#define BRIGHTNESS 255     // яркость (0 - 255)

// пины
#define SOUND_R A2         // аналоговый пин вход аудио, правый канал
#define SOUND_L A1         // аналоговый пин вход аудио, левый канал
#define SOUND_R_FREQ A3    // аналоговый пин вход аудио для режима с частотами (через кондер)
#define BTN_PIN A7         // кнопка переключения режимов
#define BTN_POWER_PIN 4    // кнопка вкл/выкл питания
#define LED_PIN 12         // пин DI светодиодной ленты
#define POT_GND A0         // пин земля для потенциометра

#define POWER_PIN_COLOR_PONT 10 // делает пин плюсом для регуляторов
#define COLOR_PONT_L A5    // регулятор цвета левой стороны ленты
#define COLOR_PONT_R A6    // регулятор цвета правой стороны ленты

// настройки радуги
#define RAINBOW_SPEED 6    // скорость движения радуги (чем меньше число, тем быстрее радуга)
#define RAINBOW_STEP 6     // шаг изменения цвета радуги

// отрисовка
#define MODE 2              // режим при запуске
#define MAIN_LOOP 5         // период основного цикла отрисовки (по умолчанию 5)
#define SMOOTH 0.5          // коэффициент плавности анимации VU (по умолчанию 0.5)
#define SMOOTH_FREQ 0.8     // коэффициент плавности анимации частот (по умолчанию 0.8)
#define MAX_COEF 1.8        // коэффициент громкости (максимальное равно срднему * этот коэф) (по умолчанию 1.8)
#define MAX_COEF_FREQ 1.5   // коэффициент порога для "вспышки" цветомузыки (по умолчанию 1.5)

// сигнал
#define MONO 1              // 1 - только один канал (ПРАВЫЙ!!!!! SOUND_R!!!!!), 0 - два канала
#define EXP 1.4             // степень усиления сигнала (для более "резкой" работы) (по умолчанию 1.4)
#define POTENT 1            // 1 - используем потенциометр, 0 - используется внутренний источник опорного напряжения 1.1 В

// нижний порог шумов
int LOW_PASS = 100;         // нижний порог шумов режим VU, ручная настройка
int SPEKTR_LOW_PASS = 40;   // нижний порог шумов режим спектра, ручная настройка
#define AUTO_LOW_PASS 0     // разрешить настройку нижнего порога шумов при запуске (по умолч. 0)
#define EEPROM_LOW_PASS 1   // порог шумов хранится в энергонезависимой памяти (по умолч. 1)
#define LOW_PASS_ADD 13     // "добавочная" величина к нижнему порогу, для надёжности (режим VU)
#define LOW_PASS_FREQ_ADD 3 // "добавочная" величина к нижнему порогу, для надёжности (режим частот)

// режим цветомузыки
#define SMOOTH_STEP 20          // шаг уменьшения яркости в режиме цветомузыки (чем больше, тем быстрее гаснет)
#define LOW_COLOR HUE_RED       // цвет низких частот
#define MID_COLOR HUE_GREEN     // цвет средних
#define HIGH_COLOR HUE_YELLOW   // цвет высоких

// режим стробоскопа
#define STROBE_PERIOD 20           // период вспышек, миллисекунды
#define STROBE_DUTY 30              // скважность вспышек (1 - 99) - отношение времени вспышки ко времени темноты
#define STROBE_COLOR HUE_YELLOW     // цвет стробоскопа
#define STROBE_SAT 0                // насыщенность. Если 0 - цвет будет БЕЛЫЙ при любом цвете (0 - 255)
#define STROBE_SMOOTH 100           // скорость нарастания/угасания вспышки (0 - 255)

/*
  Цвета для HSV
  HUE_RED
  HUE_ORANGE
  HUE_YELLOW
  HUE_GREEN
  HUE_AQUA
  HUE_BLUE
  HUE_PURPLE
  HUE_PINK
*/
// --------------------------- НАСТРОЙКИ ---------------------------

// --------------------- ДЛЯ РАЗРАБОТЧИКОВ ---------------------
#define MODE_AMOUNT 7      // количество режимов

// цвета (устаревшие)
#define BLUE     0x0000FF
#define RED      0xFF0000
#define GREEN    0x00ff00
#define CYAN     0x00FFFF
#define MAGENTA  0xFF00FF
#define YELLOW   0xFFFF00
#define WHITE    0xFFFFFF
#define BLACK    0x000000

#define STRIPE NUM_LEDS / 5

#define FHT_N 64         // ширина спектра х2
#define LOG_OUT 1
#include <FHT.h>         // преобразование Хартли
#include <EEPROMex.h>

#include "FastLED.h"
CRGB leds[NUM_LEDS];

#include "GyverButton.h"
GButton butt1(BTN_PIN);
GButton btn_power(BTN_POWER_PIN);

bool isPower = 1;

// градиент-палитра от зелёного к красному
DEFINE_GRADIENT_PALETTE(soundlevel_gp) {
  0,    0,    255,  0,  // green
  100,  255,  255,  0,  // yellow
  150,  255,  100,  0,  // orange
  200,  255,  50,   0,  // red
  255,  255,  0,    0   // red
};
CRGBPalette32 myPal = soundlevel_gp;

byte Rlenght, Llenght;
float RsoundLevel, RsoundLevel_f;
float LsoundLevel, LsoundLevel_f;

float averageLevel = 50;
int maxLevel = 100;
byte MAX_CH = NUM_LEDS / 2;
int hue;
unsigned long main_timer, hue_timer, strobe_timer;
float averK = 0.006, k = SMOOTH, k_freq = SMOOTH_FREQ;
byte count;
float index = (float)255 / MAX_CH;   // коэффициент перевода для палитры
boolean lowFlag;
byte low_pass;
int RcurrentLevel, LcurrentLevel;
int colorMusic[3];
float colorMusic_f[3], colorMusic_aver[3];
boolean colorMusicFlash[3], strobeUp_flag, strobeDwn_flag;
byte this_mode = MODE;
int thisBright[3], strobe_bright = 0;
unsigned int light_time = STROBE_PERIOD * STROBE_DUTY / 100;
int color_pont_l, color_pont_r;

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
// --------------------- ДЛЯ РАЗРАБОТЧИКОВ ---------------------

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(BRIGHTNESS);

  pinMode(POWER_PIN_COLOR_PONT, OUTPUT);
  digitalWrite(POWER_PIN_COLOR_PONT, HIGH);

  pinMode(13, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  pinMode(COLOR_PONT_L, INPUT);
  pinMode(COLOR_PONT_R, INPUT);

  pinMode(POT_GND, OUTPUT);
  digitalWrite(POT_GND, LOW);
  butt1.setTimeout(900);
  btn_power.setTimeout(900);

  // для увеличения точности уменьшаем опорное напряжение,
  // выставив EXTERNAL и подключив Aref к выходу 3.3V на плате через делитель
  // GND ---[10-20 кОм] --- REF --- [10 кОм] --- 3V3
  // в данной схеме GND берётся из А0 для удобства подключения
  if (POTENT) analogReference(EXTERNAL);
  else analogReference(INTERNAL);

  // жуткая магия, меняем частоту оцифровки до 18 кГц
  // команды на ебучем ассемблере, даже не спрашивайте, как это работает
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  sbi(ADCSRA, ADPS0);

  if (AUTO_LOW_PASS && !EEPROM_LOW_PASS) {         // если разрешена автонастройка нижнего порога шумов
    autoLowPass();
  }
  if (EEPROM_LOW_PASS) {                // восстановить значения шумов из памяти
    LOW_PASS = EEPROM.readInt(0);
    SPEKTR_LOW_PASS = EEPROM.readInt(2);
  }
}

void loop() {
  color_pont_l = map(analogRead(COLOR_PONT_L), 0, 1023, 0, 255),
  color_pont_r = map(analogRead(COLOR_PONT_R), 0, 1023, 0, 255);

  butt1.tick();  // обязательная функция отработки. Должна постоянно опрашиваться
  btn_power.tick();

  if(btn_power.isSingle()) isPower = !isPower;
  
  if(isPower) {
    if (butt1.isSingle()) {                                // если единичное нажатие
      if (++this_mode >= MODE_AMOUNT) this_mode = 0;      // изменить режим
    }


    if (butt1.isHolded()) {     // кнопка удержана
      digitalWrite(13, HIGH);   // включить светодиод 13 пин
      FastLED.setBrightness(0); // погасить ленту
      FastLED.clear();          // очистить массив пикселей
      FastLED.show();           // отправить значения на ленту
      delay(500);               // подождать чутка
      autoLowPass();            // измерить шумы
      delay(500);               // подождать
      FastLED.setBrightness(BRIGHTNESS);  // вернуть яркость
      digitalWrite(13, LOW);    // выключить светодиод
    }

    // кольцевое изменение положения радуги по таймеру
    if (millis() - hue_timer > RAINBOW_SPEED) {
      if (++hue >= 255) hue = 0;
      hue_timer = millis();
    }

    // главный цикл отрисовки
    if (millis() - main_timer > MAIN_LOOP) {
      // сбрасываем значения
      RsoundLevel = 0;
      LsoundLevel = 0;

      // перваые два режима - громкость (VU meter)
      if (this_mode == 0 || this_mode == 1 || this_mode == 2) {
        for (byte i = 0; i < 100; i ++) {                                 // делаем 100 измерений
          RcurrentLevel = analogRead(SOUND_R);                            // с правого
          if (!MONO) LcurrentLevel = analogRead(SOUND_L);                 // и левого каналов

          if (RsoundLevel < RcurrentLevel) RsoundLevel = RcurrentLevel;   // ищем максимальное
          if (!MONO) if (LsoundLevel < LcurrentLevel) LsoundLevel = LcurrentLevel;   // ищем максимальное
        }

        // фильтруем по нижнему порогу шумов
        RsoundLevel = map(RsoundLevel, LOW_PASS, 1023, 0, 500);
        if (!MONO)LsoundLevel = map(LsoundLevel, LOW_PASS, 1023, 0, 500);

        // ограничиваем диапазон
        RsoundLevel = constrain(RsoundLevel, 0, 500);
        if (!MONO)LsoundLevel = constrain(LsoundLevel, 0, 500);

        // возводим в степень (для большей чёткости работы)
        RsoundLevel = pow(RsoundLevel, EXP);
        if (!MONO)LsoundLevel = pow(LsoundLevel, EXP);

        // фильтр
        RsoundLevel_f = RsoundLevel * k + RsoundLevel_f * (1 - k);
        if (!MONO)LsoundLevel_f = LsoundLevel * k + LsoundLevel_f * (1 - k);

        if (MONO) LsoundLevel_f = RsoundLevel_f;  // если моно, то левый = правому

        // если значение выше порога - начинаем самое интересное
        if (RsoundLevel_f > 15 && LsoundLevel_f > 15) {

          // расчёт общей средней громкости с обоих каналов, фильтрация.
          // Фильтр очень медленный, сделано специально для автогромкости
          averageLevel = (float)(RsoundLevel_f + LsoundLevel_f) / 2 * averK + averageLevel * (1 - averK);

          // принимаем максимальную громкость шкалы как среднюю, умноженную на некоторый коэффициент MAX_COEF
          maxLevel = (float)averageLevel * MAX_COEF;

          // преобразуем сигнал в длину ленты (где MAX_CH это половина количества светодиодов)
          Rlenght = map(RsoundLevel_f, 0, maxLevel, 0, MAX_CH);
          Llenght = map(LsoundLevel_f, 0, maxLevel, 0, MAX_CH);

          // ограничиваем до макс. числа светодиодов
          Rlenght = constrain(Rlenght, 0, MAX_CH);
          Llenght = constrain(Llenght, 0, MAX_CH);

          animation();       // отрисовать
        }
      }

      // 3-5 режим - цветомузыка
      if (this_mode == 3 || this_mode == 4 || this_mode == 5) {
        analyzeAudio();
        colorMusic[0] = 0;
        colorMusic[1] = 0;
        colorMusic[2] = 0;
        // низкие частоты, выборка с 3 по 5 тон
        for (byte i = 3; i < 6; i++) {
          if (fht_log_out[i] > SPEKTR_LOW_PASS) {
            if (fht_log_out[i] > colorMusic[0]) colorMusic[0] = fht_log_out[i];
          }
        }
        // средние частоты, выборка с 6 по 10 тон
        for (byte i = 6; i < 11; i++) {
          if (fht_log_out[i] > SPEKTR_LOW_PASS) {
            if (fht_log_out[i] > colorMusic[1]) colorMusic[1] = fht_log_out[i];
          }
        }
        // высокие частоты, выборка с 11 по 30 тон
        for (byte i = 11; i < 31; i++) {
          if (fht_log_out[i] > SPEKTR_LOW_PASS) {
            if (fht_log_out[i] > colorMusic[2]) colorMusic[2] = fht_log_out[i];
          }
        }
        for (byte i = 0; i < 3; i++) {
          colorMusic_aver[i] = colorMusic[i] * averK + colorMusic_aver[i] * (1 - averK);  // общая фильтрация
          colorMusic_f[i] = colorMusic[i] * k_freq + colorMusic_f[i] * (1 - k_freq);      // локальная
          if (colorMusic_f[i] > ((float)colorMusic_aver[i] * MAX_COEF_FREQ)) {
            thisBright[i] = 255;
            colorMusicFlash[i] = 1;
          } else colorMusicFlash[i] = 0;
          if (thisBright[i] >= 0) thisBright[i] -= SMOOTH_STEP;
          if (thisBright[i] < 0) thisBright[i] = 0;
        }
        animation();
      }
      if (this_mode == 6) {
        if ((long)millis() - strobe_timer > STROBE_PERIOD) {
          strobe_timer = millis();
          strobeUp_flag = true;
          strobeDwn_flag = false;
        }
        if ((long)millis() - strobe_timer > light_time) {
          strobeDwn_flag = true;
        }
        if (strobeUp_flag) {                    // если настало время пыхнуть
          if (strobe_bright < 255)              // если яркость не максимальная
            strobe_bright += STROBE_SMOOTH;     // увелчить
          if (strobe_bright > 255) {            // если пробили макс. яркость
            strobe_bright = 255;                // оставить максимум
            strobeUp_flag = false;              // флаг опустить
          }
        }
        
        if (strobeDwn_flag) {                   // гаснем
          if (strobe_bright > 0)                // если яркость не минимальная
            strobe_bright -= STROBE_SMOOTH;     // уменьшить
          if (strobe_bright < 0) {              // если пробили мин. яркость
            strobeDwn_flag = false;
            strobe_bright = 0;                  // оставить 0
          }
        }
        animation();
      }

      FastLED.show();           // отправить значения на ленту
      FastLED.clear();          // очистить массив пикселей
      main_timer = millis();    // сбросить таймер
    }
  }
}

void animation() {
  // согласно режиму
  switch (this_mode) {
    case 0:
      count = 0;
      for (int i = (MAX_CH - 1); i > ((MAX_CH - 1) - Rlenght); i--) {
        leds[i] = ColorFromPalette(myPal, (count * index));   // заливка по палитре " от зелёного к красному"
        count++;
      }
      count = 0;
      for (int i = (MAX_CH); i < (MAX_CH + Llenght); i++ ) {
        leds[i] = ColorFromPalette(myPal, (count * index));   // заливка по палитре " от зелёного к красному"
        count++;
      }
      break;
    case 1:
      count = 0;
      for (int i = (MAX_CH - 1); i > ((MAX_CH - 1) - Rlenght); i--) {
        leds[i] = ColorFromPalette(RainbowColors_p, (count * index) / 2 - hue);  // заливка по палитре радуга
        count++;
      }
      count = 0;
      for (int i = (MAX_CH); i < (MAX_CH + Llenght); i++ ) {
        leds[i] = ColorFromPalette(RainbowColors_p, (count * index) / 2 - hue); // заливка по палитре радуга
        count++;
      }
      break;
    case 2:     // BM25 -> CUSTOM with colors
      for (int i = (MAX_CH - 1); i > ((MAX_CH - 1) - Rlenght); i--) leds[i] = CHSV(color_pont_r, 255, 255);
      for (int i = (MAX_CH); i < (MAX_CH + Llenght); i++ ) leds[i] = CHSV(color_pont_l, 255, 255);
      break;
    case 3:
      for (int i = 0; i < NUM_LEDS; i++) {
        if (i < STRIPE)          leds[i] = CHSV(HIGH_COLOR, 255, thisBright[2]);
        else if (i < STRIPE * 2) leds[i] = CHSV(MID_COLOR, 255, thisBright[1]);
        else if (i < STRIPE * 3) leds[i] = CHSV(LOW_COLOR, 255, thisBright[0]);
        else if (i < STRIPE * 4) leds[i] = CHSV(MID_COLOR, 255, thisBright[1]);
        else if (i < STRIPE * 5) leds[i] = CHSV(HIGH_COLOR, 255, thisBright[2]);
      }
      break;
    case 4:
      for (int i = 0; i < NUM_LEDS; i++) {
        if (i < NUM_LEDS / 3)          leds[i] = CHSV(HIGH_COLOR, 255, thisBright[2]);
        else if (i < NUM_LEDS * 2 / 3) leds[i] = CHSV(MID_COLOR, 255, thisBright[1]);
        else if (i < NUM_LEDS)         leds[i] = CHSV(LOW_COLOR, 255, thisBright[0]);
      }
      break;
    case 5:
      if (colorMusicFlash[2]) for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(HIGH_COLOR, 255, thisBright[2]);
      else if (colorMusicFlash[1]) for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(MID_COLOR, 255, thisBright[1]);
      else if (colorMusicFlash[0]) for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(LOW_COLOR, 255, thisBright[0]);
      else for (int i = 0; i < NUM_LEDS; i++) leds[i] = BLACK;
      break;
    case 6:
      for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(STROBE_COLOR, STROBE_SAT, strobe_bright);
      break;
  }
}

void autoLowPass() {
  // для режима VU
  delay(10);                                // ждём инициализации АЦП
  int thisMax = 0;                          // максимум
  int thisLevel;
  for (byte i = 0; i < 200; i++) {
    thisLevel = analogRead(SOUND_R);        // делаем 200 измерений
    if (thisLevel > thisMax)                // ищем максимумы
      thisMax = thisLevel;                  // запоминаем
    delay(4);                               // ждём 4мс
  }
  LOW_PASS = thisMax + LOW_PASS_ADD;        // нижний порог как максимум тишины + некая величина

  // для режима спектра
  thisMax = 0;
  for (byte i = 0; i < 100; i++) {          // делаем 100 измерений
    analyzeAudio();                         // разбить в спектр
    for (byte j = 2; j < 32; j++) {         // первые 2 канала - хлам
      thisLevel = fht_log_out[j];
      if (thisLevel > thisMax)              // ищем максимумы
        thisMax = thisLevel;                // запоминаем
    }
    delay(4);                               // ждём 4мс
  }
  SPEKTR_LOW_PASS = thisMax + LOW_PASS_FREQ_ADD;  // нижний порог как максимум тишины

  if (EEPROM_LOW_PASS && !AUTO_LOW_PASS) {
    EEPROM.updateInt(0, LOW_PASS);
    EEPROM.updateInt(2, SPEKTR_LOW_PASS);
  }
}

void analyzeAudio() {
  for (int i = 0 ; i < FHT_N ; i++) {
    int sample = analogRead(SOUND_R_FREQ);
    fht_input[i] = sample; // put real data into bins
  }
  fht_window();  // window the data for better frequency response
  fht_reorder(); // reorder the data before doing the fht
  fht_run();     // process the data in the fht
  fht_mag_log(); // take the output of the fht
}
