//#include "stm32f030x6.h"
#include "stm32f0xx.h"
#include "definitions.h"

#define CHK_BIT(port, bit)	((port & bit) == bit)
#define BIT_SET(port, bit)	port |= (UINT32)bit
#define BIT_CLR(port, bit)	port &= ~((UINT32)bit)

void delay(void);
void delay_nano(void);
void delay_micro(void);
void delay_short(void);
void delay_long(UINT8 len);
UINT8 GPIO_ReadInputDataBit(GPIO_TypeDef* GPIOx, UINT16 GPIO_Pin);

UINT32 SetOutput(UINT16 GPIO_Pins);
UINT32 SetPullUp(UINT16 GPIO_Pins);
UINT32 SetPullDown(UINT16 GPIO_Pins);
UINT8 BitmapConverter(UINT16 bitmap);
