#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include <stm32f0xx_tim.h>
#include "definitions.h"
#include "utils.h"

#define FAN 0x0001

static void SetFan(UINT8 on_off)
{
  if(!CHK_BIT(GPIOB->ODR, FAN) && (on_off == 1))
    BIT_SET(GPIOB->ODR, FAN);
  
  else if(CHK_BIT(GPIOB->ODR, FAN) && (on_off == 0))
    BIT_CLR(GPIOB->ODR, FAN);
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

int main (void)
{
  UINT8 fan_status = 0;
  
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
    fan_status = 0;
    SelectADC(6);
    if (ADC1->DR < 0xFF)
    {
      SelectADC(5);
      if (ADC1->DR < 0xFF)
        fan_status = 1;
    }
    SetFan(fan_status);
		delay();
	}
}
