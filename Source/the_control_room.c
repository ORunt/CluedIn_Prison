/* Brief
    This is the program for the central processing when all clues are complete,
    then it activates the final clue!
*/

#include "stm32f0xx.h"
#include "definitions.h"
#include "utils.h"
#include "stm32f0xx_it.h"
#include "stm32f0xx_syscfg.h"
#include "stm32f0xx_exti.h"
#include "stm32f0xx_misc.h"


#define PIN_IN_LASER          GPIO_Pin_0
#define PIN_IN_LIGHT          GPIO_Pin_1
#define PIN_IN_TOGGLE         GPIO_Pin_2
#define PIN_OUT_LASER         GPIO_Pin_3
#define PIN_OUT_LIGHT         GPIO_Pin_4
#define PIN_OUT_TOGGLE        GPIO_Pin_5
#define PIN_OUT_ACTUATOR_FWD  GPIO_Pin_6
#define PIN_OUT_ACTUATOR_REV  GPIO_Pin_7
#define PIN_OUT_MASK          (PIN_OUT_LASER | PIN_OUT_LIGHT | PIN_OUT_TOGGLE)

#define PROGRESS_MASK   0x3FF
#define ON              0x01
#define OFF             0x00

UINT8 laser_state = OFF;
UINT8 complete = OFF;

static void InitGpio(void)
{
	RCC->AHBENR |= 0x060000;  // PA and PB clock active
	GPIOA->MODER |= 0x55555;  // PA0 - PA9 outputs
	GPIOB->MODER = 0x5540;    // PB3 - PB7 outputs
	GPIOB->PUPDR = 0x2002A;   // PB0 - PB2, PB7 pull-down
}

static void InitInterupts(void)
{
  EXTI_InitTypeDef   EXTI_InitStructure;
  NVIC_InitTypeDef   NVIC_InitStructure;
  
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; // Enable the flippen clock!
  
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource1);
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource2);  
  
  // Configure EXTI1 line
  EXTI_InitStructure.EXTI_Line = EXTI_Line1;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  
  // Configure EXTI2 line
  EXTI_InitStructure.EXTI_Line = EXTI_Line2;
  EXTI_Init(&EXTI_InitStructure);
  
  // Configure EXTI8 line
  EXTI_InitStructure.EXTI_Line = EXTI_Line8;
  EXTI_Init(&EXTI_InitStructure);

  // Enable and set EXTI0_1 Interrupt
  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 0x00;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  // Enable and set EXTI2_3 Interrupt
  NVIC_InitStructure.NVIC_IRQChannel = EXTI2_3_IRQn;
  NVIC_Init(&NVIC_InitStructure);
}

static void DisableInterrupts(void)
{
  EXTI_InitTypeDef   EXTI_InitStructure;
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line1;
  EXTI_InitStructure.EXTI_LineCmd = DISABLE;
  EXTI_Init(&EXTI_InitStructure);
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line2;
  EXTI_Init(&EXTI_InitStructure);
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
  InitInterupts();
  
  // TODO: Maybe put something in here that auto brings the actuator down.
  
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
  }
  DisableInterrupts();
  BIT_SET(GPIOB->ODR, PIN_OUT_ACTUATOR_FWD);
  delay_long(40);
  BIT_CLR(GPIOB->ODR, PIN_OUT_ACTUATOR_FWD);
}


/***************** LIGHT *******************/
void EXTI0_1_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line1) != RESET)
  {
    delay_short();
    
    if(CHK_BIT(GPIOB->IDR, PIN_IN_LIGHT))
      BIT_SET(GPIOB->ODR, PIN_OUT_LIGHT);
    else
      BIT_CLR(GPIOB->ODR, PIN_OUT_LIGHT);
    
    EXTI_ClearITPendingBit(EXTI_Line1);
  }
}


/**************** TOGGLE *******************/
void EXTI2_3_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line2) != RESET)
  {
    delay_short();
    
    if(CHK_BIT(GPIOB->IDR, PIN_IN_TOGGLE))
      BIT_SET(GPIOB->ODR, PIN_OUT_TOGGLE);
    else
      BIT_CLR(GPIOB->ODR, PIN_OUT_TOGGLE);
    
    EXTI_ClearITPendingBit(EXTI_Line2);
  }
}

/**************** TOGGLE *******************/
void EXTI4_15_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line8) != RESET)
  {
    delay_short();
    
    if(CHK_BIT(GPIOB->IDR, PIN_OUT_ACTUATOR_REV))
      BIT_SET(GPIOB->ODR, PIN_OUT_ACTUATOR_REV);
    else
      BIT_CLR(GPIOB->ODR, PIN_OUT_ACTUATOR_REV);
    
    EXTI_ClearITPendingBit(EXTI_Line8);
  }
}
