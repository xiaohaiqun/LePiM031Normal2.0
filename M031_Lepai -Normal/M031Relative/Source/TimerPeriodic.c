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

extern uint8_t i2cStartFlag;
extern uint8_t i2cWaitCount;
extern void BtnPressTimeCounter();
uint8_t timer0flag=0;
uint8_t time0Tick=0;
void TMR0_IRQHandler(void)    //10ms中断一次                
{ 
	TIMER_ClearIntFlag(TIMER0);
	timer0flag=1;
	BtnPressTimeCounter();
	if(i2cStartFlag)
	{
		i2cWaitCount++;
	}
}


extern void PoweBtnLongPressHandler();
extern void RGB_Blink(void);
uint8_t Timer0Tick=0;
void Btn9CtlOnOffHandler()
{
	
	if(timer0flag)
	{
		Timer0Tick++;
		if(Timer0Tick>100)
		{
			Timer0Tick=0;
			PoweBtnLongPressHandler();
			LEDBlink();
		}
		RGB_Blink();
		timer0flag=0;
	}
}


extern void I2C1PowerSpy();
uint8_t timer1flag=0;
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

