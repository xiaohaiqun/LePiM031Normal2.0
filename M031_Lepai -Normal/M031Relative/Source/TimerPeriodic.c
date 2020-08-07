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
}

///////////////////////////////////////////
extern uint8_t LEDOnWork;
uint8_t LEDBlinkColor=0;
void LEDBlinkTest()
{	
	//if(!LEDOnWork)
	{
		LEDBlinkColor=!LEDBlinkColor;
		LEDChange(LEDBlinkColor);
	}
}
///////////////////////////////////////////
static uint8_t redBlinkTimes=0;
static uint8_t lowPowerRejectBootLed=1;
void redLedBlinkTimers(uint8_t times)
{
	redBlinkTimes=2*times;
	lowPowerRejectBootLed=1;
}
extern void RGBConfig(uint8_t r,uint8_t g,uint8_t b);
void redBlink()
{
	if(redBlinkTimes>0)
	{
		if(lowPowerRejectBootLed)
		{
			RGBConfig(60,0,0);
			lowPowerRejectBootLed= ! lowPowerRejectBootLed;
		}
		else
		{
			{
			RGBConfig(0,0,0);
			lowPowerRejectBootLed= ! lowPowerRejectBootLed;
			}
		}
		redBlinkTimes--;
		if(redBlinkTimes==0)
			LEDChange( 0 );
	}
}

////////////////////////////////////////////
extern void BtnPressTimeCounter(void);

uint8_t timer0flag=0;

void TMR0_IRQHandler(void)    //10ms中断一次                
{ 
	TIMER_ClearIntFlag(TIMER0);
	timer0flag=1;
}


extern void PoweBtnLongPressHandler(void);
extern void RGB_Blink(void);

uint8_t time0TickCounter=0;
uint8_t	OneSecTickFlag=0;
uint8_t halfSecTickFlag=0;
extern void TimePriod9SensorReadHandler(void);
void OneSecTickGenerator(void)
{
	time0TickCounter++;
	if((time0TickCounter%50)==0)
	{
		halfSecTickFlag=1;
	}
	/*if((time0TickCounter%5)==0)//20Hz
	{
		
	}*/
	if(time0TickCounter>99)
	{
		OneSecTickFlag=1;
		time0TickCounter=0;		
	}
}
extern void ChargeAndLowPowerLedDisplay(void);
extern void I2C1PowerSpy(void);
extern void doubleClikPowerChip(void);
uint8_t secondTickCounter=0;
extern uint8_t PowerState;
void halfSecRound(void)
{
	if(halfSecTickFlag)
	{
		halfSecTickFlag=0;
		if(PB12)//电源芯片I2C在工作状态。  &&PowerState
		{				
			ChargeAndLowPowerLedDisplay();		
		}
		redBlink();
	}
}
extern void powerDataReadRound(void);
void OneSecRound(void)
{
	halfSecRound( );
	if(OneSecTickFlag)
	{
		PoweBtnLongPressHandler();
		//LEDBlinkTest();     //To test M031 still alive!!! 
		OneSecTickFlag=0;	
		powerDataReadRound();
	}
}

void TenMicSecRound(void)
{
	if(timer0flag)//timer0flag
	{
		OneSecTickGenerator();
		BtnPressTimeCounter();
		TimePriod9SensorReadHandler();
		RGB_Blink();
		timer0flag=0;
		//CLK_SysTickDelay(10000);
	}
}

