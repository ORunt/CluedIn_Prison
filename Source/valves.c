#define STM32F051

#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_tim.h>

void InitializeTimer(int period = 500)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    TIM_TimeBaseInitTypeDef timerInitStructure;
    timerInitStructure.TIM_Prescaler = 40000;
    timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    timerInitStructure.TIM_Period = period;
    timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    timerInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM4, &timerInitStructure);
    TIM_Cmd(TIM4, ENABLE);
}

void InitializePWMChannel()
{
	TIM_OCInitTypeDef outputChannelInit = {0,};
	outputChannelInit.TIM_OCMode = TIM_OCMode_PWM1;
	outputChannelInit.TIM_Pulse = 400;
	outputChannelInit.TIM_OutputState = TIM_OutputState_Enable;
	outputChannelInit.TIM_OCPolarity = TIM_OCPolarity_High;

	TIM_OC1Init(TIM4, &outputChannelInit);
	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);

	GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_TIM4);
}

void InitializePWMChannel2()
{
	TIM_OCInitTypeDef outputChannelInit = {0,};
	outputChannelInit.TIM_OCMode = TIM_OCMode_PWM1;
	outputChannelInit.TIM_Pulse = 100;
	outputChannelInit.TIM_OutputState = TIM_OutputState_Enable;
	outputChannelInit.TIM_OCPolarity = TIM_OCPolarity_High;

	TIM_OC2Init(TIM4, &outputChannelInit);
	TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);

	GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_TIM4);
}

void InitializeLEDs()
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    GPIO_InitTypeDef gpioStructure;
    gpioStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
    gpioStructure.GPIO_Mode = GPIO_Mode_AF;
    gpioStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpioStructure);
}


int main()
{
	InitializeLEDs();
	InitializeTimer();
	InitializePWMChannel();
	InitializePWMChannel2();
	for (;;)
	{
	}
}








/*
#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include <stm32f0xx_tim.h>
#include "definitions.h"
#include "utils.h"

UINT8 run = 1;

static void StartFan(void)
{
	run = 0;
	GPIOB->ODR = 0x01;
}

static void SelectADC(UINT8 adc)
{
	switch(adc)
	{
		case 5: 
			ADC1->CHSELR = ADC_CHSELR_CHSEL5;		// set ADC channel for **POT0** = PA5 = channel 5
			break;
		case 6: 
			ADC1->CHSELR = ADC_CHSELR_CHSEL6; 	// set ADC channel for **POT1** = PA6 = channel 6
			break;
		default:
			return;
	}
	BIT_SET(ADC1->CR, ADC_CR_ADSTART);			// kick off conversion
	while( (ADC1->ISR & ADC_ISR_EOC) == 0);	// wait for conversion complete
}

static void SetupTimers(void)
{
	
}

int main (void)
{
	// Setup Light (Fan)
	BIT_SET(RCC->AHBENR, RCC_AHBENR_GPIOBEN);// enable clock for GPIOB
	GPIOB->MODER = 0x01;
	GPIOB->ODR = 0x00;
	
	// Setup ADC
	BIT_SET(RCC->AHBENR, RCC_AHBENR_GPIOAEN);// enable clock for GPIOB
	BIT_SET(GPIOA->MODER, GPIO_MODER_MODER6);	// set pin connected to **POT1** (PA6) to be in analogue mode
	BIT_SET(GPIOA->MODER, GPIO_MODER_MODER5);	// set pin connected to **POT0** (PA5) to be in analogue mode
	BIT_SET(RCC->APB2ENR, RCC_APB2ENR_ADCEN);	// enable clock for ADC
	BIT_SET(ADC1->CR, ADC_CR_ADEN);						// enable ADC
	while( (ADC1->ISR & ADC_ISR_ADRDY) == 0);	// wait for ADC ready
	BIT_SET(ADC1->CFGR1, ADC_CFGR1_RES_0); 		// set rolutions/alignment if necessary. 
	BIT_CLR(ADC1->CFGR1, ADC_CFGR1_RES_1);		//		10-bit res is advised. Done by setting bit 3 and clearing bit 4
	
	for(;;)
	{
		if (run)
		{
			SelectADC(6);
			if (ADC1->DR < 0xFF)
			{
				SelectADC(5);
				if (ADC1->DR < 0xFF)
					StartFan();
			}		
		}
		delay();
	}
}*/
