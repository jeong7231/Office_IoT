/*
 WiFiEsp test: ClientTest
http://www.kccistc.net/
작성일 : 2019.12.17 
작성자 : IoT 임베디드 KSH
*/
#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 10, 2);

#define button_pin_2 10 // [CEN_ARD] > [KSH_LAB]로 DOOR@UNLOCK 명령어를 보내 문을 모두 열리게 할 버튼
#define button_pin_1 9 // 부저 끌 버튼
#define buzzer_pin 8 // 명령어 인식 시 울릴 부저         
SoftwareSerial BTSerial(6, 7); // RX ==>BT:TXD, TX ==> BT:RXD
#include "SoftwareSerial.h"

// 데이터 및 버퍼 설정
#define CMD_SIZE 50          // 명령어 최대 크기
#define ARR_CNT 5            // 명령어 구분용 토큰 최대 개수

#define DEBUG
char lcdLine1[17] = "";
char lcdLine2[17] = "";
char sendBuf[CMD_SIZE];
char recvId[10] = "KSH_CEN";  // 데이터를 보낼 때 ID 



void setup() {
  #ifdef DEBUG
  Serial.begin(115200);  // PC 시리얼 통신 초기화
#endif
  lcd.init();            // LCD 초기화
  lcd.backlight();       // LCD 백라이트 켜기
  lcdDisplay(0, 0, lcdLine1); // 1번째 줄 표시
  lcdDisplay(0, 1, lcdLine2); // 2번째 줄 비우기
    
    pinMode(button_pin_1, INPUT); // 버튼 1(부저 OFF)
    pinMode(button_pin_2, INPUT); // 버튼 2(LAB DOOR UNLOCK)
    pinMode(buzzer_pin, OUTPUT); // LAB에서 침입 명령어 받을 시 울리는 부저

    lcd.init();                      // LCD 초기화
    lcd.backlight();                 // 백라이트 조명 활성화
    BTSerial.begin(9600); // 블루투스 시리얼 통신 시작
    
}

void loop() {
  if (BTSerial.available())
    bluetoothEvent();
}

// 서버에서 수신한 데이터 처리
void bluetoothEvent()
{
  int i = 0;
  char * pToken;
  char * pArray[ARR_CNT] = {0};
  char recvBuf[CMD_SIZE] = {0};
  int len = BTSerial.readBytesUntil('\n', recvBuf, sizeof(recvBuf) - 1);

#ifdef DEBUG
  Serial.print("Recv : ");
  Serial.println(recvBuf);
#endif

  pToken = strtok(recvBuf, "[@]");
  while (pToken != NULL)
  {
    pArray[i] =  pToken;
    if (++i >= ARR_CNT)
      break;
    pToken = strtok(NULL, "[@]");
  }
  //recvBuf : [XXX_BTM]LED@ON
  //pArray[0] = "XXX_LIN"   : 송신자 ID
  //pArray[1] = "LED"
  //pArray[2] = "ON"
  //pArray[3] = 0x0

  // [KSH_SQL] > [KSH_CEN] > [CEN_ARD]으로 부터 받은 [KSH_SQL]NAME@ROOM@UNLOCK or LOCK 데이터를 받아 LCD 출력
  if(!strcmp(pArray[1], "101"))
  {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(pArray[1]); // ROOM1 번호
  lcd.setCursor(5,0);
  lcd.print(pArray[2]); // 이름
  lcd.setCursor(10,0);
  lcd.print(pArray[3]); // ROOM1 LOCK or UNLOCK
  } else if(!strcmp(pArray[1], "102"))
  {
  lcd.setCursor(0,1);
  lcd.print(pArray[1]); // ROOM2 번호
  lcd.setCursor(5,1);
  lcd.print(pArray[2]); // 이름2
  lcd.setCursor(10,1);
  lcd.print(pArray[3]); // ROOM2 LOCK or UNLOCK
  }

  /*if (!strcmp(pArray[2], "BUZZER")) { // 부저 제어
    if (!strcmp(pArray[3], "ON")) {
      digitalWrite(buzzer_pin, HIGH);
      lcd.setCursor(0, 0);
      lcd.print("DETECTED!!      ");  // 명령어를 받고 부저가 울릴 시 LCD 화면 내 "DETECTED!!" 표시(1열)
      lcd.setCursor(0, 1);
      lcd.print("DETECTED!!      ");  // 명령어를 받고 부저가 울릴 시 LCD 화면 내 "DETECTED!!" 표시(1열)
      }
    sprintf(sendBuf, "[%s]%s@%s@%s\n", pArray[0], pArray[1], pArray[2], pArray[3]);
    // ex) [CEN_ARD(0)]NAME(1)@BUZZER(2)@ON(3) 명령어 인식 후 서버로 전달
    BTSerial.write(sendBuf);                // 블루투스 송신
  }*/

  if ((strlen(pArray[1]) + strlen(pArray[2])) < 17)
  {
    sprintf(lcdLine2, "%s %s", pArray[1], pArray[2]);
    lcdDisplay(0, 1, lcdLine2);
  }
   if (!strcmp(pArray[0], "CEN_ARD")) {
    return ;
   }
  else if (!strncmp(pArray[1], " New", 4)) // New Connected
  {
    return ;
  }
  else if (!strncmp(pArray[1], " Alr", 4)) //Already logged
  {
    return ;
  }
  // [KSH_LAB] > [KSH_CEN] > [CEN_ARD]으로 부터 받은 [KSH_LAB]NAME@BUZZER@ON 명령어로 buzzer 제어
  else if (!strcmp(pArray[2], "BUZZER")) { // 부저 제어
    if (!strcmp(pArray[3], "ON")) {
      digitalWrite(buzzer_pin, HIGH);
      lcd.setCursor(0, 0);
      lcd.print("DETECTED!!      ");  // 명령어를 받고 부저가 울릴 시 LCD 화면 내 "DETECTED!!" 표시(1열)
      lcd.setCursor(0, 1);
      lcd.print("DETECTED!!      ");  // 명령어를 받고 부저가 울릴 시 LCD 화면 내 "DETECTED!!" 표시(1열)
      }
    sprintf(sendBuf, "[%s]%s@%s@%s\n", pArray[0], pArray[1], pArray[2], pArray[3]);
    // ex) [CEN_ARD(0)]NAME(1)@BUZZER(2)@ON(3) 명령어 인식 후 서버로 전달
    BTSerial.write(sendBuf);                // 블루투스 송신
  }
}

  int button_state = 0;
  button_state = digitalRead(button_pin_1); // 부저 OFF를 작동할 button

  if(button_state == 1)
  {
    digitalWrite(buzzer_pin, LOW);
    lcd.print("EXIT             ");  // 버튼으로 부저가 종료 시 LCD 화면 내 "EXIT" 표시(1열) 
    lcd.setCursor(0, 0);
    printf("buzzer off");
    sprintf(sendBuf, "[%s]BUZZER@%s\n", "revID", "OFF");
    BTSerial.write(sendBuf);                // 블루투스 송신
  }

#ifdef DEBUG
  // PC 시리얼 입력값 블루투스로 송신
  if (Serial.available())
    BTSerial.write(Serial.read());
#endif

#ifdef DEBUG
  Serial.print("Send : ");
  Serial.print(sendBuf);
#endif
  BTSerial.write(sendBuf);
}

void lcdDisplay(int x, int y, char * str) // LCD 구현
{
  int len = 17 - strlen(str);
  lcd.setCursor(x, y);
  lcd.print(str);
  for (int i = len; i > 0; i--)
    lcd.write(' ');
}
