#include <stdio.h>
#include "NuMicro.h"
#include <ButtonProcess.h>
#include <pwm_light.h>


//this should be added to device_GPIO_Init();
void Button_GPIO_Init(void)
{
		GPIO_SetMode(PB, (BIT13), GPIO_MODE_OUTPUT);//模拟按键控制引脚
		GPIO_SetMode(PB, (BIT12), GPIO_MODE_INPUT); //ip5328 I2C状态检测引脚
	
		GPIO_SetMode(PB, (BIT1|BIT0), GPIO_MODE_QUASI);
	  GPIO_SetMode(PF, (BIT3|BIT2|BIT15), GPIO_MODE_QUASI);
		GPIO_SetMode(PA, (BIT12|BIT13|BIT14|BIT15), GPIO_MODE_QUASI); 
	  //////////////////////////////////////////////////////////////////
	  /////for interupt raspberry//////////////////////////////////////
	  GPIO_SetMode(PB, BIT5, GPIO_MODE_QUASI);
	  PB5=0;
	  
	  //////////////////////////////////////////////////////////
	  //////for display raspberry's state////////////////////
		GPIO_SetMode(PB, BIT4, GPIO_MODE_QUASI);
		//GPIO_SetMode(PB, BIT4, GPIO_MODE_INPUT);
		GPIO_EnableInt(PB, (4), GPIO_INT_RISING); //raspberry ONOFF,上升沿触发（raspberry软关机时关机引脚变为高电平）
		//PB4=0;
    
	  //多余引脚配置成输入模式
		GPIO_SetMode(PF, BIT0|BIT1, GPIO_MODE_INPUT);
		GPIO_SetMode(PB, BIT14|BIT15, GPIO_MODE_QUASI);
	  //////////////////////////////////////////////////////
    GPIO_EnableInt(PB, (1), GPIO_INT_BOTH_EDGE); //BTN1
		GPIO_EnableInt(PB, (0), GPIO_INT_BOTH_EDGE);//BTN2
		GPIO_EnableInt(PF, (3), GPIO_INT_BOTH_EDGE);//BTN3
		GPIO_EnableInt(PF, (2), GPIO_INT_BOTH_EDGE);//BTN4
		GPIO_EnableInt(PF, (15),GPIO_INT_BOTH_EDGE);//BTN5
		GPIO_EnableInt(PA, (12),GPIO_INT_BOTH_EDGE);//BTN6
		GPIO_EnableInt(PA, (13),GPIO_INT_BOTH_EDGE);//BTN7
		GPIO_EnableInt(PA, (14),GPIO_INT_BOTH_EDGE);//BTN8
		GPIO_EnableInt(PA, (15),GPIO_INT_BOTH_EDGE);//BTN9
    NVIC_EnableIRQ(GPIO_PAPB_IRQn);
		NVIC_EnableIRQ(GPIO_PCPDPEPF_IRQn);
    /* Enable interrupt de-bounce function and select de-bounce sampling cycle time is 1024 clocks of LIRC clock */
    GPIO_SET_DEBOUNCE_TIME(GPIO_DBCTL_DBCLKSRC_LIRC, GPIO_DBCTL_DBCLKSEL_64);
    
		GPIO_ENABLE_DEBOUNCE(PB,(BIT0|BIT1|BIT4));
    GPIO_ENABLE_DEBOUNCE(PF, (BIT3|BIT2|BIT15));
		GPIO_ENABLE_DEBOUNCE(PA, (BIT12|BIT13|BIT14|BIT15));
		PB13=0;
}
//this shold be added to GPXXXXXXX_IRQHandle()

extern uint8_t PowerState;
extern uint8_t LEDOnWork;

uint8_t NowBtn=0xff;
uint8_t PowerBtnPressTime;

uint8_t Btn9timerStart=0;

uint8_t InPowerStarting=0;  //开机过程中标记
uint8_t InPowerOffFlag=0;  //关机过程中的标记


