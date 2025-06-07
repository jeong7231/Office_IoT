#include "mfrc522.h"
#include "main.h"
#include "string.h"
#include "stm32f4xx_hal.h"

extern SPI_HandleTypeDef hspi1;

void MFRC522_WriteRegister(uint8_t addr, uint8_t val) {
	uint8_t data[2];
	data[0] = (addr << 1) & 0x7E;
	data[1] = val;
	HAL_GPIO_WritePin(RC522_RST_GPIO_Port, RC522_RST_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RC522_RST_GPIO_Port, RC522_RST_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); // NSS = LOW
	HAL_SPI_Transmit(&hspi1, data, 2, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);   // NSS = HIGH
}

uint8_t MFRC522_ReadRegister(uint8_t addr) {
	uint8_t tx = ((addr << 1) & 0x7E) | 0x80;
	uint8_t rx;
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, &tx, 1, HAL_MAX_DELAY);
	HAL_SPI_Receive(&hspi1, &rx, 1, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
	return rx;
}

void MFRC522_SetBitMask(uint8_t reg, uint8_t mask) {
	uint8_t tmp = MFRC522_ReadRegister(reg);
	MFRC522_WriteRegister(reg, tmp | mask);
}

void MFRC522_ClearBitMask(uint8_t reg, uint8_t mask) {
	uint8_t tmp = MFRC522_ReadRegister(reg);
	MFRC522_WriteRegister(reg, tmp & (~mask));
}

void MFRC522_AntennaOn(void) {
	uint8_t temp = MFRC522_ReadRegister(TxControlReg);
	if (!(temp & 0x03)) {
		MFRC522_SetBitMask(TxControlReg, 0x03);
	}
}

void MFRC522_Reset(void) {
	MFRC522_WriteRegister(CommandReg, PCD_RESETPHASE);
}

void MFRC522_Init(void)
{
    // ① RST 핀으로 리셋 신호 제공
    HAL_GPIO_WritePin(RC522_RST_GPIO_Port, RC522_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(RC522_RST_GPIO_Port, RC522_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(50);

    // ② 소프트웨어 초기화
    MFRC522_Reset();

    MFRC522_WriteRegister(TModeReg, 0x8D);
    MFRC522_WriteRegister(TPrescalerReg, 0x3E);
    MFRC522_WriteRegister(TReloadRegL, 30);
    MFRC522_WriteRegister(TReloadRegH, 0);
    MFRC522_WriteRegister(TxASKReg, 0x40);
    MFRC522_WriteRegister(ModeReg, 0x3D);

    // ③ 안테나 켜기
    MFRC522_AntennaOn();
}


char MFRC522_Request(uint8_t reqMode, uint8_t *TagType) {
	char status;
	uint16_t backBits;

	MFRC522_WriteRegister(BitFramingReg, 0x07);

	TagType[0] = reqMode;
	status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);

	if ((status != MI_OK) || (backBits != 0x10))
		status = MI_ERR;

	return status;
}

char MFRC522_ToCard(uint8_t command, uint8_t *sendData, uint8_t sendLen,
					uint8_t *backData, uint16_t *backLen) {
	char status = MI_ERR;
	uint8_t irqEn = 0x00;
	uint8_t waitIRq = 0x00;
	uint8_t lastBits;
	uint8_t n;
	uint16_t i;

	switch (command) {
	case PCD_AUTHENT:
		irqEn = 0x12;
		waitIRq = 0x10;
		break;
	case PCD_TRANSCEIVE:
		irqEn = 0x77;
		waitIRq = 0x30;
		break;
	}

	MFRC522_WriteRegister(CommIEnReg, irqEn | 0x80);
	MFRC522_ClearBitMask(CommIrqReg, 0x80);
	MFRC522_SetBitMask(FIFOLevelReg, 0x80);

	MFRC522_WriteRegister(CommandReg, PCD_IDLE);

	for (i = 0; i < sendLen; i++) {
		MFRC522_WriteRegister(FIFODataReg, sendData[i]);
	}
	MFRC522_WriteRegister(CommandReg, command);
	if (command == PCD_TRANSCEIVE) {
		MFRC522_SetBitMask(BitFramingReg, 0x80);
	}

	i = 2000;
	do {
		n = MFRC522_ReadRegister(CommIrqReg);
		i--;
	} while ((i != 0) && !(n & 0x01) && !(n & waitIRq));

	MFRC522_ClearBitMask(BitFramingReg, 0x80);

	if (i != 0) {
		if (!(MFRC522_ReadRegister(ErrorReg) & 0x1B)) {
			status = MI_OK;
			if (n & irqEn & 0x01)
				status = MI_NOTAGERR;
			if (command == PCD_TRANSCEIVE) {
				n = MFRC522_ReadRegister(FIFOLevelReg);
				lastBits = MFRC522_ReadRegister(ControlReg) & 0x07;
				if (lastBits)
					*backLen = (n - 1) * 8 + lastBits;
				else
					*backLen = n * 8;
				for (i = 0; i < n; i++) {
					backData[i] = MFRC522_ReadRegister(FIFODataReg);
				}
			}
		}
	}
	return status;
}

char MFRC522_Anticoll(uint8_t *serNum) {
	char status;
	uint8_t i;
	uint8_t serNumCheck = 0;
	uint16_t unLen;

	MFRC522_WriteRegister(BitFramingReg, 0x00);
	serNum[0] = PICC_ANTICOLL;
	serNum[1] = 0x20;
	status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

	if (status == MI_OK) {
		for (i = 0; i < 4; i++) {
			serNumCheck ^= serNum[i];
		}
		if (serNumCheck != serNum[4])
			status = MI_ERR;
	}
	return status;
}

char MFRC522_Check(uint8_t *id)
{
    char status;
    uint8_t type[2];

    // ① 카드가 감지되는지 먼저 확인 (Request)
    status = MFRC522_Request(PICC_REQIDL, type);
    if (status != MI_OK)
        return MI_ERR; // 카드 없음

    // ② UID 읽기 (Anti-collision)
    status = MFRC522_Anticoll(id);
    if (status != MI_OK)
        return MI_ERR; // UID 읽기 실패

    return MI_OK; // 성공
}

