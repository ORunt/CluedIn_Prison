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

//#define INTERRUPTS_ON       // Toggle this if I ever feel like fixing the interrupts

#define PIN_IN_LASER          GPIO_Pin_0
#define PIN_IN_LIGHT          GPIO_Pin_1
#define PIN_IN_TOGGLE         GPIO_Pin_2
#define PIN_OUT_LASER         GPIO_Pin_3
#define PIN_OUT_LIGHT         GPIO_Pin_4
#define PIN_OUT_TOGGLE        GPIO_Pin_5
#define PIN_IN_ACTUATOR_REV   GPIO_Pin_6
#define PIN_OUT_ACTUATOR_FWD  GPIO_Pin_7
#define PIN_OUT_ACTUATOR_REV  GPIO_Pin_8

#define PIN_OUT_MASK          (PIN_OUT_LASER | PIN_OUT_LIGHT | PIN_OUT_TOGGLE)

#define PROGRESS_MASK   0x3FF
#define ON              0x01
#define OFF             0x00
#define TRUE            0x01
#define FALSE           0x00

UINT8 laser_state = OFF;
UINT8 complete = OFF;

static void InitGpio(void)
{
	RCC->AHBENR |= 0x060000;  // PA and PB clock active
	GPIOA->MODER |= 0x55555;  // PA0 - PA9 outputs
  GPIOB->MODER = SetOutput(PIN_OUT_LASER | PIN_OUT_LIGHT | PIN_OUT_TOGGLE | PIN_OUT_ACTUATOR_FWD | PIN_OUT_ACTUATOR_REV);
	GPIOB->PUPDR = SetPullDown(PIN_IN_LASER | PIN_IN_LIGHT | PIN_IN_TOGGLE | PIN_IN_ACTUATOR_REV);
}

static void ActuatorInterruptRoutine(void)
{
  if(CHK_BIT(GPIOB->IDR, PIN_IN_ACTUATOR_REV))
    BIT_SET(GPIOB->ODR, PIN_OUT_ACTUATOR_REV);
  else
    BIT_CLR(GPIOB->ODR, PIN_OUT_ACTUATOR_REV);
}

static void ToggleInterruptRoutine(void)
{
  if(CHK_BIT(GPIOB->IDR, PIN_IN_TOGGLE))
    BIT_SET(GPIOB->ODR, PIN_OUT_TOGGLE);
  else
    BIT_CLR(GPIOB->ODR, PIN_OUT_TOGGLE);
}

static void LightPicInterruptRoutine(void)
{
  if(CHK_BIT(GPIOB->IDR, PIN_IN_LIGHT))
    BIT_SET(GPIOB->ODR, PIN_OUT_LIGHT);
  else
    BIT_CLR(GPIOB->ODR, PIN_OUT_LIGHT);
}

#ifdef INTERRUPTS_ON
static void InitInterupts(void)
{
  EXTI_InitTypeDef   EXTI_InitStructure;
  NVIC_InitTypeDef   NVIC_InitStructure;
  
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; // Enable the flippen clock!
  
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource1);
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource2);  
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource6); 
  
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
  EXTI_InitStructure.EXTI_Line = EXTI_Line6;
  EXTI_Init(&EXTI_InitStructure);

  // Enable and set EXTI0_1 Interrupt
  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 0x00;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  // Enable and set EXTI2_3 Interrupt
  NVIC_InitStructure.NVIC_IRQChannel = EXTI2_3_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  
  // Enable and set EXTI4_15 Interrupt
  NVIC_InitStructure.NVIC_IRQChannel = EXTI4_15_IRQn;
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

static void AllInterruptRoutines(void){}

#else
static void InitInterupts(void){}
static void DisableInterrupts(void){}
  
static void AllInterruptRoutines(void)
{
  ActuatorInterruptRoutine();
  ToggleInterruptRoutine();
  LightPicInterruptRoutine();
}
#endif

static void IncBar(void)
{
  UINT16 progress = GPIOA->ODR & PROGRESS_MASK;
	
  if (progress != PROGRESS_MASK)
  {
    if (progress)
      progress = (progress >> 1) | 0x200;
    else 
      progress = 0x200;
    
    GPIOA->ODR |= progress;
    delay_long(4);
  }
}

static void DecBar(void)
{
  UINT32 progress = GPIOA->ODR & PROGRESS_MASK;
  
	if(progress)
  {
    progress = (progress << 1) | (~PROGRESS_MASK);
    
    GPIOA->ODR &= progress;
    delay_long(1);
  }
}

int main(void)
{
	InitGpio();
  InitInterupts();
  
  // TODO: Maybe put something in here that auto brings the actuator down.
  
  while(!complete)
  {
    AllInterruptRoutines();
    
    /***************** LASER2 *******************/
    if(CHK_BIT(GPIOB->IDR, PIN_IN_LASER))
      IncBar();
    else if (GPIOA->ODR & PROGRESS_MASK)
      DecBar();
    
    if (CHK_BIT(GPIOA->ODR, PROGRESS_MASK)) // If it hit the top
      BIT_SET(GPIOB->ODR, PIN_OUT_LASER);
    else
      BIT_CLR(GPIOB->ODR, PIN_OUT_LASER);
    
    if (CHK_BIT(GPIOB->ODR, PIN_OUT_MASK))
        complete = TRUE;
  }
  DisableInterrupts();  // TODO: for non interrupts, disable leds so they can't be reacivated
  BIT_SET(GPIOB->ODR, PIN_OUT_ACTUATOR_FWD);
  delay_long(40);
  delay_long(40);
  BIT_CLR(GPIOB->ODR, PIN_OUT_ACTUATOR_FWD);
  for(;;)
  {
#ifndef INTERRUPTS_ON
    ActuatorInterruptRoutine();
    delay_short();
#endif
  }
}

#ifdef INTERRUPTS_ON
/***************** LIGHT *******************/
void EXTI0_1_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line1) != RESET)
  {
    LightPicInterruptRoutine();
    delay_nano();
    
    EXTI_ClearITPendingBit(EXTI_Line1);
  }
}


/**************** TOGGLE *******************/
void EXTI2_3_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line2) != RESET)
  {    
    ToggleInterruptRoutine();
    delay_nano();
    EXTI_ClearITPendingBit(EXTI_Line2);
  }
}

/**************** ACTUATOR *******************/
void EXTI4_15_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line6) != RESET)
  {
    ActuatorInterruptRoutine();
    delay_nano();
    
    EXTI_ClearITPendingBit(EXTI_Line6);
  }
}
#endif
