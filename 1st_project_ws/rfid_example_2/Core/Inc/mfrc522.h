#ifndef __MFRC522_H
#define __MFRC522_H

#include "stm32f4xx_hal.h"

//=====================[ 명령어 정의 ]=====================
#define PCD_IDLE              0x00
#define PCD_AUTHENT           0x0E
#define PCD_RECEIVE           0x08
#define PCD_TRANSMIT          0x04
#define PCD_TRANSCEIVE        0x0C
#define PCD_RESETPHASE        0x0F

#define PICC_REQIDL           0x26
#define PICC_REQALL           0x52
#define PICC_ANTICOLL         0x93
#define PICC_SELECTTAG        0x93
#define PICC_AUTHENT1A        0x60
#define PICC_AUTHENT1B        0x61
#define PICC_READ             0x30
#define PICC_WRITE            0xA0
#define PICC_DECREMENT        0xC0
#define PICC_INCREMENT        0xC1
#define PICC_RESTORE          0xC2
#define PICC_TRANSFER         0xB0
#define PICC_HALT             0x50

//=====================[ 상태 코드 ]=====================
#define MI_OK                 0
#define MI_NOTAGERR           1
#define MI_ERR                2

//=====================[ 레지스터 주소 ]=====================
#define CommandReg            0x01
#define CommIEnReg            0x02
#define DivIEnReg             0x03
#define CommIrqReg            0x04
#define DivIrqReg             0x05
#define ErrorReg              0x06
#define Status1Reg            0x07
#define Status2Reg            0x08
#define FIFODataReg           0x09
#define FIFOLevelReg          0x0A
#define ControlReg            0x0C
#define BitFramingReg         0x0D
#define CollReg               0x0E

#define ModeReg               0x11
#define TxModeReg             0x12
#define RxModeReg             0x13
#define TxControlReg          0x14
#define TxASKReg              0x15
#define TModeReg              0x2A
#define TPrescalerReg         0x2B
#define TReloadRegL           0x2D
#define TReloadRegH           0x2C

//=====================[ 함수 선언 ]=====================
void MFRC522_Init(void);
void MFRC522_Reset(void);
void MFRC522_WriteRegister(uint8_t addr, uint8_t val);
uint8_t MFRC522_ReadRegister(uint8_t addr);
void MFRC522_SetBitMask(uint8_t reg, uint8_t mask);
void MFRC522_ClearBitMask(uint8_t reg, uint8_t mask);
void MFRC522_AntennaOn(void);

char MFRC522_Request(uint8_t reqMode, uint8_t *TagType);
char MFRC522_ToCard(uint8_t command, uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint16_t *backLen);
char MFRC522_Anticoll(uint8_t *serNum);
char MFRC522_Check(uint8_t *id);

#endif
