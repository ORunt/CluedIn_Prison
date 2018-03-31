#include "stm32f0xx.h"
#include "definitions.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

#define SPEAKER		0x01	// GPIOB_0
#define RED_LED		0x02	// GPIOB_1
#define GREEN_LED	0x04	// GPIOB_2
#define GROUND		0x08	// GPIOB_3
#define BLUE_LED	0x10	// GPIOB_4
#define SOLENOID 	0x20	// GPIOB_5


#define ROW_1			GPIO_Pin_5
#define ROW_2			GPIO_Pin_0
#define ROW_3			GPIO_Pin_1
#define ROW_4			GPIO_Pin_3
#define COL_1			GPIO_Pin_4
#define COL_2			GPIO_Pin_6
#define COL_3			GPIO_Pin_2


/*
#define ROW_1			GPIO_Pin_0
#define ROW_2			GPIO_Pin_1
#define ROW_3			GPIO_Pin_2
#define ROW_4			GPIO_Pin_3
#define COL_1			GPIO_Pin_5
#define COL_2			GPIO_Pin_6
#define COL_3			GPIO_Pin_7
*/
const UINT8 secret_code[4] = {3, 9, 6, 1};
//const UINT8 secret_code[4] = {7,7,7,7};
UINT8 code_buffer[4] = {0};
UINT8 retry_counter = 0;
UINT16 offset = 0;

static void initgpio(void)
{
	RCC->AHBENR |= 0x060000;	// This must be changed later on the actual puzzle
	
	GPIOA->MODER |= (COL_1 * COL_1) | (COL_2 * COL_2) | (COL_3 * COL_3);	// Columns are outputs
	GPIOA->PUPDR |= (ROW_1 * ROW_1) | (ROW_2 * ROW_2) | (ROW_3 * ROW_3) | (ROW_4 * ROW_4); // Rows (inputs) are pull up
	
	//GPIOA->PUPDR |= ((COL_1 * COL_1) << 1) | ((COL_2 * COL_2) << 1) | ((COL_3 * COL_3) << 1);		// Rows (inputs) are pull down
	//GPIOA->MODER |= (ROW_1 * ROW_1) | (ROW_2 * ROW_2) | (ROW_3 * ROW_3) | (ROW_4 * ROW_4);	// Columns are outputs
		
	GPIOA->OSPEEDR |= 0xFFFF;   // High speed
	
	GPIOA->ODR |= 0xFFF;
	
	GPIOB->MODER = 0x5555;
	GPIOB->ODR = BLUE_LED;
}

static void display_red(void)
{
	GPIOB->ODR = RED_LED | SPEAKER;
	delay_long(5);
	GPIOB->ODR = 0;
}

static void blink_red(void)
{
	UINT8 i;
	
	for(i = 0; i<10; i++)
	{
		GPIOB->ODR = RED_LED | SPEAKER;
		delay_long(2);
		GPIOB->ODR = 0;
		delay_long(2);
	}
}

static void winner (void)
{
	retry_counter = 0;
	GPIOB->ODR = GREEN_LED | SOLENOID | SPEAKER;
	delay_long(5);
	GPIOB->ODR = BLUE_LED;
	delay();
}

static void loser (void)
{	
	retry_counter ++;

	if(retry_counter >= 3)
	{
		blink_red();
		retry_counter = 0;
	}
	else
		display_red();
	
	GPIOB->ODR = BLUE_LED;
	delay();
}

static void play_tone(void)
{
	GPIOB->ODR = SPEAKER;
	delay_long(1);
	GPIOB->ODR = 0;
}

static void disp (UINT8 num)
{
	play_tone();
	
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
/*
static void the_work_horse(void)
{
		GPIOA->BRR = COL_2;	//set bit as low  COL 2
		GPIOA->BRR = COL_3;	//set bit as low  COL 3
		//delay_nano();
		GPIOA->BSRR = COL_1;	//set bit as high	COL 1

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
		
		GPIOA->BRR = COL_1;	//set bit as low
		GPIOA->BRR = COL_3;	//set bit as low
		//delay_nano();
		GPIOA->BSRR = COL_2;	//set bit as high
		
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
		
		GPIOA->BRR = COL_1;	//set bit as low
		GPIOA->BRR = COL_2;	//set bit as low
		//delay_nano();
		GPIOA->BSRR = COL_3;	//set bit as high
		
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
*/
/*
static void the_work_horse_v2(void)
{
		GPIOA->BSRR = ROW_1;	//set bit as high	COL 1
		GPIOA->BRR = ROW_2;	//set bit as low
		GPIOA->BRR = ROW_3;	//set bit as low
		GPIOA->BRR = ROW_4;	//set bit as low
		
		{
			if(GPIO_ReadInputDataBit(GPIOC, COL_1))
				disp(1);
			if(GPIO_ReadInputDataBit(GPIOC, COL_2))
				disp(2);
			if(GPIO_ReadInputDataBit(GPIOC, COL_3))
				disp(3);
		}
		
		GPIOA->BRR = ROW_1;	//set bit as low
		GPIOA->BSRR = ROW_2;	//set bit as high
		GPIOA->BRR = ROW_3;	//set bit as low
		GPIOA->BRR = ROW_4;	//set bit as low
		
		{
			if(GPIO_ReadInputDataBit(GPIOC, COL_1))
				disp(4);
			if(GPIO_ReadInputDataBit(GPIOC, COL_2))
				disp(5);
			if(GPIO_ReadInputDataBit(GPIOC, COL_3))
				disp(6);
		}
		
		GPIOA->BRR = ROW_1;	//set bit as low
		GPIOA->BRR = ROW_2;	//set bit as low
		GPIOA->BSRR = ROW_3;	//set bit as high
		GPIOA->BRR = ROW_4;	//set bit as low  COL 3
		
		{
			if(GPIO_ReadInputDataBit(GPIOC, COL_1))
				disp(7);
			if(GPIO_ReadInputDataBit(GPIOC, COL_2))
				disp(8);
			if(GPIO_ReadInputDataBit(GPIOC, COL_3))
				disp(9);
		}
		
		GPIOA->BRR = ROW_1;	//set bit as low
		GPIOA->BRR = ROW_2;	//set bit as low
		GPIOA->BRR = ROW_3;	//set bit as high
		GPIOA->BSRR = ROW_4;	//set bit as low  COL 3
		
		{
			if(GPIO_ReadInputDataBit(GPIOC, COL_1))
				disp(254);
			if(GPIO_ReadInputDataBit(GPIOC, COL_2))
				disp(0);
			if(GPIO_ReadInputDataBit(GPIOC, COL_3))
				disp(255);
		}
}*/

static void the_work_horse(void)
{
		GPIOA->BSRR = COL_2;	//set bit as low  COL 2
		GPIOA->BSRR = COL_3;	//set bit as low  COL 3
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
		
		GPIOA->BSRR = COL_1;	//set bit as low
		GPIOA->BSRR = COL_3;	//set bit as low
		GPIOA->BRR = COL_2;	//set bit as high
		
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
		
		GPIOA->BSRR = COL_1;	//set bit as low
		GPIOA->BSRR = COL_2;	//set bit as low
		GPIOA->BRR = COL_3;	//set bit as high
		
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

int main (void)
{
	initgpio();
	
	for(;;)
	{
		the_work_horse();
	}
}
