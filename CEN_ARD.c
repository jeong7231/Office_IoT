#include <SoftwareSerial.h>             // 블루투스 모듈과 통신하기 위한 라이브러리
#include <Wire.h>                       // I2C 통신을 위한 기본 라이브러리
#include <LiquidCrystal_I2C.h>          // I2C 방식 LCD 사용을 위한 라이브러리

LiquidCrystal_I2C lcd(0x27, 16, 2);     // LCD 객체 생성, 주소 0x27, 16x2 사이즈

// 핀 번호 정의
#define button_pin_2 10                 // 버튼 2 핀: ALL ROOM UNLOCK 명령 전송용
#define button_pin_1 9                  // 버튼 1 핀: EXIT 표시 및 BUZZER OFF 용
#define buzzer_pin 8                    // 부저 출력 핀

SoftwareSerial BTSerial(6, 7);          // 블루투스 시리얼 통신: D6(RX), D7(TX)

#define CMD_SIZE 50                     // 전송/수신 명령어 최대 크기
#define ARR_CNT 5                       // 명령어 파싱 시 최대 토큰 수

char recvBuf[50];
int idx = 0;
char sendBuf[CMD_SIZE];                 // 전송할 문자열 버퍼
char recvId[10] = "PRJ_CEN";            // 이 장치의 ID (송수신 구분용)

// 상태 변수: LCD 토글 상태 기억용
bool isExitMode = false;                // EXIT 모드 여부
bool isUnlockMode = false;             // UNLOCK 모드 여부

// 버튼 상태 추적 변수 (이전 상태 저장용)
bool button1Prev = HIGH;
bool button2Prev = HIGH;

// 디바운싱을 위한 시간 변수
unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;
const unsigned long debounceDelay = 50; // 50ms 이상 눌림 유지되어야 유효

// 버튼 누른 횟수 카운터
int button1Count = 0;
int button2Count = 0;

void setup() {
  Serial.begin(115200);                // 시리얼 모니터 통신 속도 설정
  lcd.init();                          // LCD 초기화
  lcd.backlight();                     // LCD 백라이트 켜기
  lcd.clear();                         // LCD 초기화 (화면 지우기)

  pinMode(button_pin_1, INPUT_PULLUP); // 버튼 1: 내부 풀업 저항 사용
  pinMode(button_pin_2, INPUT_PULLUP); // 버튼 2: 내부 풀업 저항 사용
  pinMode(buzzer_pin, OUTPUT);         // 부저 핀 출력으로 설정

  BTSerial.begin(9600);                // 블루투스 시리얼 시작 (9600 baud rate)

  Serial.println("System Initialized"); // 초기화 메시지 출력
}

void loop() {
  if (BTSerial.available()) bluetoothEvent(); // 블루투스 수신 데이터 처리

  // 현재 버튼 상태 읽기
  bool button1Current = digitalRead(button_pin_1);
  bool button2Current = digitalRead(button_pin_2);

  // 버튼 1(9번 핀) 눌림 감지 + 디바운싱 + 토글 처리
  if (button1Prev == HIGH && button1Current == LOW && (millis() - lastDebounceTime1) > debounceDelay) {
    lastDebounceTime1 = millis();             // 마지막 눌림 시간 갱신
    isExitMode = !isExitMode;                 // 상태 토글
    button1Count++;                           // 눌린 횟수 증가
    handleExitToggle(isExitMode);             // 동작 처리 함수 호출
  }

  // 버튼 2(10번 핀) 눌림 감지 + 디바운싱 + 토글 처리
  if (button2Prev == HIGH && button2Current == LOW && (millis() - lastDebounceTime2) > debounceDelay) {
    lastDebounceTime2 = millis();
    isUnlockMode = !isUnlockMode;
    button2Count++;
    handleUnlockToggle(isUnlockMode);
  }

  // 버튼 이전 상태 업데이트 (다음 루프에서 비교용)
  button1Prev = button1Current;
  button2Prev = button2Current;

  // PC → 블루투스로 문자 전달 (디버깅용)
  if (Serial.available()) {
    BTSerial.write(Serial.read());
  }
}

