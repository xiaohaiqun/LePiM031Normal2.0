#include "IP5328.h"
#include <i2c.h>
#include <stdio.h>
#include <pwm_light.h>
#include "I2C0Dev.h"
uint8_t PowerState=1;
extern uint8_t i2c0InUseFlag;
/////////////ip5328  i2c 读写函数封装////////////////////////////////
uint8_t IP5328_WriteByte(uint8_t IP5328_reg, uint8_t IP5328_data)
{
	return I2C_WriteByteOneReg(I2C0,ip5328_slave_adress, IP5328_reg, IP5328_data);
}

uint8_t IP5328_ReadByte(uint8_t IP5328_reg)
{
	 return I2C_ReadByteOneReg(I2C0,ip5328_slave_adress,IP5328_reg);
}

uint8_t IP5328_ReadMutiByte(uint8_t IP5328_reg,uint8_t* data,uint8_t len)
{
	return I2C_ReadMultiByte(ip5328_slave_adress,IP5328_reg,data,len);	
}
////////////////////////////////////////////////////////////////////////////////////////

//////////////////////just used for m031 test/////////////////////////////
uint8_t PowerStateSetOn()
{
	if((IP5328_ReadByte(0x59)&0x04)==0x04)
	{
		PowerState=1;
		return 1;
	}
	return 0;
}
uint8_t PowerStateSetOff()
{
	if((IP5328_ReadByte(0x59)&0x04)==0)
	{
		PowerState=0;
		return 1;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

////////////////////////vout1 and vout2 ctl///////////////////////////////////
#define vout12ctl 0x59
void OpenVout1()
{
	uint8_t state=IP5328_ReadByte(0x0E);
	IP5328_WriteByte(0x0E, state|0x04);
	state=IP5328_ReadByte(vout12ctl);
	IP5328_WriteByte(vout12ctl, (state|0x0C));
}
void CloseVout1()
{
	uint8_t state=IP5328_ReadByte(0x0E);
	IP5328_WriteByte(0x0E, state|0x04);
	state=IP5328_ReadByte(vout12ctl);
	IP5328_WriteByte(vout12ctl, (state&0xFB));
}
void OpenVout2(void)
{
	uint8_t state=IP5328_ReadByte(vout12ctl);
	IP5328_WriteByte(vout12ctl, (state|0x30));
}
void CloseVout2(void)
{
	uint8_t state=IP5328_ReadByte(vout12ctl);
	IP5328_WriteByte(vout12ctl, (state&0xDF));
}
uint8_t powerOnOffFlag=0;

uint8_t PowerOn(void){
	uint8_t m=0;
	powerOnOffFlag=1;
	PB13=1;
	TIMER_Delay(TIMER1, 100000);
	PB13=0;
	TIMER_Delay(TIMER1, 100000);
	for(m=0;m<2;m++)
	{
		if(PB12)
		{
			OpenVout1();
			OpenVout2();
			I2C_SetBusClockFreq(I2C0,400000);
			PowerStateSetOn();
			return 1;
		}
		else
			{//模拟1s内短按两次关机。
			PB13=1;
			TIMER_Delay(TIMER1, 100000);
			PB13=0;
			
			TIMER_Delay(TIMER1, 60000);
			
			PB13=1;
			TIMER_Delay(TIMER1, 100000);
			PB13=0;
		}
	}
	powerOnOffFlag=0;
	return 0;
}

uint8_t PowerOff(){
	 powerOnOffFlag=1;
	 CloseVout1();
	 CloseVout2();
	 I2C_SetBusClockFreq(I2C0,9000);
	 //TIMER_Stop(TIMER1);
	 if(PowerStateSetOff())
	 {
		 powerOnOffFlag=0;
		 return 1;
	 }
	 else
	 {
		 powerOnOffFlag=0;
		 return 0;
	 }
}
////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
uint8_t BATpowerNum;
void IP5328Init(){

	uint8_t tempdata=0;
	uint16_t BATpower_Temp=0;
	//Boost升压使能，charge充电使能
	tempdata=IP5328_ReadByte(0x01);
	IP5328_WriteByte(0x01, tempdata|0x06);	
	//开机寄存器复位使能
	//tempdata=IP5328_ReadByte(0x03);
	//IP5328_WriteByte(0x03, tempdata|0x80);
	//disable 芯片温度控制
	tempdata=IP5328_ReadByte(0x04);
	IP5328_WriteByte(0x04, 0x00);
	//LED模式寄存器使能（i2c模式下可以用过0xDB查看电量计算结果）
	tempdata=IP5328_ReadByte(0x0A);
	IP5328_WriteByte(0x0A, tempdata|0xE0);
	
	//使能同充同放
	tempdata=IP5328_ReadByte(0x0D);
	IP5328_WriteByte(0x0E, tempdata|0x01);
	
	//使能在待机时可以通过I2C访问电源芯片
	tempdata=IP5328_ReadByte(0x0E);
	IP5328_WriteByte(0x0E, tempdata|0x04);
	
	//BAT实际电压低电关机电压设定  3.0~3.1V
	tempdata=IP5328_ReadByte(0x10);
	IP5328_WriteByte(0x10, tempdata|0x30);
	
	//轻载关机阈值设定，调成0，希望轻载不关机
	tempdata=IP5328_ReadByte(0x81);
	IP5328_WriteByte(0x81, tempdata&0x00);
	tempdata=IP5328_ReadByte(0x84);
	IP5328_WriteByte(0x84, tempdata&0x00);
	//测试9v
	PowerOn();
	
	while((BATpowerNum)==0||(BATpowerNum>100))
	{
		BATpower_Temp=(IP5328_ReadByte(0x7B));          //电池开路电压,计算电量 
		BATpower_Temp=BATpower_Temp<<8 | (IP5328_ReadByte(0x7A));
		BATpowerNum=(BATpower_Temp-2000)/40.0;
	}
	
}
//////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

#define BATVADC_DAT_L 0x64
#define BATVADC_DAT_H 0x65         //VBAT=BATVADC*0.26855mv+2.6V BATPIN 的真实电压 
#define BATIADC_DAT_L 0x66
#define BATIADC_DAT_H 0x67         //BATIADC 数据的高 8bit IBAT=BATVADC*1.27883mA 

#define VOUT1IADC_DAT_L 0x70
#define VOUT1IADC_DAT_H 0x71       //VOUT1I= VOUT1IADC *LSB 
#define VOUT2IADC_DAT_L 0x72
#define VOUT2IADC_DAT_H 0x73       //VOUT2I= VOUT2IADC *LSB 

#define BATOCV_DAT_L  0x7A
#define BATOCV_DAT_H  0x7B          //OCV=BATOCV*0.26855mv+2.6V 电池的开路电压，用来估算电量
extern uint8_t NowBtn;
uint16_t BATpower=0;
uint8_t ChargeInfo=0;

extern uint8_t data[6];
void I2C1PowerSpy()
{
	uint8_t ChargeInfo_Temp=0;
	uint16_t BATpower_Temp=0;
	uint8_t temp=0;
	if(!powerOnOffFlag)
	{
		BATpower_Temp=(IP5328_ReadByte(0x7B));          //电池开路电压,计算电量 
		BATpower_Temp=BATpower_Temp<<8 | (IP5328_ReadByte(0x7A));
		ChargeInfo_Temp=IP5328_ReadByte(0xD7);               //充电状态	
		//if((BATpower!=BATpower_Temp)||(ChargeInfo!=ChargeInfo_Temp))
		{
			BATpower=BATpower_Temp;
			temp=(BATpower_Temp-2800)/32.0;
			//if((BATpowerNum-temp)*(BATpowerNum-temp)<100)
				BATpowerNum=temp;
			NowBtn=0x55;                                        //电池电量变化
			
			ChargeInfo=ChargeInfo_Temp;
			switch(ChargeInfo&0x03)
			{
				case 0x00:  //未在充电
					NowBtn=0x50;                             //电池
					break;
				case 0x01:  //涓流充电
				case 0x02:  //恒流充电
				case 0x03:  //恒压充电
					NowBtn=0x51;
					break;
				case 0x05:  //电池充满结束
					NowBtn=0x52;
					break;
				default:
					break;
			}
			PB5=!PB5;
		}
	}
}
extern void RGBConfig(uint8_t r,uint8_t g,uint8_t b);
extern uint8_t LEDOnWork;
uint8_t InChargeFlag=0;
uint8_t chargeLed=0;
uint8_t lowPowerLed=0;
uint16_t lowPowerShutDownTimer=0;
void ChargeAndLowPowerLedDisplay(void)
{
	if(IP5328_ReadByte(0xD7)&0xF0)//在充电
	{
		lowPowerShutDownTimer=0;
		InChargeFlag=1;
		if(chargeLed==1)
			RGBConfig(0,0,0);
		else if(chargeLed==2)
			RGBConfig(0,33,0);
		else if(chargeLed==3)
			RGBConfig(0,66,0);
		else if(chargeLed==4)
		{
			RGBConfig(0,100,0);
			chargeLed=0;
		}
		chargeLed++;		
	}
	else //不在充电
	{
		if(InChargeFlag)
		{
			InChargeFlag=0;
			if(!LEDOnWork)
				RGBConfig(0,0,0);//不在充电就及时关闭充电指示灯。
		}
		if((BATpowerNum<=5) && (PowerState==1))//低电量(低于百分之五)处理
		{
			lowPowerShutDownTimer++;
			if(lowPowerShutDownTimer<30)//低电提示30秒
			{
				if(lowPowerLed)
				{
					lowPowerLed=0;
					RGBConfig(0,0,0);
				}
				else
				{
					lowPowerLed=1;
					RGBConfig(60,0,0);
				}
			}
			else            //提示三十秒后强制关机
			{
				PowerOff();
				lowPowerShutDownTimer=0;
				RGBConfig(0,0,0);
			}
		}
		else
		{
			lowPowerShutDownTimer=0;
		}
	}
}
uint8_t lowPowerDetect()
{
	return BATpowerNum<=10;
}

void I2C1readPower(uint8_t* data)              //读取电池电量估计以及充电状态信息，两个字节
{
	uint16_t BATpower_Temp;
	BATpower_Temp=(IP5328_ReadByte(0x7B));       //电池开路电压,计算电量 
	BATpower_Temp=BATpower_Temp<<8 | (IP5328_ReadByte(0x7A));
	BATpowerNum=(BATpower_Temp-2800)/32.0;
	data[0]=BATpowerNum;                         //灯显模式计算的电量
	data[1]=IP5328_ReadByte(0xD7);               //充电状态	
	IP5328_ReadMutiByte(BATOCV_DAT_L,data+2,2);  //开路电压读取，用于进一步估算电池电量
}

void I2C1readVout1_2_A(uint8_t* data)   //读取Vout1和vout2的输出电流，4个字节
{
	IP5328_ReadMutiByte(VOUT1IADC_DAT_L,data,4);
}

void I2C1readBAT_V_I(uint8_t* data)     //读取电池的电压和电流，4个字节
{
	IP5328_ReadMutiByte(BATVADC_DAT_L,data,4);
}

//////////////////////////////////
///////////I2C Order handler//////
void PowerHandler(uint8_t u8data){
	switch(u8data){
		case 0x51:
			OpenVout2();
			break;
		case 0x50:
			CloseVout2();
			break;
		default:
			break;
	}
}

////////////////////////////////////////////////
