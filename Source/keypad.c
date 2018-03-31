#include "stm32f0xx.h"
#include "definitions.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

// Inputs
#define TRIGGER				0x01	// GPIOB_0 - no pull
#define EXT_CTRL			0x40	// GPIOB_6 - no pull

// Outputs
#define ALARM					0x02	// GPIOB_1 - no pull
#define LIGHT_CELL		0x04	// GPIOB_2 - no pull
#define LIGHT_GUARDS	0x08	// GPIOB_3 - no pull
#define LIGHT_CONTROL	0x10	// GPIOB_4 - no pull
#define LIGHT_UV			0x20	// GPIOB_5 - no pull

// Keypad
#define ROW_1			GPIO_Pin_0	// GPIOA_0
#define ROW_2			GPIO_Pin_1	// GPIOA_1
#define ROW_3			GPIO_Pin_2	// GPIOA_2
#define ROW_4			GPIO_Pin_3	// GPIOA_3
#define COL_1			GPIO_Pin_4	// GPIOA_4
#define COL_2			GPIO_Pin_5	// GPIOA_5
#define COL_3			GPIO_Pin_6	// GPIOA_6

const UINT8 secret_code[4] = {1,2,3,4};
UINT8 code_buffer[4] = {0};
UINT8 retry_counter = 0;
UINT16 offset = 0;
UINT8 alarm_on = 0;

static void initgpio(void)
{
	RCC->AHBENR |= 0x060000;	// This must be changed later on the actual puzzle
	
	// Port A
	GPIOA->MODER |= (COL_1 * COL_1) | (COL_2 * COL_2) | (COL_3 * COL_3);	// Columns are outputs
	GPIOA->PUPDR |= (ROW_1 * ROW_1) | (ROW_2 * ROW_2) | (ROW_3 * ROW_3) | (ROW_4 * ROW_4); // Rows (inputs) are pull up
	GPIOA->OSPEEDR |= 0xFFFF;   // High speed
	GPIOA->ODR |= 0xFFF;
	
	// Port B
	GPIOB->MODER = SetOutput(ALARM | LIGHT_CELL | LIGHT_GUARDS | LIGHT_CONTROL | LIGHT_UV);
	GPIOB->PUPDR = 0; // no push or pull
}

static void winner (void)
{
	
}

static void loser (void)
{	
	
}

/*
static void play_tone(void)
{
	GPIOB->ODR = SPEAKER;
	delay_long(1);
	GPIOB->ODR = 0;
}
*/

static void disp (UINT8 num)
{
	//play_tone();
	
	code_buffer[offset++] = num;
	
	if (offset >= sizeof(secret_code))
	{
		offset = 0;
		if (memcmp(secret_code, code_buffer, 4))
			loser();
		else
			winner();
	}
	delay();
}

static void AlarmLoop(void)
{
	while (alarm_on)
	{
		GPIOA->BSRR = COL_3;	//set bit as high  COL 3
		GPIOA->BSRR = COL_2;	//set bit as high  COL 2
		GPIOA->BRR = COL_1;	//set bit as low	COL 1

		{
			if(GPIO_ReadInputDataBit(GPIOA, ROW_1))	// ROW 1
				disp(1);
			if(GPIO_ReadInputDataBit(GPIOA, ROW_2))	// ROW 2
				disp(4);
			if(GPIO_ReadInputDataBit(GPIOA, ROW_3))	// ROW 3
				disp(7);
			if(GPIO_ReadInputDataBit(GPIOA, ROW_4))	// ROW 4
				disp(254);
		}
		
		GPIOA->BSRR = COL_1;	//set bit as high
		GPIOA->BSRR = COL_3;	//set bit as high
		GPIOA->BRR = COL_2;	//set bit as low
		
		{
			if(GPIO_ReadInputDataBit(GPIOA, ROW_1))
				disp(2);
			if(GPIO_ReadInputDataBit(GPIOA, ROW_2))
				disp(5);
			if(GPIO_ReadInputDataBit(GPIOA, ROW_3))
				disp(8);
			if(GPIO_ReadInputDataBit(GPIOA, ROW_4))
				disp(10);
		}
		
		GPIOA->BSRR = COL_2;	//set bit as high
		GPIOA->BSRR = COL_1;	//set bit as high
		GPIOA->BRR = COL_3;	//set bit as low
		
		{
			if(GPIO_ReadInputDataBit(GPIOA, ROW_1))
				disp(3);
			if(GPIO_ReadInputDataBit(GPIOA, ROW_2))
				disp(6);
			if(GPIO_ReadInputDataBit(GPIOA, ROW_3))
				disp(9);
			if(GPIO_ReadInputDataBit(GPIOA, ROW_4))
				disp(255);
		}
	}
}

int main (void)
{
	initgpio();
	
	BIT_SET(GPIOB->ODR, LIGHT_CELL | LIGHT_GUARDS | LIGHT_CONTROL);
	
	for(;;)
	{
		while (!CHK_BIT(GPIOB->IDR, TRIGGER)); // check this later
		BIT_CLR(GPIOB->ODR, LIGHT_CELL | LIGHT_GUARDS | LIGHT_CONTROL);
		BIT_SET(GPIOB->ODR, LIGHT_UV | ALARM);
		
		

		AlarmLoop();
	}
}