// 버튼 1 토글 시 LCD 및 부저 제어
void handleExitToggle(bool state) {
  lcd.clear(); // LCD 초기화

  if (state) {
    digitalWrite(buzzer_pin, LOW);         // 부저 끄기
    lcd.setCursor(0, 0); lcd.print("EXIT");// LCD 표시

    Serial.println("[TOGGLE] EXIT ON");
    Serial.print("EXIT Button Count: "); Serial.println(button1Count);
    Serial.println("LCD State: EXIT");

    sprintf(sendBuf, "[%s]BUZZER@OFF\n", recvId); // 명령 구성
    BTSerial.write(sendBuf);                     // 블루투스로 전송
  } else {
    lcd.setCursor(0, 0); lcd.print("                "); // LCD 지우기
    Serial.println("[TOGGLE] EXIT OFF");
    Serial.print("EXIT Button Count: "); Serial.println(button1Count);
    Serial.println("LCD State: CLEARED");
  }
}

// 버튼 2 토글 시 LCD 및 블루투스 명령 전송
void handleUnlockToggle(bool state) {
  lcd.clear();

  if (state) {
    lcd.setCursor(0, 0); lcd.print("ALL ROOM UNLOCK");

    Serial.println("[TOGGLE] ALL ROOM UNLOCK ON");
    Serial.print("UNLOCK Button Count: "); Serial.println(button2Count);
    Serial.println("LCD State: ALL ROOM UNLOCK");

    sprintf(sendBuf, "[PRJ_LAB]ALL@ALL@UNLOCK\n");
    BTSerial.write(sendBuf);
    Serial.print("Send to SQL : "); Serial.println(sendBuf);
  } else {
    lcd.setCursor(0, 0); lcd.print("                ");
    Serial.println("[TOGGLE] ALL ROOM UNLOCK OFF");
    Serial.print("UNLOCK Button Count: "); Serial.println(button2Count);
    Serial.println("LCD State: CLEARED");
  }
}

// 블루투스로 수신된 데이터 처리 함수
void bluetoothEvent() {
  char* pToken;
  char* pArray[ARR_CNT] = { 0 };       // 명령 파싱 결과 저장 배열
  char recvBuf[CMD_SIZE] = { 0 };      // 수신된 원본 명령
  int i = 0;

  // 개행(\n)까지 수신 데이터를 recvBuf에 저장
  int len = BTSerial.readBytesUntil('\n', recvBuf, sizeof(recvBuf) - 1);

  Serial.print("Recv : "); Serial.println(recvBuf); // 수신 확인 출력

  // 명령어를 "[@]" 기준으로 파싱
  pToken = strtok(recvBuf, "[@]");
  while (pToken != NULL) {
    pArray[i] = pToken;
    if (++i >= ARR_CNT) break;
    pToken = strtok(NULL, "[@]");
  }

  // 메시지 형식: [ID]ROOM@NAME@DOOR
  // 예: [PRJ_LAB]101@홍길동@OPEN
  if (i >= 4 && strlen(pArray[1]) > 0 && strlen(pArray[2]) > 0 && strlen(pArray[3]) > 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Room: ");
    lcd.print(pArray[1]);
    lcd.setCursor(0, 1);
    lcd.print(pArray[2]);
    lcd.print(" ");
    lcd.print(pArray[3]);
  }

  // BUZZER ON 명령 수신 시 처리
  if (!strcmp(pArray[2], "BUZZER") && !strcmp(pArray[3], "ON")) {
    digitalWrite(buzzer_pin, HIGH); // 부저 울림
    lcd.setCursor(0, 0); lcd.print("DETECTED!!      ");
    lcd.setCursor(0, 1); lcd.print("DETECTED!!      ");
  }
}
