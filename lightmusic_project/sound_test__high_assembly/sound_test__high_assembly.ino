#define MAIN_LOOP 5         // период основного цикла отрисовки (по умолчанию 5)
#define MAX_COEF 1.8      // коэффициент громкости (максимальное равно срднему * этот коэф) (по умолчанию 1.8)

#define SOUND_R A2         // аналоговый пин вход аудио, правый канал
#define SOUND_L A1         // аналоговый пин вход аудио, левый канал
#define SOUND_R_FREQ A3    // аналоговый пин вход аудио для режима с частотами (через кондер)
#define POT_GND A0         // пин земля для потенциометра

// сигнал
#define MONO 1              // 1 - только один канал (ПРАВЫЙ!!!!! SOUND_R!!!!!), 0 - два канала
#define EXP 1.4             // степень усиления сигнала (для более "резкой" работы) (по умолчанию 1.4)
#define POTENT 1            // 1 - используем потенциометр, 0 - используется внутренний источник опорного напряжения 1.1 В

// нижний порог шумов
int LOW_PASS = 100;         // нижний порог шумов режим VU, ручная настройка
int SPEKTR_LOW_PASS = 40;   // нижний порог шумов режим спектра, ручная настройка
#define AUTO_LOW_PASS 0     // разрешить настройку нижнего порога шумов при запуске (по умолч. 0)
#define LOW_PASS_ADD 13     // "добавочная" величина к нижнему порогу, для надёжности (режим VU)
#define LOW_PASS_FREQ_ADD 3 // "добавочная" величина к нижнему порогу, для надёжности (режим частот)

byte Rlenght, Llenght;
float RsoundLevel, RsoundLevel_f;
float LsoundLevel, LsoundLevel_f;

float averageLevel = 50;
int maxLevel = 100;
unsigned long main_timer;
float averK = 0.006, k = 0.5, k_freq = 0.8;
byte count;
boolean lowFlag;
byte low_pass;
int RcurrentLevel, LcurrentLevel;

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

void setup() {
  Serial.begin(9600);

  pinMode(POT_GND, OUTPUT);
  digitalWrite(POT_GND, LOW);

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

  if (AUTO_LOW_PASS) {          // если разрешена автонастройка нижнего порога шумов
    autoLowPass();
  }
}

void loop() {

  // главный цикл отрисовки
  if (millis() - main_timer > MAIN_LOOP) {
    // сбрасываем значения
    RsoundLevel = 0;
    LsoundLevel = 0;

    // перваые два режима - громкость (VU meter)
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

      Rlenght = map(RsoundLevel_f, 0, maxLevel, 0, 100);
      Llenght = map(LsoundLevel_f, 0, maxLevel, 0, 100);

      Rlenght = constrain(Rlenght, 0, 100);
      Llenght = constrain(Llenght, 0, 100);

      Serial.print(Rlenght);Serial.print(',');
      // Serial.println(Rlenght);
    }

    main_timer = millis();    // сбросить таймер
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
}