#include "stm32f0xx.h"
#include "definitions.h"
#include "utils.h"

#define PIN_IN_LASER			GPIO_Pin_0
#define PIN_IN_LIGHT			GPIO_Pin_1
#define PIN_IN_TOGGLE			GPIO_Pin_2
#define PIN_OUT_LASER			GPIO_Pin_3
#define PIN_OUT_LIGHT			GPIO_Pin_4
#define PIN_OUT_TOGGLE		GPIO_Pin_5
#define PIN_OUT_ACTUATOR	GPIO_Pin_6
#define PIN_OUT_MASK			(PIN_OUT_LASER | PIN_OUT_LIGHT | PIN_OUT_TOGGLE)

#define PROGRESS_MASK		0x3FF
#define ON							0x01
#define OFF							0x00

UINT8 laser_state = OFF;
UINT8 complete = OFF;

static void InitGpio(void)
{
	RCC->AHBENR |= 0x060000;	// PA and PB clock active
	GPIOA->MODER |= 0x55555;	// PA0 - PA9 outputs
	GPIOB->MODER = 0x1540;		// PB3 - PB6 outputs
	GPIOB->PUPDR = 0x2A;			// PB0 - PB2 pull-down
}

static void InitInterupts(void)
{
	
}

static void IncBar(void)
{
	UINT16 progress = GPIOA->ODR & PROGRESS_MASK;
	
	laser_state = ON;
	
	if (progress)
		progress = (progress >> 1) | 0x200;
	else 
		progress = 0x200;
	
	GPIOA->ODR |= progress;
	
	if ((progress & PROGRESS_MASK) == PROGRESS_MASK)
	{
		SET_BIT(GPIOB->ODR, PIN_OUT_LASER);
		if (CHK_BIT(GPIOB->ODR, PIN_OUT_MASK))
			complete = ON;
	}
}

static void DecBar(void)
{
	UINT32 progress = GPIOA->ODR & PROGRESS_MASK;
	
	progress = (progress << 1) | (~PROGRESS_MASK);
	
	GPIOA->ODR &= progress;
	
	if (!(progress & PROGRESS_MASK))
		laser_state = OFF;
}

int main(void)
{
	InitGpio();
		while(!complete)
		{
			/***************** LASER *******************/
			while (CHK_BIT(GPIOB->IDR, PIN_IN_LASER))
			{
				IncBar();
				delay_long(4);
			}
			if (laser_state == ON)
			{
				DecBar();
				delay_long(1);
			}
			/*******************************************/
			
			/***************** LIGHT *******************/
			if(CHK_BIT(GPIOB->IDR, PIN_IN_LIGHT))
				BIT_SET(GPIOB->ODR, PIN_OUT_LIGHT);
			else
				BIT_CLR(GPIOB->ODR, PIN_OUT_LIGHT);
			/*******************************************/
			
			/**************** TOGGLE *******************/
			if(CHK_BIT(GPIOB->IDR, PIN_IN_TOGGLE))
				BIT_SET(GPIOB->ODR, PIN_OUT_TOGGLE);
			else
				BIT_CLR(GPIOB->ODR, PIN_OUT_TOGGLE);
			/********************************************/
		}
		BIT_SET(GPIOB->ODR, PIN_OUT_ACTUATOR);
}
