#include "stm32f0xx.h"
#include "definitions.h"
#include "utils.h"

#define TEST
#ifdef TEST
	typedef UINT8 UINTX;
	#define LIGHT_TEMPLATE	0x0E
	#define BUTTON_MASK			0x0F
	#define SEQUENCE				{1, 3, 0, 2}
	#define SEQ_SIZE				4
#else
	typedef UINT16 UINTX;
	#define LIGHT_TEMPLATE	0x3FFE
	#define BUTTON_MASK			0x8FFF
	#define SEQUENCE				{6, 3, 8, 10, 0, 9, 5, 4, 2, 1, 7}
	#define SEQ_SIZE				11
#endif

#define BUTTON  ((UINTX)(GPIOA->IDR))
#define LIGHT   ((UINTX)(GPIOB->ODR))
#define SET			1
#define CLR			0

UINT8 lookup[SEQ_SIZE] = SEQUENCE;
UINTX prev_buttons = 0;
UINT8 run = 1;
UINTX buttons_pushed = 0;

static UINT8 bitmap_converter(UINTX bitmap)
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

static void alter_light (UINTX light_bitmap, UINT8 set)
{
	UINTX looked_up_light = (UINTX)(1 << lookup[bitmap_converter(light_bitmap)]);
	if (set)
		BIT_SET(GPIOB->ODR, looked_up_light);
	else
		BIT_CLR(GPIOB->ODR, looked_up_light);
}

int main (void)
{
	RCC->AHBENR |= 0x060000;	// This must be changed later on the actual puzzle
	GPIOA->PUPDR = 0x55;
	GPIOB->MODER = 0x5555;
	GPIOB->ODR = 0x00;
	for(;;)
	{
		if (run)
		{
			buttons_pushed = ~BUTTON & BUTTON_MASK;
			if (buttons_pushed != prev_buttons)
			{
				UINTX button_changed = buttons_pushed ^ prev_buttons;
				
				if (button_changed & buttons_pushed)									// Button PUSHED?
				{
					if (button_changed & LIGHT)													// Light ON?
					{
						BIT_CLR(GPIOB->ODR, button_changed);
						alter_light(button_changed, SET);
					}
					else																								// Light OFF?
						alter_light(button_changed, SET);
				}
				else																									// Button RELEASED?
					alter_light(button_changed, CLR);
				
				if (LIGHT == LIGHT_TEMPLATE)
				{
					GPIOB->ODR = 0x80;
					run = 0;
				}
				prev_buttons = buttons_pushed;
				delay_short();
			}
		}
	}
}
