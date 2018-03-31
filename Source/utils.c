#include "stm32f0xx.h"
#include "definitions.h"
#include "utils.h"

void delay(void)
{
	UINT32 delay_counter;
	for(delay_counter = 0; delay_counter < 300000; delay_counter++);
}

void delay_nano(void)
{
	UINT16 delay_counter;
	for(delay_counter = 0; delay_counter < 100; delay_counter++);
}

void delay_micro(void)
{
	UINT16 delay_counter;
	for(delay_counter = 0; delay_counter < 1000; delay_counter++);
}

void delay_short(void)
{
	UINT32 delay_counter;
	for(delay_counter = 0; delay_counter < 50000; delay_counter++);
}

void delay_long(UINT8 len)
{
	UINT32 delay_counter;
	for(delay_counter = 0; delay_counter < len; delay_counter++)
		delay();
}
/*
UINT8 GPIO_ReadInputDataBit(GPIO_TypeDef* GPIOx, UINT16 GPIO_Pin)
{
  UINT8 bitstatus = 0;

  if ((GPIOx->IDR & GPIO_Pin) != 0)
    bitstatus = 1;
  else
    bitstatus = 0;
	
  return bitstatus;
}*/

UINT8 GPIO_ReadInputDataBit(GPIO_TypeDef* GPIOx, UINT16 GPIO_Pin)
{
  UINT8 bitstatus = 0;

  if ((GPIOx->IDR & GPIO_Pin) == 0)
    bitstatus = 1;
  else
    bitstatus = 0;
	
  return bitstatus;
}

UINT32 SetPullDown(UINT16 GPIO_Pins)
{
	UINT8 i;
	UINT32 out = 0;
	UINT16 temp;
	
	for(i=0; i<16; i++)
	{
		temp = GPIO_Pins & (0x0001 << i);
		if (temp)
			out |= ((temp * temp) << 1);
	}
	return out;
}

UINT32 SetPullUp(UINT16 GPIO_Pins)
{
	UINT8 i;
	UINT32 out = 0;
	UINT16 temp;
	
	for(i=0; i<16; i++)
	{
		temp = GPIO_Pins & (0x0001 << i);
		if (temp)
			out |= (temp * temp);
	}
	return out;
}

UINT32 SetOutput(UINT16 GPIO_Pins)
{
	UINT8 i;
	UINT32 out = 0;
	UINT16 temp;
	
	for(i=0; i<16; i++)
	{
		temp = GPIO_Pins & (0x0001 << i);
		if (temp)
			out |= (temp * temp);
	}
	return out;
}

UINT8 BitmapConverter(UINT16 bitmap)
{
	UINT8 offset = 0;
	while (1)
	{
		if (bitmap == 1)
				return offset;
		bitmap = bitmap >> 1;
		offset++;
	}
}
