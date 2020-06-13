// LSM6DSL Driver: 3-axis Gyroscope + 3-axis accelerometer + temperature

#include <stdio.h>
#include <stdint.h>
#include "NuMicro.h"
#include "LSM6DSL.h"
#include "I2C0Dev.h"
void LSM6DSL_WriteByte(uint8_t LSM6DSL_reg, uint8_t LSM6DSL_data)
{
 	I2C_Write(LSM6DSL_I2C_SLA ,  LSM6DSL_reg, LSM6DSL_data);
}

uint8_t LSM6DSL_ReadByte(uint8_t LSM6DSL_reg)
{
	  return I2C_ReadOneByte(LSM6DSL_I2C_SLA,LSM6DSL_reg);
}

void LSM6DSL_Read6Bytes(uint8_t LSM6DSL_reg,uint8_t* data)
{
	I2C_ReadMultiByte(LSM6DSL_I2C_SLA, LSM6DSL_reg,data, 6);
}

void Init_LSM6DSL(void)
{
	//printf("MPU init start******************\n");
	LSM6DSL_WriteByte(LSM6DSL_ACC_GYRO_CTRL1_XL, LSM6DSL_ACC_ODR_833_HZ);
	LSM6DSL_WriteByte(LSM6DSL_ACC_GYRO_CTRL2_G, LSM6DSL_GYRO_ODR_833_HZ);
	//printf("*************************************\n");
	LSM6DSL_WriteByte(LSM6DSL_ACC_GYRO_CTRL6_C, 1);  	// open gravity
	LSM6DSL_WriteByte(LSM6DSL_ACC_GYRO_CTRL7_G, 1);
}

////////////I2C1 Order Handler////////////////////////////
void I2C1readAcc(uint8_t* data){
	LSM6DSL_Read6Bytes(LSM6DSL_ACC_GYRO_OUTX_L_XL,data);    //LoByte is at first.
}

void I2C1readGyro(uint8_t* data){
	LSM6DSL_Read6Bytes(LSM6DSL_ACC_GYRO_OUTX_L_G,data);  //LoByte is at first.
}

extern void Init_BMM150(void);
extern void BMM150_ToSleepMOde(void);
extern void BMM150_ToNormalMode(void);

uint8_t AccOn=1,GyroOn=1,MagnOn=1;
uint8_t timerPriod=100;
uint8_t NineSensorOnOff=0;
void SensoODR_ONOFF_Handler(uint8_t u8data){
	if(u8data&0x01){
			LSM6DSL_WriteByte(LSM6DSL_ACC_GYRO_CTRL1_XL, LSM6DSL_ACC_ODR_833_HZ);
			AccOn= LSM6DSL_ReadByte(LSM6DSL_ACC_GYRO_CTRL1_XL);
			if((AccOn&0xF0)==LSM6DSL_GYRO_ODR_833_HZ)
				NineSensorOnOff|=0x01;//AccOn=1;
			else
				NineSensorOnOff&=0xFE;//AccOn=0;
	}
	else{
		LSM6DSL_WriteByte(LSM6DSL_ACC_GYRO_CTRL1_XL,LSM6DSL_ACC_ODR_POWER_DOWN );
		AccOn= LSM6DSL_ReadByte(LSM6DSL_ACC_GYRO_CTRL1_XL);
		if((AccOn&0xF0)==LSM6DSL_ACC_ODR_POWER_DOWN)
			NineSensorOnOff&=0xFE;//AccOn=0;
		else
			NineSensorOnOff|=0x01;//AccOn=1;
	}
	if(u8data&0x02){
		LSM6DSL_WriteByte(LSM6DSL_ACC_GYRO_CTRL2_G, LSM6DSL_GYRO_ODR_833_HZ);
		GyroOn= LSM6DSL_ReadByte(LSM6DSL_ACC_GYRO_CTRL2_G);
		if((GyroOn&0xF0)== LSM6DSL_GYRO_ODR_833_HZ)
			NineSensorOnOff|=0x02;//GyroOn=1;
		else
			NineSensorOnOff&=0xFD;//GyroOn=0;
	}
	else{	
		LSM6DSL_WriteByte(LSM6DSL_ACC_GYRO_CTRL2_G, LSM6DSL_GYRO_ODR_POWER_DOWN);
		GyroOn= LSM6DSL_ReadByte(LSM6DSL_ACC_GYRO_CTRL2_G);
		if((GyroOn&0xF0)== LSM6DSL_GYRO_ODR_POWER_DOWN)
			NineSensorOnOff&=0xFE;//GyroOn=0;
		else
			NineSensorOnOff|=0x02;//GyroOn=1;
	}
	if(u8data&0x04){
		BMM150_ToNormalMode();
		MagnOn=I2C_ReadOneByte(0x10,0x4C);
		if((MagnOn&0x38)== 0x38)
			NineSensorOnOff|=0x04;//MagnOn=1;
		else
			NineSensorOnOff&=0xF7;//MagnOn=0;
	}
	else{
		BMM150_ToSleepMOde();
		MagnOn=I2C_ReadOneByte(0x10,0x4C);
		if((MagnOn&0x38)== 0x00)
			NineSensorOnOff&=0xF7;//MagnOn=0;
		else
			NineSensorOnOff|=0x04;//MagnOn=1;
	}
}


