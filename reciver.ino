#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

// Инициализируем радио-модуль на пинах 9 и 10
RF24 radio(9, 10);  

// Зуммер
int zoom = 2;
// Красный LED
int ledPin = 4;
// Кнопка
int btn = 7;

bool isAlarm = false;
// Сигнал внимание от передатчика - 1
char alarmSignal[1] = {"1"}; 

char checkConnectSignal[1] = {"2"};

// Каналы для получения данных
byte address[][6] = {"1Channel", "2Channel", "3Channel", "4Channel", "5Channel", "6Channel"};

// Инициализация
void setup() {
  Serial.begin(9600); // Порт для связи с ПК
  
  // Инициализация digital пинов
  pinMode(zoom, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(btn, INPUT);
  
  // "Приветствие" при включении платы
  wealcome();
  // Инициализация радио-модуля и его настройка на прием
  radioInit();
}

// Главный цикл программы
void loop() {
  byte pipeNo;
  char signalOfTransmitter[1];

  // Остновка тревоги если нажали на кнопку
  if (digitalRead(btn)) {
    alarmStop();
  }

  // Включение тревоги
  if (isAlarm) {
    alarmStart();
  }

  while (radio.available(&pipeNo)) {        // слушаем эфир со всех труб
    radio.read(&signalOfTransmitter, sizeof(signalOfTransmitter));  // чиатем входящий сигнал

    bool hasValue = isRecive(signalOfTransmitter, alarmSignal, 0);
    bool hasCheckValue = isRecive(signalOfTransmitter, checkConnectSignal, 0);

    Serial.println("_________________");

    if (hasValue) {
      Serial.println("if call");
      // Установка переменной для включения тревоги
      isAlarm = true;
    }

    if (hasCheckValue) {
      Serial.println("check connect");
      led(true);
      delay(10);
      led(false);
    }
  }
}



// Проверка на получение сообщения от передатчика
bool isRecive(char a[], char b[], int i) {
  return a[i] == b[i];
}

/**
Метод запуска тревоги
Включение "сирены" при получении уведомления от передатчика
*/
void alarmStart() {
  led(true);
  // увеличиваем частоту звука
   for (int x = 300; x < 2000 ; x++) {
     tone (zoom, x);
     delay(1);
    }
   // уменьшаем частоту звука
   for (int x = 2000; x > 300 ; x--) {
     tone (zoom, x);
     delay(1);
    }
}

/**
Метод остановки тревоги
*/
void alarmStop() {
  isAlarm = false;
  led(false);
  noTone(zoom);
  Serial.println("reset");
  asm("JMP 0");
}

/**
Звук приветствия
Запускает зуммер последовательно с разной тональностью
*/
void wealcome() {
  led(true);
  tone(zoom, 500);
  delay(200);
  tone(zoom, 1000);
  delay(200);
  tone(zoom, 1500);
  delay(200);
  noTone(zoom);
  led(false);
}

/**
Управление LED
*/
void led(bool on) {
  digitalWrite(ledPin, on);
}

/**
Метод инициализации радио-модуля и его настройки
*/
void radioInit() {
  // Активация модуля
  radio.begin();

  if (radio.isPVariant()) {
    Serial.println("Модуль приемника nRF24L01 подключен");
  } else {
    Serial.println("Неизвестный модуль приемника");
  }

  // режим подтверждения приёма, 1 вкл 0 выкл
  radio.setAutoAck(0);    
  // (время между попыткой достучаться, число попыток)    
  radio.setRetries(0, 15);
  // Размер пакета, в байтах
  radio.setPayloadSize(16);
  // Слушаем канал 0
  radio.openReadingPipe(1, address[0]);   
  // Выбираем канал (в котором нет шумов!)
  radio.setChannel(0x60);
  // Уровень мощности передатчика.
  // Варианты: RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setPALevel (RF24_PA_MAX);
  // Скорость обмена. Варианты: RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
  // Скорость должна быть одинакова на приёмнике и передатчике!
  // При самой низкой скорости имеем самую высокую чувствительность и дальность.
  radio.setDataRate (RF24_250KBPS); 
  // Начать работу
  radio.powerUp();
  // Начинаем слушать эфир, мы приёмный модуль        
  radio.startListening(); 

  Serial.println("START!");
}