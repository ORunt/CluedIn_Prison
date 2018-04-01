#include "stm32f0xx.h"
#include "definitions.h"
#include "utils.h"
#include <string.h>

#define BUTTON_MASK  	0x1FF
#define LIGHT_MASK  	0x1FF
#define E_LOCK			 	0x200	// GPIO_9

UINT8 offset = 0;
UINT8 input_buffer[5] = {0};
UINT8 ans_buffer[5] = {0,1,8,7,4};

static void InitGpio(void)
{
	RCC->AHBENR |= 0x060000;	// Enable clock on PA and PB
	
	// Touch Buttons
	GPIOA->MODER |= 0x0000;	// All are input
	GPIOA->PUPDR = 0x0000;	// All are floating
	GPIOA->ODR = 0x0000;
	
	// Lights
	GPIOB->MODER = 0x55555555;
	GPIOB->ODR = 0x0000;
}

static void Winner(void)
{
	GPIOB->ODR = 0x0000;
	delay_long(2);
	GPIOB->ODR = 0x0155; // X
	delay_long(2);
	GPIOB->ODR = 0x00BA; // +
	delay_long(2);
	GPIOB->ODR = 0x0155; // X
	delay_long(2);
	GPIOB->ODR = 0x00BA; // +
	delay_long(2);
	GPIOB->ODR = 0x0155; // X
	delay_long(2);
	GPIOB->ODR = 0x00BA; // +
	delay_long(2);
	GPIOB->ODR = E_LOCK;
	delay_long(14);		// 3 seconds
	GPIOB->ODR = 0x0000;
}

static UINT8 CheckButton(UINT16 button)
{
	UINT8 current_button = BitmapConverter(button);
	UINT8 i;
	
	for (i = 0; i < offset; i++)
	{
		if(input_buffer[i] == current_button)
			return 1;
	}
	return 0;
}

static void CheckPattern(void)
{
	if(!memcmp(input_buffer, ans_buffer, sizeof(input_buffer)))
		Winner();
}

static void Insert(UINT16 item)
{
	GPIOB->ODR |= item;
	input_buffer[offset] = BitmapConverter(item);
	offset++;
	if (offset == sizeof(input_buffer))
	{
		offset = 0;
		delay_long(2);
		CheckPattern();
		GPIOB->ODR = 0;
	}
}

int main(void)
{
	UINT16 button_bitmap;
	
	InitGpio();
	
	for(;;)
	{
		button_bitmap = BUTTON_MASK & GPIOA->IDR;
		if (button_bitmap)
		{
			if(!CheckButton(button_bitmap))
			{
				Insert(button_bitmap);
				delay();
			}
		}
	}
}
