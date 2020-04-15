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
 static struct{
  bool btn1;
	bool btn2;
	bool btn3;
	bool btn4;
	bool btn5;
	bool btn6;
	bool btn7;
	bool btn8;
	bool btn9;
}ISButtonPressed={0,0,0,0,0,0,0,0,0};

 static struct{
  bool btn1;
	bool btn2;
	bool btn3;
	bool btn4;
	bool btn5;
	bool btn6;
	bool btn7;
	bool btn8;
	bool btn9;
	bool ONOFF;
}BtnINTFlag={0,0,0,0,0,0,0,0,0,0};


static struct {
	uint8_t btn1;
	uint8_t btn2;
	uint8_t btn3;
	uint8_t btn4;
	uint8_t btn5;
	uint8_t btn6;
	uint8_t btn7;
	uint8_t btn8;
	uint8_t btn9;
}BtnTimer={0,0,0,0,0,0,0,0,0};
	
extern uint8_t PowerState;
uint8_t PowerBtnPressTime;
uint8_t NowBtn=0xff;
uint8_t Btn9timerStart=0;
extern uint8_t LEDOnWork;

uint8_t InPowerStarting=0;
uint8_t ShutDownFlag;
uint8_t InPowerOffFlag=0;
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

extern uint8_t PowerOn();
extern uint8_t PowerOff();
extern int RGBBlinkTimes;
void PoweBtnLongPressHandler()
{
	if(!InPowerStarting)
	{
		if(Btn9timerStart)
		{
			PowerBtnPressTime++;
			if(PowerBtnPressTime>=5)
			{//长按关机
				InPowerOffFlag=1;
				Btn9timerStart=0;
				LEDChange(yellow);
				PowerOff();     
				InPowerOffFlag=0;
			}
			else if(PowerBtnPressTime>=2)
			{
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
					PowerOn();
					RGBBlinkTimes=8;
					//powerOnLight();
					LEDChange(dark);
					InPowerStarting=0;
				}		
			}			
		}
		if(ShutDownFlag)
		{
			ShutDownTime++;
			if(ShutDownTime>=7)
			{
				if(PB4)
				{
					if(!InPowerOffFlag)
					{
						InPowerOffFlag=1;
						PowerOff();       //Shut down the power supply of raspbery and m51.
						InPowerOffFlag=0;
					}
				}
				ShutDownTime=0;
				ShutDownFlag=0;
			}
		}
	}
}

