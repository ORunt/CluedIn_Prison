#include "stm32f0xx.h"
#include "definitions.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

// Inputs
#define TRIGGER				  0x01	// GPIOB_0 - no pull
#define EXT_CTRL			  0x40	// GPIOB_6 - no pull
#define TIMER_COMPLETE  0x100 // GPIOB_8 - no pull

// Outputs
#define ALARM					0x02	// GPIOB_1 - no pull
#define LIGHT_CELL		0x04	// GPIOB_2 - no pull
#define LIGHT_GUARDS	0x08	// GPIOB_3 - no pull
#define LIGHT_CONTROL	0x10	// GPIOB_4 - no pull
#define LIGHT_UV			0x20	// GPIOB_5 - no pull
#define TIMER_CMD     0x80  // GPIOB_7 - no pull

// Keypad
#define ROW_1			GPIO_Pin_0	// GPIOA_0
#define ROW_2			GPIO_Pin_1	// GPIOA_1
#define ROW_3			GPIO_Pin_2	// GPIOA_2
#define ROW_4			GPIO_Pin_3	// GPIOA_3
#define COL_1			GPIO_Pin_4	// GPIOA_4
#define COL_2			GPIO_Pin_5	// GPIOA_5
#define COL_3			GPIO_Pin_6	// GPIOA_6

const UINT8 secret_code[4] = {1,2,3,4};
const UINT8 cmd_ok[16] = {0x00, 0x00, 0xFF, 0x55, 0x55, 0x77, 0x77, 0x77, 0x77, 0x75, 0xD5, 0x55, 0xD7, 0x77, 0x77, 0x7F};
const UINT8 cmd_al2[16] = {0x00, 0x00, 0xFF, 0x55, 0x55, 0x77, 0x77, 0x77, 0x77, 0x5D, 0xDD, 0x55, 0xD5, 0x77, 0x77, 0x7F};
const UINT8 cmd_on[16] = {0x00, 0x00, 0xFF, 0x55, 0x55, 0x77, 0x77, 0x77, 0x77, 0x55, 0x55, 0x77, 0x77, 0x77, 0x77, 0x7F};
UINT8 code_buffer[4] = {0};
UINT8 retry_counter = 0;
UINT16 offset = 0;
UINT8 alarm_on = 0;


static void DelayUs(UINT32 micros){
	micros *= (8000000 / 1000000);
	
	//substitute 8 cycles for each call of asm code below == //micros /= 8; 
	__asm(" mov r0, micros \n"
	"loop: subs r0, #8 \n"
	" bhi loop \n");
}

static void SendCmd(const UINT8 *cmd, UINT8 len)
{
  UINT8 i,j;
  
  for(i = 0; i < len; i++)
  {
    for(j = 7; j <= 7; j--)
    {
      if(cmd[i] & (1 << j))
        BIT_SET(GPIOB->ODR, TIMER_CMD);
      else
        BIT_CLR(GPIOB->ODR, TIMER_CMD);
      DelayUs(635);
    }
  }
}

static void initgpio(void)
{
	RCC->AHBENR |= 0x060000;	// This must be changed later on the actual puzzle
	
	// Port A
	GPIOA->MODER |= (COL_1 * COL_1) | (COL_2 * COL_2) | (COL_3 * COL_3);	// Columns are outputs
	GPIOA->PUPDR |= (ROW_1 * ROW_1) | (ROW_2 * ROW_2) | (ROW_3 * ROW_3) | (ROW_4 * ROW_4); // Rows (inputs) are pull up
	GPIOA->OSPEEDR |= 0xFFFF;   // High speed
	GPIOA->ODR |= 0xFFF;
	
	// Port B
	GPIOB->MODER = SetOutput(ALARM | LIGHT_CELL | LIGHT_GUARDS | LIGHT_CONTROL | LIGHT_UV | TIMER_CMD);
	GPIOB->PUPDR = 0;//SetPullDown(LIGHT_CELL); // no push or pull
}

static void winner (void)
{
	
}

static void loser (void)
{	
	
}

/*
static void play_tone(void)
{
	GPIOB->ODR = SPEAKER;
	delay_long(1);
	GPIOB->ODR = 0;
}
*/

void StartTimer(void)
{
  delay_long(10);
  SendCmd(cmd_on, sizeof(cmd_on));
  delay_long(10);
  SendCmd(cmd_al2, sizeof(cmd_al2));
  delay_long(10);
  SendCmd(cmd_ok, sizeof(cmd_ok));
  delay_long(10);
}

static UINT8 CheckTimerTimeout(void)
{
  UINT8 timer_complete_loop = 0;
  UINT16 i;
  
  for (i = 0; i < 400; i++)
    timer_complete_loop |= CHK_BIT(GPIOB->IDR, TIMER_COMPLETE);
  
  return (timer_complete_loop == 0);
}

static void disp (UINT8 num)
{
	//play_tone();
	
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

static void AlarmLoop(void)
{
	while (alarm_on)
	{
		GPIOA->BSRR = COL_3;	//set bit as high  COL 3
		GPIOA->BSRR = COL_2;	//set bit as high  COL 2
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
		
		GPIOA->BSRR = COL_1;	//set bit as high
		GPIOA->BSRR = COL_3;	//set bit as high
		GPIOA->BRR = COL_2;	//set bit as low
		
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
		
		GPIOA->BSRR = COL_2;	//set bit as high
		GPIOA->BSRR = COL_1;	//set bit as high
		GPIOA->BRR = COL_3;	//set bit as low
		
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
}

int main (void)
{
  UINT8 emergency_mode = 1;
	initgpio();
	
  // Initially all the lights are on, and the timer command line must be high.
	BIT_SET(GPIOB->ODR, LIGHT_CELL | LIGHT_GUARDS | LIGHT_CONTROL | TIMER_CMD);
	
	for(;;)
	{
    // 1. Wait for the prison door lock to be openened
		while (!CHK_BIT(GPIOB->IDR, TRIGGER));
    
    // 2. Turn all the lights off
		BIT_CLR(GPIOB->ODR, LIGHT_CELL | LIGHT_GUARDS | LIGHT_CONTROL);
    
    // 3. Turn the UV light and Alarm on. 
		BIT_SET(GPIOB->ODR, LIGHT_UV | ALARM);
    
    // 4. Set Timer to start counting down
    StartTimer();
    
    // 5. Wait in keypad for alarm loop, while also waiting for countdown timer to complete.
    while (emergency_mode)
    {
      AlarmLoop();
      if (CheckTimerTimeout())
        emergency_mode = 0;
    }
    
    // 6. Turn all the lights on
		BIT_SET(GPIOB->ODR, LIGHT_CELL | LIGHT_GUARDS | LIGHT_CONTROL);
    
    // 7. Turn the UV light and Alarm off. 
		BIT_CLR(GPIOB->ODR, LIGHT_UV | ALARM);
		
    while(1);
		//
	}
}
