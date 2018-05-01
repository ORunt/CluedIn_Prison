//#include <stm32f0xx_rcc.h>
#include <stm32f0xx_adc.h>
//#include <stm32f0xx_gpio.h>

void InitAdc(void)
{
  ADC_InitTypeDef     ADC_InitStructure;
  //GPIO_InitTypeDef    GPIO_InitStructure;

  //RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);   // GPIOA Periph clock enable
  //RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);  // ADC1  Periph clock enable
  
  /* Configure ADC Channel11 as analog input */
#ifdef USE_STM320518_EVAL
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 ;
#else
  //GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 ;
#endif /* USE_STM320518_EVAL */
  //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  //GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  //GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  /* ADCs DeInit */  
  ADC_DeInit(ADC1);
  
  /* Initialize ADC structure */
  ADC_StructInit(&ADC_InitStructure);
  
  /* Configure the ADC1 in continuous mode with a resolution equal to 12 bits  */
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; 
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_ScanDirection = ADC_ScanDirection_Upward;
  ADC_Init(ADC1, &ADC_InitStructure); 
  
  /* Convert the ADC1 Channel 11 with 239.5 Cycles as sampling time */ 
#ifdef USE_STM320518_EVAL
  ADC_ChannelConfig(ADC1, ADC_Channel_11 , ADC_SampleTime_239_5Cycles);
#else
  ADC_ChannelConfig(ADC1, ADC_Channel_10 , ADC_SampleTime_239_5Cycles);
#endif /* USE_STM320518_EVAL */

  /* ADC Calibration */
  ADC_GetCalibrationFactor(ADC1);
  
  /* Enable the ADC peripheral */
  ADC_Cmd(ADC1, ENABLE);     
  
  /* Wait the ADRDY flag */
  while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADRDY)); 
  
  /* ADC1 regular Software Start Conv */ 
  ADC_StartOfConversion(ADC1);
  
}


int main()
{
	InitAdc();
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
