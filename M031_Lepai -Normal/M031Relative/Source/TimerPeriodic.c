#include <sys.h>
#include <stdio.h>
#include <pwm_light.h>

void TIMER_GPIO_Init()
{
	/* Enable TIMER module clock */
	CLK_EnableModuleClock(TMR0_MODULE);
	CLK_EnableModuleClock(TMR1_MODULE);
	CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HIRC, 0);
	CLK_SetModuleClock(TMR1_MODULE, CLK_CLKSEL1_TMR1SEL_PCLK0, 0);
}

void TIMER_Init(void){
	  TIMER_Open(TIMER0, TIMER_PERIODIC_MODE,100);
    TIMER_EnableInt(TIMER0);
    NVIC_EnableIRQ(TMR0_IRQn);
    TIMER_Start(TIMER0);
	
		TIMER_Open(TIMER1, TIMER_PERIODIC_MODE,2);//电量检测时钟
    TIMER_EnableInt(TIMER1);
    NVIC_EnableIRQ(TMR1_IRQn);
    TIMER_Start(TIMER1);
}

///////////////////////////////////////////
extern uint8_t LEDOnWork;
uint8_t LEDBlinkColor=0;
void LEDBlink()
{	
	if(!LEDOnWork)
	{
		LEDBlinkColor=!LEDBlinkColor;
		LEDChange(LEDBlinkColor);
	}
}


////////////////////////////////////////////
extern void BtnPressTimeCounter(void);

uint8_t timer0flag=0;

void TMR0_IRQHandler(void)    //10ms中断一次                
{ 
	TIMER_ClearIntFlag(TIMER0);
	timer0flag=1;
	//BtnPressTimeCounter();
}


extern void PoweBtnLongPressHandler(void);
extern void RGB_Blink(void);

uint8_t time0TickCounter=0;
uint8_t	OneSecTickFlag=0;
void OneSecTickGenerator(void)
{
	time0TickCounter++;
	if(time0TickCounter>99)
	{
		OneSecTickFlag=1;
		time0TickCounter=0;
	}
}

void OneSecRound(void)
{
	if(OneSecTickFlag)
	{
		PoweBtnLongPressHandler();
		//LEDBlink();     //To test M031 still alive!!! 
		OneSecTickFlag=0;
	}
}

void TenMicSecRound(void)
{
	if(timer0flag)
	{
		OneSecTickGenerator();
		BtnPressTimeCounter();
		RGB_Blink();
		timer0flag=0;
	}
}

extern void I2C1PowerSpy(void);
extern uint8_t i2c1InUseFlag;
extern uint8_t i2c0InUseFlag;
void TMR1_IRQHandler(void)                              //used for  9Sensor and powerSpy
{
		if(TIMER_GetIntFlag(TIMER1) == 1)
    {
      //Clear Timer1 time-out interrupt flag
      TIMER_ClearIntFlag(TIMER1);
			if(!i2c1InUseFlag&&(!i2c0InUseFlag))
			{	
				I2C1PowerSpy();
			}
    }
}