uint8_t ShutDownTime=0;
//Button9 按下相关操作
void Btn9pressHandler()
{			
	if(Btn9timerStart==0)     //计时归零
	{
		PowerBtnPressTime=0;
		Btn9timerStart=1;		//开始计时	
		LEDOnWork=1;        //占用PWM LED
	}
}
//Button 弹起相关操作
extern uint8_t shutdonwn_flag;
void Btn9releaseHandler()
{
		Btn9timerStart=0;
	  PowerBtnPressTime=0;
		LEDChange(dark);
		LEDOnWork=0;
}

extern int RGBBlinkTimes;
bool RaspberryONOFF=0;;   //标记树莓派是否确认关机。
extern uint8_t lowPowerDetect(void);
extern void redLedBlinkTimers(uint8_t);
extern void PowerStateCheck();
void PoweBtnLongPressHandler()
{
	if(!InPowerStarting)
	{
		if(Btn9timerStart)
		{
			PowerBtnPressTime++;// +1s
			if(PowerBtnPressTime==5)
			{//长按关机
				InPowerOffFlag=1; //关机中标记
				Btn9timerStart=0;
				LEDChange(yellow);
				PowerOff();     
				InPowerOffFlag=0;
			}
			else if(PowerBtnPressTime==2)//>=2会发送发送3遍
			{
				//add powerState check
				PowerStateCheck();
				if(PowerState==1){//软关机			
					NowBtn=0xA9;
					PB5=!PB5;  
					LEDChange(red);
				}
				else if(PowerState==0)//开机
				{
					InPowerStarting=1;
					Btn9timerStart=0;
					PowerBtnPressTime=0;
					LEDChange(green);
					//低电量拒绝开机控制。
					//if( lowPowerDetect() )	//如果低电，不能开机，闪红灯提示。
					//{
					//	redLedBlinkTimers(5);
					//}
					//else                   //如果电量充足，执行开机
					{
						PowerOn();
						RGBBlinkTimes=10;
					}
					LEDChange(dark);
					InPowerStarting=0;
					RaspberryONOFF=0;
				}		
			}			
		}
		if(RaspberryONOFF)
		{
			ShutDownTime++;
			if(ShutDownTime>=15)
			{
				if(PB4)//如果PB4仍然为高电平（关机信号）
				{
					if(!InPowerOffFlag)
					{
						InPowerOffFlag=1;
						PowerOff();       //Shut down the power supply of raspbery and m51.
						InPowerOffFlag=0;
					}
				}
				ShutDownTime=0;
				RaspberryONOFF=0;
			}
		}
	}
}

////////////////////////////////////////////////
#define LongPressTime 60      //600ms
uint8_t BtnPressIntFlag=0;    //标记是否有按键按下（方便按键按下计时使用）
uint8_t btnStatus[9]={0}; //记录所有按键状态，以备树莓派查询。0：弹起，1：按下；

void BtnPressTimeCounter()//放入timer0中断中计时     10ms调用一次。
{
	if(BtnPressIntFlag)//按键按下标记
	{
		if(btnStatus[0])
			(BtnTimer.btn1)++;
		else if(btnStatus[1])
			(BtnTimer.btn2)++;
		else if(btnStatus[2])
			(BtnTimer.btn3)++;
		else if(btnStatus[3])
			(BtnTimer.btn4)++;
		else if(btnStatus[4])
			(BtnTimer.btn5)++;
		else if(btnStatus[5])
			(BtnTimer.btn6)++;
		else if(btnStatus[6])
			(BtnTimer.btn7)++;
		else if(btnStatus[7])
			(BtnTimer.btn8)++;
		else if(btnStatus[8])
			(BtnTimer.btn9)++;
	}
}

