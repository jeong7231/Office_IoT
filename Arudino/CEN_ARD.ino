
#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <MsTimer2.h>
#include <TimerOne.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define button_pin_2 10
#define button_pin_1 9
#define buzzer_pin 8

SoftwareSerial BTSerial(6, 7);

#define CMD_SIZE 50
#define ARR_CNT 5

char recvBuf[50];
char sendBuf[CMD_SIZE];
char recvId[10] = "PRJ_CEN";

bool button1Prev = HIGH;
bool button2Prev = HIGH;

unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;
const unsigned long debounceDelay = 50;

volatile bool updateFlag_1 = false;
volatile bool updateFlag_2 = false;

unsigned long prevSendTime = 0;
int button1Count = 0;
int button2Count = 0;

const unsigned long displayDuration = 1000;
bool showstop = false;
bool showUnlock = false;
unsigned long stopDisplayStart = 0;
unsigned long unlockDisplayStart = 0;

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.clear();

  pinMode(button_pin_1, INPUT_PULLUP);
  pinMode(button_pin_2, INPUT_PULLUP);
  pinMode(buzzer_pin, OUTPUT);

  BTSerial.begin(9600);
  Serial.println("System Initialized");

  MsTimer2::set(5000, onTimer);
  MsTimer2::start();
  Timer1.initialize(7000000);
  Timer1.attachInterrupt(onTimer2);
}

void loop() {
  unsigned long now = millis();

  // 시간 경과 후 LCD 자동 지우기
  if (showstop && (now - stopDisplayStart >= displayDuration)) {
    showstop = false;
    lcd.clear();
  }
  if (showUnlock && (now - unlockDisplayStart >= displayDuration)) {
    showUnlock = false;
    lcd.clear();
  }

  if (BTSerial.available()) bluetoothEvent();

  bool button1Current = digitalRead(button_pin_1);
  bool button2Current = digitalRead(button_pin_2);

  if (updateFlag_1) {
    updateFlag_1 = false;
    sprintf(sendBuf, "[PRJ_SQL]GETROOM@101\n");
    BTSerial.write(sendBuf);
  }

  if (updateFlag_2) {
    updateFlag_2 = false;
    sprintf(sendBuf, "[PRJ_SQL]GETROOM@102\n");
    BTSerial.write(sendBuf);
  }

  if (button1Prev == HIGH && button1Current == LOW && (now - lastDebounceTime1) > debounceDelay) {
    lastDebounceTime1 = now;
    button1Count++;
    showstop = true;
    stopDisplayStart = now;
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("STOP");
    digitalWrite(buzzer_pin, LOW);
    sprintf(sendBuf, "[%s]BUZZER@OFF\n", recvId);
    BTSerial.write(sendBuf);
  }

  if (button2Prev == HIGH && button2Current == LOW && (now - lastDebounceTime2) > debounceDelay) {
    lastDebounceTime2 = now;
    button2Count++;
    showUnlock = true;
    unlockDisplayStart = now;
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("ALL ROOM UNLOCK");
    sprintf(sendBuf, "[PRJ_STM]UNLOCK\n");
    BTSerial.write(sendBuf);
    Serial.print("Send to SQL : "); Serial.println(sendBuf);
  }

  button1Prev = button1Current;
  button2Prev = button2Current;

  if (Serial.available()) {
    BTSerial.write(Serial.read());
  }

  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    msg.trim();
    if (msg == "DETECTED@101") {
      digitalWrite(buzzer_pin, HIGH);
      lcd.setCursor(0, 0); lcd.print("DETECTED!!      ");
      lcd.setCursor(0, 1); lcd.print("DETECTED!!      ");
    }
  }
}

void bluetoothEvent() {
  char* pToken;
  char* pArray[ARR_CNT] = { 0 };
  char recvBuf[CMD_SIZE] = { 0 };
  int i = 0;

  int len = BTSerial.readBytesUntil('\n', recvBuf, sizeof(recvBuf) - 1);
  recvBuf[len] = '\0';
  Serial.print("Recv : "); Serial.println(recvBuf);

  pToken = strtok(recvBuf, "[@]");
  while (pToken != NULL && i < ARR_CNT) {
    pArray[i++] = pToken;
    pToken = strtok(NULL, "[@]");
  }

  for (int j = 0; j < i; j++) {
    int plen = strlen(pArray[j]);
    while (plen > 0 && (pArray[j][plen - 1] == '\r' || pArray[j][plen - 1] == '\n')) {
      pArray[j][--plen] = '\0';
    }
  }

  if (i >= 5) {
    if (strcmp(pArray[2], "101") == 0) {
      lcd.setCursor(0, 0);
      lcd.print(pArray[2]); lcd.print("-JTY-"); lcd.print(pArray[4]);
      if (!strcmp(pArray[4], "LOCK")) lcd.print("  ");
    } else if (strcmp(pArray[2], "102") == 0) {
      lcd.setCursor(0, 1);
      lcd.print(pArray[2]); lcd.print("-LEE-"); lcd.print(pArray[4]);
      if (!strcmp(pArray[4], "LOCK")) lcd.print("  ");
    }
  }

  if (i >= 3 && !strcmp(pArray[1], "DETECTED") && !strcmp(pArray[2], "101")) {
    digitalWrite(buzzer_pin, HIGH);
    lcd.setCursor(0, 0); lcd.print("ROOM : 101      ");
    lcd.setCursor(0, 1); lcd.print("DETECTED!!      ");
  } else if (i >= 3 && !strcmp(pArray[1], "DETECTED") && !strcmp(pArray[2], "102")) {
    digitalWrite(buzzer_pin, HIGH);
    lcd.setCursor(0, 0); lcd.print("ROOM : 102      ");
    lcd.setCursor(0, 1); lcd.print("DETECTED!!      ");
  }
}

void onTimer(){
  updateFlag_1 = true;
}

void onTimer2() {
  updateFlag_2 = true;
}
