#include <stdio.h>
#include <stdint.h>
#include "NuMicro.h"
#include "BMM150.h"

void BMM150_WriteByte(uint8_t LSM6DSL_reg, uint8_t BMM_data)
{
	uint8_t temp=0,n=0;
	while(1){
		I2C_WriteByteOneReg(I2C0,0x10 ,  LSM6DSL_reg, BMM_data);
		temp=I2C_ReadByteOneReg(I2C0,0x10,LSM6DSL_reg);
		n++;
		if(temp==BMM_data|n>100)
			break;
	}
}

uint8_t BMM_ReadByte(uint8_t BMM_reg)
{
	 return I2C_ReadByteOneReg(I2C0,0x10,BMM_reg);
}

void BMM150_Read6Bytes(uint8_t BMM_reg,uint8_t* data)
{
	I2C_ReadMultiBytesOneReg(I2C0,0x10, BMM_reg,data, 6);
}

#define BMM150_ODR-OPMODE_ADDR 0x4C
#define BMM150_ODR_mask     0x38
#define BMM150_ODR_30HZ 0x38
#define BMM150_OPMODE_mask  0x06
#define BMM150_OPMODE_ToNormalMode  0x00
#define BMM150_OPMODE_ToForceMode   0x02
#define BMM150_OPMODE_ToSleepMode   0x06

#define BMM150_RESET-POWERMODE_ADDR 0x4B
#define BMM150_POWERMODE_mask 0x01
#define BMM150_POWERMODE_ToSuspend 0x00
#define BMM150_POWERMODE_ ToWork   0x01

void Init_BMM150()
{
	//printf("BMM150 init...............\n");
	I2C_WriteByteOneReg(I2C0,0x10,0x4B,0x01); //上电，进入工作模式。
	I2C_WriteByteOneReg(I2C0,0x10,0x4C,0x38);	//30HZ,正常模式 	
}
void BMM150_ToSleepMOde()
{
	I2C_WriteByteOneReg(I2C0,0x10,0x4C,0x3E);	//
}

void BMM150_ToNormalMode()
{
	I2C_WriteByteOneReg(I2C0,0x10,0x4C,0x38);	//
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