/////////////////////////////////////////////////////////////////
void BtnLongPressHandler()   //对长按进行处理
{
	if(BtnPressIntFlag)
	{
		if(BtnTimer.btn1>LongPressTime)
		{
			NowBtn=0x91;
			BtnPressIntFlag=0;
			PB5=!PB5;
		}
		else if(BtnTimer.btn2>LongPressTime)
		{
			NowBtn=0x92;
			BtnPressIntFlag=0;
			PB5=!PB5;
		}
		else if(BtnTimer.btn3>LongPressTime)
		{
			NowBtn=0x93;
			BtnPressIntFlag=0;
			PB5=!PB5;
		}
		else if(BtnTimer.btn4>LongPressTime)
		{
			NowBtn=0x94;
			BtnPressIntFlag=0;
			PB5=!PB5;
		}
		else if(BtnTimer.btn5>LongPressTime)
		{
			NowBtn=0x95;
			BtnPressIntFlag=0;
			PB5=!PB5;
		}
		else if(BtnTimer.btn6>LongPressTime)
		{
			NowBtn=0x96;
			BtnPressIntFlag=0;
			PB5=!PB5;
		}
		else if(BtnTimer.btn7>LongPressTime)
		{
			NowBtn=0x97;
			BtnPressIntFlag=0;
			PB5=!PB5;
		}
		else if(BtnTimer.btn8>LongPressTime)
		{
			NowBtn=0x98;
			BtnPressIntFlag=0;
			PB5=!PB5;
		}
		else if(BtnTimer.btn9>LongPressTime)
		{
			NowBtn=0x99;
			BtnPressIntFlag=0;
			PB5=!PB5;
		}
	}
}

///////////////////////////////////////////////////////////////////////


