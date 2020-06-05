#include "I2C0Dev.h"

void I2C0_GPIO_Init(){
	CLK_EnableModuleClock(I2C0_MODULE);
	SYS->GPC_MFPL = (SYS->GPC_MFPL & ~(SYS_GPC_MFPL_PC0MFP_Msk | SYS_GPC_MFPL_PC1MFP_Msk)) |
                    (SYS_GPC_MFPL_PC0MFP_I2C0_SDA | SYS_GPC_MFPL_PC1MFP_I2C0_SCL);
};

void I2C0_Init(void)
{
    /* Open I2C module and set bus clock */
    I2C_Open(I2C0, 400000);
    /* Enable I2C interrupt */
    I2C_DisableInt(I2C0);
		//I2C_DisableTimeout(I2C0);
    NVIC_DisableIRQ(I2C0_IRQn);	
}

/************************************************************************
***将对i2c0的访问封装成以下三个函数，提供给九轴和电源芯片使用，**********
***使用i2c0InUseFlag对其加锁，防止访问冲突。****************************/

uint8_t i2c0InUseFlag=0;

uint8_t I2C_Write(uint8_t u8SlaveAddr, uint8_t u8DataAddr, uint8_t data)
{
	uint8_t i=0,reData;
	if(!i2c0InUseFlag)
	{
		i2c0InUseFlag=1;
		for(i=0;i<100;i++)
		{
			I2C_WriteByteOneReg(I2C0, u8SlaveAddr, u8DataAddr, data);
			reData=I2C_ReadByteOneReg(I2C0, u8SlaveAddr, u8DataAddr);
			if(reData==data)
			{
				i2c0InUseFlag=0;
				return 1; //success
			}
		}
		i2c0InUseFlag=0;
		return 0;      //unsuccess
	}
	return 0; //unsuccess
}

uint8_t I2C_ReadOneByte(uint8_t u8SlaveAddr, uint8_t u8DataAddr)
{
	uint8_t reData=0;
	if(!i2c0InUseFlag)
	{
		i2c0InUseFlag=1;
		reData=I2C_ReadByteOneReg(I2C0, u8SlaveAddr, u8DataAddr);
		i2c0InUseFlag=0;
		return reData;
	}
	else
		return 0;
}

uint32_t I2C_ReadMultiByte(uint8_t u8SlaveAddr, uint8_t u8DataAddr, uint8_t rdata[], uint32_t u32rLen)
{
	if(!i2c0InUseFlag)
	{
		i2c0InUseFlag=1;
		I2C_ReadMultiBytesOneReg(I2C0, u8SlaveAddr, u8DataAddr, rdata, u32rLen);
		i2c0InUseFlag=0;
		return 1;
	}
	return 0;
}
/*************************************************************************************/
