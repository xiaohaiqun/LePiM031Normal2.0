#include <stdio.h>
#include <sys.h>
#include <gpio.h>
#include <pwm.h>
#include <pwm_light.h>
uint8_t LEDOnWork=0;
void PWM_GPIO_Init(){
	CLK_SetModuleClock(PWM0_MODULE, CLK_CLKSEL2_PWM0SEL_PCLK0, 0);
	SYS_ResetModule(PWM0_RST);
	SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA0MFP_Msk |
                                       SYS_GPA_MFPL_PA1MFP_Msk |
                                       SYS_GPA_MFPL_PA2MFP_Msk |
                                       SYS_GPA_MFPL_PA3MFP_Msk ))|
                    (SYS_GPA_MFPL_PA0MFP_PWM0_CH5 |
                     SYS_GPA_MFPL_PA1MFP_PWM0_CH4 |
                     SYS_GPA_MFPL_PA2MFP_PWM0_CH3 |
                     SYS_GPA_MFPL_PA3MFP_PWM0_CH2 );

   SYS->GPF_MFPL = (SYS->GPF_MFPL & ~(SYS_GPF_MFPL_PF4MFP_Msk |
                                       SYS_GPF_MFPL_PF5MFP_Msk)) |
                    (SYS_GPF_MFPL_PF4MFP_PWM0_CH1 |
                     SYS_GPF_MFPL_PF5MFP_PWM0_CH0 );
}
//uint8_t m=0,n=0;
void RGBConfig(uint8_t r,uint8_t g,uint8_t b)
{
	PWM_ConfigOutputChannel(PWM0,redpin1 , 2400000,(100-r));
	PWM_ConfigOutputChannel(PWM0,redpin2 , 2400000,(100-r));
	PWM_ConfigOutputChannel(PWM0,greenpin1 , 2400000,(100-g));
	PWM_ConfigOutputChannel(PWM0,greenpin2 , 2400000,(100-g));
	PWM_ConfigOutputChannel(PWM0,bluepin1 , 2400000,(100-b));
	PWM_ConfigOutputChannel(PWM0,bluepin2 , 2400000,(100-b));
}

uint8_t onOffBlink=0;
uint8_t powerOnLightFlag=1;
void powerOnLight(){
	int m, n;
	LEDChange(dark);
	PWM_EnableOutput(PWM0, 0x3F);
	PWM_Start(PWM0, 0x3F);
	for(m=0;m<8;m++){
		 for(n=0;n<100;n++){
			RGBConfig( n, 100-n, n);
			CLK_SysTickDelay(1000000000);
			CLK_SysTickDelay(1000000000);
			CLK_SysTickDelay(1000000000);	 
		}
	}
	LEDChange(dark);
	CLK_SysTickDelay(1000000000);
	 //PWM_ForceStop(PWM0, 0x3F);	
}



int RGBBlinkTimes=0,n=0;
void RGB_Blink(void)
{
	if(RGBBlinkTimes>0)
	{
		RGBConfig(n, 100-n, n);
		n++;
		if(n==100)
		{
			RGBBlinkTimes--;
			n=0;
		}
	}
}


void LEDChange(uint8_t Ledstate)
{
		uint8_t r=0,g=0,b=0;
		switch(Ledstate){				
			case LedON:
				PWM_EnableOutput(PWM0, 0x38);
				PWM_Start(PWM0, 0x38);
				break;
			case LedOFF:
				r=0,g=0,b=0;
				//PWM_ForceStop(PWM0, 0x3F);
				break;
			case red:
				r=100;
				break;
			case green:
				g=100;
				break;
			case lightgreen:
				g=30;
				break;
			case blue:
				b=100;
				break;
			case yellow:
				r=100,g=100;
				break;
			case dark:
				r=0,g=0,b=0;
			default:
				break;			
		}			
		RGBConfig( r, g, b);
}


void LED1Change(uint8_t Ledstate)
{
		uint8_t r=100,g=100,b=100;
		switch(Ledstate){				
			case LedON:
				PWM_EnableOutput(PWM0, 0x38);
				PWM_Start(PWM0, 0x38);
				break;
			case LedOFF:
				r=100,g=100,b=100;
				//PWM_ForceStop(PWM0, 0x3F);
				break;
			case red:
				r=0;
				break;
			case green:
				g=0;
				break;
			case blue:
				b=0;
				break;
			case yellow:
				r=0,g=0;
				break;
			case dark:
			default:
				break;			
		}			
		PWM_ConfigOutputChannel(PWM0,redpin1 , 2400000,r);
		PWM_ConfigOutputChannel(PWM0,greenpin1 , 2400000,g);
		PWM_ConfigOutputChannel(PWM0,bluepin1 , 2400000,b);
}

void LED2Change(uint8_t Ledstate)
{
		uint8_t r=100,g=100,b=100;
		switch(Ledstate){				
			case LedON:
				PWM_EnableOutput(PWM0, 0x07);
				PWM_Start(PWM0, 0x07);
				break;
			case LedOFF:
				r=100,g=100,b=100;
				//PWM_ForceStop(PWM0, 0x3F);
				break;
			case red:
				r=0;
				break;
			case green:
				g=0;
				break;
			case blue:
				b=0;
				break;
			case yellow:
				r=0,g=0;
				break;
			case dark:
			default:
				break;			
		}			
		PWM_ConfigOutputChannel(PWM0,redpin2 , 2400000,r);
		PWM_ConfigOutputChannel(PWM0,greenpin2 , 2400000,g);
		PWM_ConfigOutputChannel(PWM0,bluepin2 , 2400000,b);
}
uint8_t rblink=0;
void blinkred()
{
	if(rblink)
	{
		LEDChange(red);
	}
	else
	{
		LEDChange(dark);
	}
	rblink=!rblink;
}


uint8_t gblink;
void blinkgreen()
{
	if(rblink)
	{
		LEDChange(green);
	}
	else
	{
		LEDChange(dark);
	}
	gblink=!gblink;
}
