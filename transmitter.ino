#include <SPI.h>          // библиотека для работы с шиной SPI
#include "nRF24L01.h"     // библиотека радиомодуля
#include "RF24.h"         // ещё библиотека радиомодуля

RF24 radio(9, 10); // "создать" модуль на пинах 9 и 10 Для Уно

int btn = 2;

int digitalHoll = 5;
int analogHoll = A5;

//Сигнал внимание (1 - приемник зажигает красный светодиод)
char alarm[1] = {'1'};
char checkConnect[1] = {'2'};

byte address[][6] = {"1Channel", "2Channel", "3Channel", "4Channel", "5Channel", "6Channel"}; //возможные номера труб
byte counter;

void setup() {
  Serial.begin(9600);         // открываем порт для связи с ПК
  pinMode(btn, INPUT);
  pinMode(digitalHoll, INPUT);

  radioInit();
}

unsigned long timing;

void loop() {
  uint32_t sec = millis() / 1000ul;
  int timeSecs = (sec % 3600ul) % 60ul;

  if (digitalRead(btn) || digitalRead(digitalHoll)) {
    Serial.println("HALL ALARM!!!!");
    Serial.println("Button is active!");
    radio.write(&alarm, sizeof(alarm));
  }
  
  // Перезагрузка платы каждые 5 минут (восстановление связи при потере)
  if (timeSecs == 300) {
    Serial.println("reset");
    asm("JMP 0");
  }

  // Отправка сигнала проверки соединения каждые 3 сек.
  if (millis() - timing > 3000){
    timing = millis(); 
    Serial.println("check connect...");
    for (int x = 0; x < 5 ; x++) {
      radio.write(&checkConnect, sizeof(checkConnect));
      delay(1);
    }
  }

}


void radioInit() {
  radio.begin();              // активировать модуль

  if (radio.isPVariant()) {
    Serial.println("Модуль передатчика nRF24L01 подключен");
    // radio.printDetails();
  } else {
    Serial.println("Неизвестный модуль передатчика");
  }

  radio.setAutoAck(0);        // режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);    // (время между попыткой достучаться, число попыток)
  //radio.enableAckPayload();   // разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(16);   // размер пакета, в байтах

  radio.openWritingPipe(address[0]);  // мы - труба 0, открываем канал для передачи данных
  // radio.setChannel(0x60);             // выбираем канал (в котором нет шумов!)
  radio.setChannel(0x60);

  radio.setPALevel (RF24_PA_MAX);   // уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate (RF24_250KBPS); // скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
  //должна быть одинакова на приёмнике и передатчике!
  //при самой низкой скорости имеем самую высокую чувствительность и дальность!!

  radio.powerUp();        // начать работу
  radio.stopListening();  // не слушаем радиоэфир, мы передатчик
}
