#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

//Определение пинов сегментов, индикаторов и кнопки
#define SEGMENT_A D0
#define SEGMENT_B D1
#define SEGMENT_C D2
#define SEGMENT_D D3

#define BOARD_A D4
#define BOARD_B D5 
#define BOARD_C D6 
#define BOARD_D D7 

#define INDICATOR D8

#define BUTTON A0

//Определение данных о WiFi
#define SSID "3.14zda"
#define PASSWORD "komnata_s_efimom"

//Порт UDP сервера
#define PORT 9000

WiFiUDP Udp;

uint8_t packetBuffer[9];
uint8_t d[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

//Функция обработки приходящих данных от бэкенда
void handle() {
  for (uint8_t i = 0; i < 9; i++)
  {
    d[i] = packetBuffer[i];
  }
}

//Отправка данных о нажатой кнопке
void sendButton(uint8_t buttonId) {
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  Udp.write(buttonId);
  Udp.endPacket();
}

//Рендер динамической индикации
void render() {
  //В цикле проходимся по массиву значений ячеек
  for (uint8_t valIndex = 0; valIndex < 9; valIndex++)
  {
    //Если значение ячейки больше 0, то отрисовываем данные
    if (d[valIndex] > 0) {
      //Применяем побитовый сдвиг и побитовое "и" для получения значений пинов (Перевод числа из десятичной в двоичную СС)
      digitalWrite(BOARD_A, valIndex & 0x0001);
      digitalWrite(BOARD_B, (valIndex >> 1) & 0x0001);
      digitalWrite(BOARD_C, (valIndex >> 2) & 0x0001);
      digitalWrite(BOARD_D, (valIndex >> 3) & 0x0001);
      digitalWrite(SEGMENT_A, d[valIndex] & 0x0001);
      digitalWrite(SEGMENT_B, (d[valIndex] >> 1) & 0x0001);
      digitalWrite(SEGMENT_C, (d[valIndex] >> 2) & 0x0001);
      digitalWrite(SEGMENT_D, (d[valIndex] >> 3) & 0x0001);
      digitalWrite(INDICATOR, HIGH);
    } else {
      //Если данных нет, отправляем все пины в логическую единицу для отрисовки пустого индикатора
      digitalWrite(BOARD_A, HIGH);
      digitalWrite(BOARD_B, HIGH);
      digitalWrite(BOARD_C, HIGH);
      digitalWrite(BOARD_D, HIGH);
      digitalWrite(SEGMENT_A, HIGH);
      digitalWrite(SEGMENT_B, HIGH);
      digitalWrite(SEGMENT_C, HIGH);
      digitalWrite(SEGMENT_D, HIGH);
      digitalWrite(INDICATOR, LOW);
    }
  
    //Ожидаем 1000 микросекунд перед следующим индексом ячейки (для уменьшения потери яркости)
    delayMicroseconds(1000);
  }
}

void setup() {
  //Устанавливаем все требуемые пины в режим выхода
  pinMode(SEGMENT_A, OUTPUT);
  pinMode(SEGMENT_B, OUTPUT);
  pinMode(SEGMENT_C, OUTPUT);
  pinMode(SEGMENT_D, OUTPUT);

  pinMode(BOARD_A, OUTPUT);
  pinMode(BOARD_B, OUTPUT);
  pinMode(BOARD_C, OUTPUT);
  pinMode(BOARD_D, OUTPUT);

  pinMode(INDICATOR, OUTPUT);

  //Пин кнопки устанавливаем в режим INPUT_PULLUP (подтягивающий резистор внтури МК)
  pinMode(BUTTON, INPUT_PULLUP);

  Serial.begin(115200);

  //Устанавливаем WiFi соединение
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf("[SETUP] WIFI WAIT \n");
    delay(300);
  }

  //Устанавливаем MDNS сервер для доступа без IP адреса
  if (MDNS.begin("diplom")) {
    Serial.println("mDNS responder started");
  }
  
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  //Создаем UDP сервер
  Udp.begin(PORT);
}

bool hasValue = false;

void loop() {
  //Обновляем MDNS слушатель
  MDNS.update();
  
  //Обновляем UDP сервер
  int packetSize = Udp.parsePacket();
  //Если есть данные от UDP сервера записываем их в буфер и вызываем функцию обработки
  if (packetSize) {
    Udp.read(packetBuffer, 9);
    handle();
  }

  //Вызываем функцию рендера матрицы
  render();
  
  //Считываем аналоговые данные с пина кнопки
  int val = analogRead(BUTTON);
  if (hasValue) {
    if (val == 1024) {
    hasValue = false;
    }
  } else {
    if (val < 1024) {
      hasValue = true;
    }
    if (hasValue) {
      if (val > 420 && val < 470) { //2
        d[1] = 0;
        sendButton(1);
      } else if (val >= 470 && val < 520) { //3
        d[2] = 0;
        sendButton(2);
      } else if (val >= 520 && val < 560) { //4
        d[3] = 0;
        sendButton(3);
      } else if (val >= 560 && val < 590) { //5
        d[4] = 0;
        sendButton(4);
      } else if (val >= 590 && val < 640) { //6
        d[5] = 0;
        sendButton(5);
      } else if (val >= 640 && val < 650) { //7
        d[6] = 0;
        sendButton(6);
      } else if (val >= 650 && val < 700) { //8
        d[7] = 0;
        sendButton(7);
      } else if (val >= 700 && val < 750) { //9
        d[8] = 0;
        sendButton(8);
      }
    }
  }
}