void GPABIRQ2Flag(void)   //标记哪个按键触发中断，并翻转那个按键的状态。
{ 	
	volatile uint32_t temp;
	if(GPIO_GET_INT_FLAG(PB, BIT0)){//btn1
		GPIO_CLR_INT_FLAG(PB, BIT0);
		btnStatus[0]=!PB0;
		if(!PB0)
		{
			BtnPressIntFlag=1;
			NowBtn=0x81; 
			PB5=!PB5;
		}
		else
		{
			if(BtnTimer.btn1<LongPressTime)
			{						
				NowBtn=0x11;//短按，如果是长按则会在长按操作函数中处理;
				BtnPressIntFlag=0;
			}
			else 
				NowBtn=0x01;
			PB5=!PB5;			
		}
  }			
	else if(GPIO_GET_INT_FLAG(PB, BIT1)){//btn2
		GPIO_CLR_INT_FLAG(PB, BIT1);
		btnStatus[1]=!PB1;
		if(!PB1)
			{
				BtnPressIntFlag=1;
				NowBtn=0x82;
				PB5=!PB5;
			}
			else
			{
				if(BtnTimer.btn2<LongPressTime)
				{						
					NowBtn=0x12;//短按，如果是长按则会在长按操作函数中处理;
					BtnPressIntFlag=0;
				}
				else
					NowBtn=0x02;
				PB5=!PB5;
			}
  }
	else if(GPIO_GET_INT_FLAG(PA, BIT12)){//btn6
		GPIO_CLR_INT_FLAG(PA, BIT12);
		btnStatus[5]=!PA12;
			if(!PA12)
			{
				BtnPressIntFlag=1;
				NowBtn=0x86;
				PB5=!PB5;
			}
			else
			{
				if(BtnTimer.btn6<LongPressTime)
				{						
					NowBtn=0x16;//短按，如果是长按则会在长按操作函数中处理;
					BtnPressIntFlag=0;		
				}
				else 
					NowBtn=0x06;
				PB5=!PB5;
			}
	}
		
	else if(GPIO_GET_INT_FLAG(PA, BIT13)){//btn7
		GPIO_CLR_INT_FLAG(PA, BIT13);
		btnStatus[6]=!PA13;
		if(!PA13)
			{
				BtnPressIntFlag=1;
				NowBtn=0x87;
				PB5=!PB5;
			}
			else
			{
				if(BtnTimer.btn7<LongPressTime)
				{						
					NowBtn=0x17;//短按，如果是长按则会在长按操作函数中处理;
					BtnPressIntFlag=0;				
				}
				else
					NowBtn=0x07;
				PB5=!PB5;
			}
	}
	
	else if(GPIO_GET_INT_FLAG(PA, BIT14)){//btn8
		GPIO_CLR_INT_FLAG(PA, BIT14);
		btnStatus[7]=!PA14;
		if(!PA14)
			{
				BtnPressIntFlag=1;
				NowBtn=0x88;
				PB5=!PB5;
			}
			else
			{
				if(BtnTimer.btn8<LongPressTime)
				{						
					NowBtn=0x18;//短按，如果是长按则会在长按操作函数中处理;
					BtnPressIntFlag=0;			
				}
				else
					NowBtn=0x08;
				PB5=!PB5;
			}
	}	
	
  else if(GPIO_GET_INT_FLAG(PA, BIT15)){//btn9
		GPIO_CLR_INT_FLAG(PA, BIT15); 
		btnStatus[8]=!PA15;
			if(!PA15)
				{//开关机键按下	 
					BtnPressIntFlag=1;	
					NowBtn=0x89;
					PB5=!PB5;
					Btn9pressHandler();
				}
				else
				{	
					if(BtnTimer.btn9<LongPressTime)
					{						
						NowBtn=0x19;//短按，如果是长按则会在长按操作函数中处理;
						BtnPressIntFlag=0;		
					}
					else
						NowBtn=0x09;
					PB5=!PB5;
					Btn9releaseHandler();
				}
	}
	///////////////generate IRQ to raspberry//////////////////
	else if(GPIO_GET_INT_FLAG(PB, BIT4)){
		//printf("raspberry shutdown\n");
		GPIO_CLR_INT_FLAG(PB, BIT4);
		RaspberryONOFF=1;
	}
	else{
		/* Un-expected interrupt. Just clear all PB interrupts */
		temp = PA->INTSRC;
		PA->INTSRC = temp;		
		temp = PB->INTSRC;
		PB->INTSRC = temp;
	}
	BtnTimer.btn1=0;BtnTimer.btn2=0;BtnTimer.btn3=0;
	BtnTimer.btn4=0;BtnTimer.btn5=0;BtnTimer.btn6=0;
	BtnTimer.btn7=0;BtnTimer.btn8=0;BtnTimer.btn9=0;
}
void GPCDEFIRQ2Flag()
{	
	volatile uint32_t temp;
	if(GPIO_GET_INT_FLAG(PF, BIT3)){//btn3
		GPIO_CLR_INT_FLAG(PF, BIT3);
		btnStatus[2]=!PF3;
			if(!PF3)
			{
				BtnPressIntFlag=1;
				NowBtn=0x83;
				PB5=!PB5;
			}
			else
			{
				if(BtnTimer.btn3<LongPressTime)
				{						
					NowBtn=0x13;//短按，如果是长按则会在长按操作函数中处理;
					BtnPressIntFlag=0;					
				}
				else
					NowBtn=0x03;
				PB5=!PB5;
			}
	}
	else if(GPIO_GET_INT_FLAG(PF, BIT2)){//btn4
		GPIO_CLR_INT_FLAG(PF, BIT2);
		btnStatus[3]=!PF2;
			if(!PF2)
			{
				BtnPressIntFlag=1;
				NowBtn=0x84;
				PB5=!PB5;
			}
			else
			{
				if(BtnTimer.btn4<LongPressTime)
				{						
					NowBtn=0x14;//短按，如果是长按则会在长按操作函数中处理;
					BtnPressIntFlag=0;
				}
				else
					NowBtn=0x04;
				PB5=!PB5;
			}
	}
		
	else if(GPIO_GET_INT_FLAG(PF, BIT15)){//btn5
		GPIO_CLR_INT_FLAG(PF, BIT15);
		btnStatus[4]=!PF15;
		if(!PF15)
			{
				BtnPressIntFlag=1;
				NowBtn=0x85;
				PB5=!PB5;
			}
			else
			{
				if(BtnTimer.btn5<LongPressTime)
				{						
					NowBtn=0x15;//短按，如果是长按则会在长按操作函数中处理;
					BtnPressIntFlag=0;
				}
				else
					NowBtn=0x05;
				PB5=!PB5;
			}
	} 	
	else{
		temp = PF->INTSRC;
		PF->INTSRC = temp;
	}
	BtnTimer.btn1=0;BtnTimer.btn2=0;BtnTimer.btn3=0;
	BtnTimer.btn4=0;BtnTimer.btn5=0;BtnTimer.btn6=0;
	BtnTimer.btn7=0;BtnTimer.btn8=0;BtnTimer.btn9=0;
}


///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
//为了及时响应，将按键的短按处理放在GPIO中断
void GPAB_IRQHandler(void){
	GPABIRQ2Flag();
	
}
void GPCDEF_IRQHandler(void){
	GPCDEFIRQ2Flag();
}
