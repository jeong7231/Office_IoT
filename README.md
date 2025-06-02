# 1st_project
인텔 아카데미 미니 프로젝트
<250531>
-bluetooth를 모듈을 통해 연결
(아두이노 보드 부분)
-[PRJ_STM]으로 부터 명령어가 들어오면 BUZZER ON, button_1 누르면 BUZZER OFF

-[PRJ_STM]으로 부터 명령어가 들어오면 LCD 내 "DETECTED" 출력

-[PRJ_SQL]에서 받은 ROOM,NAME,UNLOCK 상태를 LCD로 출력(진행중)

<250601>
(아두이노 보드 부분)
-[PRJ_SQL]에서 받은 ROOM,NAME,UNLOCK 상태를 LCD로 출력

-button_2 누를 시 [PRJ_STM]ROOM@NAME@DOOR 명령어 전송 및 LCD 화면 내 출력

(서버 추가)
-[PRJ_SQL]에서 받은 방정보인 [ID]ROOM@NAME@DOOR를 SERVER로 보내 SERVER에서 5초 주기로 [PRJ_CEN] 클라이언트로 전송 후 그 정보를 블루투스 모듈을 통해 아두이노 보드에서 받아 LCD 출력
*버튼 입력 시에는 버튼 정보 LCD 출력 후 바로 방정보로 변경됨 

(클라이언트 변경중/PRJ_CEN)
-아두이노에서 전달 시 SRV, 클라이언트에서 아두이노로 전달 시 ARD / 부저 버튼 누를 시 [PRJ_CEN]클라이언트 내에서 내에서 중복 송출 되는 중

(내일 두번째로 실행할 목록)
- LAB에서 부저 신호 보내면 아두이노에서 부저 ON 실행되는거 확인

<250602>
- 버튼 토글에서 > 단일 클릭 후 mills함수로 초기화

- mills함수로 일정 시간마다 LCD 내 101,102호 상태 구현

- SQL 클라이언트와 정보 받기와 UNLOCK 명령어 LAB으로 보내는 걸로 구현