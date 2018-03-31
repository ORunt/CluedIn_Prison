#include "stm32f0xx.h"
#include "definitions.h"
#include "utils.h"
#include <string.h> 

#define ROWS	4
#define COLS	5

#define BUTTON_MASK		0x1F

#define ALL_ON				BUTTON_MASK
#define ALL_OFF				0

#define BTN_DELAY			80
#define FLSH_DELAY		200

#define ROW_0					(0x0001 << (COLS + 1))

UINT8 run = 1;
UINT8 light_pattern[ROWS] = {0x0A, 0x00, 0x0A, 0x04};
UINT8 light_ans[ROWS] = {0x01, 0x01, 0x01, 0x01};

static void initgpio(void)
{
	RCC->AHBENR |= 0x060000;	// Enable clock on PA and PB
	
	// Touch Buttons
	GPIOA->MODER |= 0x5400;	// First 5 are input, next 3 are output (PA7 is for winning? maybe? lets see)
	GPIOA->PUPDR = 0x0000;	// All are floating
	GPIOA->ODR = 0x0000;
	
	// Lights
	GPIOB->MODER = 0x55555555;
	GPIOB->ODR = 0x0000;
}

static void DispPattern(void)
{
	UINT8 row_num;
	
	for (row_num = 0; row_num < ROWS; row_num++)
	{
		GPIOB->ODR = ROW_0 << row_num;	
		GPIOB->ODR |= light_pattern[row_num];
		delay_micro();
	}
}

static void DispPatternDelay(UINT16 len)
{
	UINT16 i;
	for (i = 0; i < len; i++)
		DispPattern();
}

static void ToggleLights(UINT8 on)
{
	UINT8 row;
	for (row = 0; row < ROWS; row++)
		light_pattern[row] = on;
}

static void CheckForWinningCombo(void)
{
	if (!memcmp(light_pattern, light_ans, ROWS))
	{
		UINT8 i;
		
		run = 0;
		DispPatternDelay(FLSH_DELAY);
		
		for(i = 0; i<2; i++)
		{
			ToggleLights(ALL_OFF);
			DispPatternDelay(FLSH_DELAY);
			ToggleLights(ALL_ON);
			DispPatternDelay(FLSH_DELAY);
			//GPIOC->ODR = 0x01;	//TODO: What port are we gonna use here?
		}
	}
}

static void CheckForButtonPress(void)
{
	UINT8 row_num;	// multiplexer number
	UINT32 btn_pshed;
	
	for (row_num = 0; row_num < ROWS; row_num++)
	{
		GPIOA->ODR = row_num << COLS;
		//delay_micro();
		btn_pshed = GPIOA->IDR & BUTTON_MASK;
		
		if(btn_pshed)
		{
			light_pattern[row_num] ^= btn_pshed;
			DispPatternDelay(BTN_DELAY);
			CheckForWinningCombo();
		}
	}
}

int main (void)
{
	initgpio();
	
	for(;;)
	{
		if (run)
		{
			DispPattern();
			CheckForButtonPress();
		}
	}
}
