#include <stdio.h>
#include <stdint.h>
#include "NuMicro.h"
#include "BMM150.h"
#include <I2C0Dev.h>
/**************************************************************************
*地磁传感器具有三种能耗模式：
*1、挂起模式:上电复位后的默认模式，只能访问功率控制位（0x4B）信息.
*2、睡眠模式:power位（0x4B bit0）设置为1将从挂起模式转为睡眠模式，将power位设置为0可从睡眠模式重新转为挂起模式。
*3、活动模式：将opMode（0x4C bit2 bit1）设置为11可从睡眠模式切换到活动模式，此模式下可执行磁场测量。
*****************************************************************************************************************/

void BMM150_WriteByte(uint8_t LSM6DSL_reg, uint8_t BMM_data)
{
		
	I2C_Write(0x10 ,  LSM6DSL_reg, BMM_data);
}

uint8_t BMM_ReadByte(uint8_t BMM_reg)
{
	 return I2C_ReadOneByte(0x10,BMM_reg);
}

void BMM150_Read6Bytes(uint8_t BMM_reg,uint8_t* data)
{
	I2C_ReadMultiByte(0x10, BMM_reg,data, 6);
}

#define BMM150_ODR_OPMODE_ADDR 0x4C
#define BMM150_ODR_mask     0x38
#define BMM150_ODR_30HZ 0x38
#define BMM150_OPMODE_mask  0x06
#define BMM150_OPMODE_ToNormalMode  0x00
#define BMM150_OPMODE_ToForceMode   0x02
#define BMM150_OPMODE_ToSleepMode   0x06

#define BMM150_RESET_POWERMODE_ADDR 0x4B
#define BMM150_POWERMODE_mask 0x01
#define BMM150_POWERMODE_ToSuspend 0x00
#define BMM150_POWERMODE_ ToWork   0x01

void Init_BMM150()
{
	//printf("BMM150 init...............\n");
	BMM150_WriteByte(0x4B,0x01); //上电后，将power位设置为1，由挂起模式进入睡眠模式
	BMM150_WriteByte(0x4C,0x38);	//将地磁数据输出速率（[6:4]）设置为111b(最高的30HZ),将opMode([2:1])设置为00b,由睡眠模式进入活动模式；
	BMM150_WriteByte(0x4D,0x80);  //允许数据溢出；
	BMM150_WriteByte(0x4E,0x00);  //使能xyz轴，禁用中断;
}
void BMM150_ToSleepMOde()
{
	BMM150_WriteByte(0x4C,0x3E);	//将opMode位设置为11b，由活动模式进入睡眠模式。
}

void BMM150_ToNormalMode()
{
	BMM150_WriteByte(0x4B,0x01);
	BMM150_WriteByte(0x4C,0x38);	//由睡眠模式回到活动模式
}

uint8_t BMM_whoami()
{
	uint8_t ID=0;
	ID=BMM_ReadByte(BMM050_CHIP_ID);
	//printf("BMM's ID is:%x\n",ID);
	return ID;
}

/////////////////////I2C read orderhandler/////////////////////////////
void I2C1readMagn(uint8_t* data){
	uint16_t temp=0;
	BMM150_Read6Bytes(0x42,data);  //LoByte is at first.
	temp=data[1]<<8|data[0];
	temp=temp>>3;
	data[1]=temp>>8;
	data[0]=temp;//X axis
	
	temp=0;
	temp=data[3]<<8|data[2];
	temp=temp>>3;
	data[3]=temp>>8;
	data[2]=temp;//Y axis
	
	temp=0;
	temp=data[5]<<8|data[4];
	temp=temp>>1;
	data[5]=temp>>8;
	data[4]=temp;//Z axis
}
