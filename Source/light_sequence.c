#include "stm32f0xx.h"
#include "definitions.h"
#include "utils.h"
#include <string.h>

//#define QUESTION	// comment for ANSWER

#define BUT_QUESTION	GPIO_Pin_0
#define BUT_ANSWER		GPIO_Pin_0
#define BUT_LEFT			GPIO_Pin_1
#define BUT_RIGHT			GPIO_Pin_2
#define LGT_LEFT			GPIO_Pin_3
#define LGT_RIGHT			GPIO_Pin_4
#define LGT_MIRROR		GPIO_Pin_5


UINT8 const sequence[7] = {0x08, 0x10, 0x08, 0x08, 0x10, 0x10, 0x08};
UINT8 input_buffer[256] = { 0x00 };
UINT8 offset = 0;
UINT8 run = 1;


static void create_sequence(const UINT8* array, char len)
{
    UINT8 i;
    for (i = 0; i < len; i++)
    {
        GPIOB->ODR = array[i];
        delay_long(2);
        GPIOB->ODR = 0x00;
        delay_long(2);
    }
}

#ifndef QUESTION
static void check_sequence(void)
{
    create_sequence(input_buffer, offset);
    if ((offset == sizeof(sequence)) && !(memcmp(input_buffer, sequence, sizeof(sequence))))
    {
        GPIOB->ODR = LGT_MIRROR;  // high nibble is output. Connected to light behind mirror
        run = 0;
    }
    offset = 0x00;
}

static void insert(UINT8 item)
{
		UINT8 i;
    for (i = offset; i > 0; i--)
        input_buffer[i] = input_buffer[i-1];
    
    input_buffer[0] = item;
    offset++;
		delay_long(2);
}
#endif
int main (void)
{
	RCC->AHBENR |= 0x060000;
	GPIOB->PUPDR = SetPullUp(BUT_ANSWER | BUT_LEFT | BUT_RIGHT);	//0x15;
	GPIOB->MODER = SetOutput(LGT_LEFT | LGT_RIGHT | LGT_MIRROR);	//0x0540;
	GPIOB->ODR = 0x00;
	
	for(;;)
	{
		while (run)
		{
#ifdef QUESTION
			if (!CHK_BIT(GPIOB->IDR, BUT_QUESTION))	// question
				create_sequence(sequence, sizeof(sequence));
#else
			if (!CHK_BIT(GPIOB->IDR, BUT_ANSWER))		// answer
				check_sequence();
			
			if (!CHK_BIT(GPIOB->IDR, BUT_LEFT))			// left button
				insert(LGT_LEFT);
			
			if (!CHK_BIT(GPIOB->IDR, BUT_RIGHT))		// right button
				insert(LGT_RIGHT);
#endif
		}
	}
}