////////////////////////////////////////////////
#define LongPressTime 60      //600ms
uint8_t BtnPressIntFlag=0;    //标记是否有按键按下（方便按键按下计时使用）
void BtnPressTimeCounter()//放入timer0中断中计时     10ms调用一次。
{
	if(BtnPressIntFlag)//按键按下标记
	{
		if(ISButtonPressed.btn1)
			(BtnTimer.btn1)++;
		else if(ISButtonPressed.btn2)
			(BtnTimer.btn2)++;
		else if(ISButtonPressed.btn3)
			(BtnTimer.btn3)++;
		else if(ISButtonPressed.btn4)
			(BtnTimer.btn4)++;
		else if(ISButtonPressed.btn5)
			(BtnTimer.btn5)++;
		else if(ISButtonPressed.btn6)
			(BtnTimer.btn6)++;
		else if(ISButtonPressed.btn7)
			(BtnTimer.btn7)++;
		else if(ISButtonPressed.btn8)
			(BtnTimer.btn8)++;
		else if(ISButtonPressed.btn9)
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
uint8_t btnStatus[9]={0}; //记录所有按键状态，以备树莓派查询。0：弹起，1：按下；
void GPABIRQ2Flag(void)   //标记哪个按键触发中断，并翻转那个按键的状态。
{ 	
	volatile uint32_t temp;
	if(GPIO_GET_INT_FLAG(PB, BIT0)){//btn1
		GPIO_CLR_INT_FLAG(PB, BIT0);
		BtnINTFlag.btn1=1;
		btnStatus[0]=!btnStatus[0];
  }			
	else if(GPIO_GET_INT_FLAG(PB, BIT1)){//btn2
		GPIO_CLR_INT_FLAG(PB, BIT1);
		BtnINTFlag.btn2=1;
		btnStatus[1]=!btnStatus[1];
  }
	else if(GPIO_GET_INT_FLAG(PA, BIT12)){//btn6
		GPIO_CLR_INT_FLAG(PA, BIT12);
		BtnINTFlag.btn6=1;
		btnStatus[5]=!btnStatus[5];
	}
		
	else if(GPIO_GET_INT_FLAG(PA, BIT13)){//btn7
		GPIO_CLR_INT_FLAG(PA, BIT13);
		BtnINTFlag.btn7=1;
		btnStatus[6]=!btnStatus[6];
	}
	
	else if(GPIO_GET_INT_FLAG(PA, BIT14)){//btn8
		GPIO_CLR_INT_FLAG(PA, BIT14);
		BtnINTFlag.btn8=1;
		btnStatus[7]=!btnStatus[7];
	}	
	
  else if(GPIO_GET_INT_FLAG(PA, BIT15)){
		GPIO_CLR_INT_FLAG(PA, BIT15); 
		BtnINTFlag.btn9=1;
		btnStatus[8]=!btnStatus[8];
	}
	///////////////generate IRQ to raspberry//////////////////
	else if(GPIO_GET_INT_FLAG(PB, BIT4)){
		//printf("raspberry shutdown\n");
		GPIO_CLR_INT_FLAG(PB, BIT4);
		BtnINTFlag.ONOFF=1;
	}
	else{
		/* Un-expected interrupt. Just clear all PB interrupts */
		temp = PA->INTSRC;
		PA->INTSRC = temp;
		
		temp = PB->INTSRC;
		PB->INTSRC = temp;
	}
}
void GPCDEFIRQ2Flag()
{	
	volatile uint32_t temp;
	if(GPIO_GET_INT_FLAG(PF, BIT3)){//btn3
		GPIO_CLR_INT_FLAG(PF, BIT3);
		BtnINTFlag.btn3=1;
		btnStatus[2]=!btnStatus[2];
	}
	else if(GPIO_GET_INT_FLAG(PF, BIT2)){//btn4
		GPIO_CLR_INT_FLAG(PF, BIT2);
		BtnINTFlag.btn4=1;
		btnStatus[3]=!btnStatus[3];
	}
		
	else if(GPIO_GET_INT_FLAG(PF, BIT15)){//btn5
		GPIO_CLR_INT_FLAG(PF, BIT15);
		BtnINTFlag.btn5=1;
		btnStatus[4]=!btnStatus[4];
	} 	
	else{
		temp = PF->INTSRC;
		PF->INTSRC = temp;
	}
}


///////////////////////////////////////////////////////////////////////////
uint8_t hasBtnINTFlag=0; //标记是否有按键中断
uint8_t ShutDownFlag=0;   //标记树莓派是否确认关机。
void Button_IRQFlagHandler(void)
{ 	
	if(hasBtnINTFlag)//按键中断标记
	{
		if(BtnINTFlag.btn1)//btn1
		{
			BtnINTFlag.btn1=0;
			if(!ISButtonPressed.btn1)
			{
				BtnTimer.btn1=0;
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
				BtnTimer.btn1=0;
			}
			ISButtonPressed.btn1=!ISButtonPressed.btn1;		
		}			
		else if(BtnINTFlag.btn2)//btn2
		{
			BtnINTFlag.btn2=0;
			if(!ISButtonPressed.btn2)
			{
				BtnTimer.btn2=0;
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
				BtnTimer.btn2=0;
			}
			ISButtonPressed.btn2=!ISButtonPressed.btn2;
		}
		
		else if(BtnINTFlag.btn3)//btn3
		{
			BtnINTFlag.btn3=0;
			if(!ISButtonPressed.btn3)
			{
				BtnTimer.btn3=0;
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
				BtnTimer.btn3=0;
			}
			ISButtonPressed.btn3=!ISButtonPressed.btn3;
		}
		
		else if(BtnINTFlag.btn4)//btn4
		{
			BtnINTFlag.btn4=0;
			if(!ISButtonPressed.btn4)
			{
				BtnTimer.btn4=0;
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
				BtnTimer.btn4=0;
			}
			ISButtonPressed.btn4=!ISButtonPressed.btn4;
		}
			
		else if(BtnINTFlag.btn5)///btn5
		{
			BtnINTFlag.btn5=0;
			if(!ISButtonPressed.btn5)
			{
				BtnTimer.btn5=0;
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
				BtnTimer.btn5=0;
			}
			ISButtonPressed.btn5=!ISButtonPressed.btn5;
		} 	
		
		else if(BtnINTFlag.btn6)//btn6
		{
			BtnINTFlag.btn6=0;
			if(!ISButtonPressed.btn6)
			{
				BtnTimer.btn6=0;
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
				BtnTimer.btn6=0;
			}
			ISButtonPressed.btn6=!ISButtonPressed.btn6;
		}
			
		else if(BtnINTFlag.btn7)//btn7
		{
			BtnINTFlag.btn7=0;
			if(!ISButtonPressed.btn7)
			{
				BtnTimer.btn7=0;
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
				BtnTimer.btn7=0;
			}
			ISButtonPressed.btn7=!ISButtonPressed.btn7;
		}
		
		else if(BtnINTFlag.btn8)//btn8
		{
			BtnINTFlag.btn8=0;
			if(!ISButtonPressed.btn8)
			{
				BtnTimer.btn8=0;
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
				BtnTimer.btn8=0;
			}
			ISButtonPressed.btn8=!ISButtonPressed.btn8;
		}	
		
		else if(BtnINTFlag.btn9)
		{
			BtnINTFlag.btn9=0;
			if(!ISButtonPressed.btn9)
				{//开关机键按下	
					BtnTimer.btn9=0;
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
					BtnTimer.btn9=0;
					Btn9releaseHandler();
				}
				ISButtonPressed.btn9=!ISButtonPressed.btn9;
		}
		///////////////generate IRQ to raspberry//////////////////
		if(BtnINTFlag.ONOFF){
			//printf("raspberry shutdown\n");
			BtnINTFlag.ONOFF=0;
			ShutDownFlag=1;
		}
		hasBtnINTFlag=0;
	}
}
///////////////////////////////////////////////////////////////////////////
//为了及时响应，将按键的短按处理放在GPIO中断
void GPAB_IRQHandler(void){
	GPABIRQ2Flag();
	if(!InPowerStarting) //当处于开关机过程时放弃处理按键事件
		hasBtnINTFlag=1;
	Button_IRQFlagHandler();
}
void GPCDEF_IRQHandler(void){
	GPCDEFIRQ2Flag();
	if(!InPowerStarting) ////当处于开关机过程时放弃处理按键事件
		hasBtnINTFlag=1;
	Button_IRQFlagHandler();
}