uint8_t AccP=0,GyroP=0,MagnP=0;
uint8_t AccData[2][6]={0};
uint8_t GyroData[2][6]={0};
uint8_t MagnData[2][6]={0};

extern uint8_t time0Tick;//九轴缓存用，放弃。。。。
void TimePriod9SensorReadHandler()
{
	if(1)//(time0Tick%20)==0
	{
		if(AccOn)
			{
				I2C1readAcc(AccData[!AccP]);
				AccP=!AccP;
			}
			if(GyroOn)
			{
				//I2C1readGyro(GyroData[!GyroP]);
				GyroP=!GyroP;
			}			
			if(MagnOn)
			{
				//I2C1readMagn(MagnData[!MagnP]);
				MagnP=!MagnP;
			}
	}
}



//////////////////////////////////////////////////////////////////////////////
/*
int16_t Read_LSM6DSL_AccX(void)
{
	uint8_t LoByte, HiByte;
	LoByte = LSM6DSL_ReadByte(LSM6DSL_ACC_GYRO_OUTX_L_XL); // read Accelerometer X_Low  value
	HiByte = LSM6DSL_ReadByte(LSM6DSL_ACC_GYRO_OUTX_H_XL); // read Accelerometer X_High value
	return((HiByte<<8) | LoByte);
}

int16_t Read_LSM6DSL_ACC(void)
{
	uint8_t bytes[2];
	I2C_ReadMultiBytesOneReg(I2C0, LSM6DSL_I2C_SLA, LSM6DSL_ACC_GYRO_OUTX_L_XL, bytes, 2);
	return ((bytes[1]<<8)|bytes[0]);
}

int16_t Read_LSM6DSL_AccY(void)
{
	uint8_t LoByte, HiByte;
	LoByte = LSM6DSL_ReadByte(LSM6DSL_ACC_GYRO_OUTY_L_XL); // read Accelerometer X_Low  value
	HiByte = LSM6DSL_ReadByte(LSM6DSL_ACC_GYRO_OUTY_H_XL); // read Accelerometer X_High value
	return ((HiByte<<8) | LoByte);
}

int16_t Read_LSM6DSL_AccZ(void)
{
	uint8_t LoByte, HiByte;
	LoByte = LSM6DSL_ReadByte(LSM6DSL_ACC_GYRO_OUTZ_L_XL); // read Accelerometer X_Low  value
	HiByte = LSM6DSL_ReadByte(LSM6DSL_ACC_GYRO_OUTZ_H_XL); // read Accelerometer X_High value
	return ((HiByte<<8) | LoByte);
}

int16_t Read_LSM6DSL_GyroX(void)
{
	uint8_t LoByte, HiByte;
	LoByte = LSM6DSL_ReadByte(LSM6DSL_ACC_GYRO_OUTX_L_G); // read Accelerometer X_Low  value
	HiByte = LSM6DSL_ReadByte(LSM6DSL_ACC_GYRO_OUTX_H_G); // read Accelerometer X_High value
	return ((HiByte<<8) | LoByte);
}

int16_t Read_LSM6DSL_GyroY(void)
{
	uint8_t LoByte, HiByte;
	LoByte = LSM6DSL_ReadByte(LSM6DSL_ACC_GYRO_OUTY_L_G); // read Accelerometer X_Low  value
	HiByte = LSM6DSL_ReadByte(LSM6DSL_ACC_GYRO_OUTY_H_G); // read Accelerometer X_High value
	return ((HiByte<<8) | LoByte);
}

int16_t Read_LSM6DSL_GyroZ(void)
{
	uint8_t LoByte, HiByte;
	LoByte = LSM6DSL_ReadByte(LSM6DSL_ACC_GYRO_OUTZ_L_G); // read Accelerometer X_Low  value
	HiByte = LSM6DSL_ReadByte(LSM6DSL_ACC_GYRO_OUTZ_H_G); // read Accelerometer X_High value
	return ((HiByte<<8) | LoByte);
}

uint16_t Read_LSM6DSL_ID(void)
{
	return (LSM6DSL_ReadByte(LSM6DSL_ACC_GYRO_WHO_AM_I_REG));
}
*/////////////////////////////////////////////////////////////////////////////////////////////////////
