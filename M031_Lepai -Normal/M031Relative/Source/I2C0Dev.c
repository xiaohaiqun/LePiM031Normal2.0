#include "I2C0Dev.h"
#include "pwm_light.h"
//#include <stdio.h>
void I2C0_GPIO_Init(){
	CLK_EnableModuleClock(I2C0_MODULE);
	SYS->GPC_MFPL = (SYS->GPC_MFPL & ~(SYS_GPC_MFPL_PC0MFP_Msk | SYS_GPC_MFPL_PC1MFP_Msk)) |
                    (SYS_GPC_MFPL_PC0MFP_I2C0_SDA | SYS_GPC_MFPL_PC1MFP_I2C0_SCL);
};
#define MASTER_I2C I2C0

void I2C0_Close(void)
{
    /* Disable I2C0 interrupt and clear corresponding NVIC bit */
    I2C_DisableInt(I2C0);
    NVIC_DisableIRQ(I2C0_IRQn);
    /* Disable I2C0 and close I2C0 clock */
    I2C_Close(I2C0);
    CLK_DisableModuleClock(I2C0_MODULE);
}

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/

static uint8_t I2C0MasterRxTxEndFlag = 0;

static uint8_t SlaveAddr;
static uint8_t RegAddr;
static uint8_t Rxdata;
static uint8_t I2C0Err;

static uint8_t I2C0RxRawLen;
static uint8_t* I2C0RxMultiDatas;
static uint8_t I2C0RxMultiLen = 0U;
uint32_t u32txLen = 0U;
uint8_t	I2C0tLen=0;
static int8_t I2C0Txdata;
/*---------------------------------------------------------------------------------------------------------*/
/*  I2C Rx Callback Function                                                                               */
/*---------------------------------------------------------------------------------------------------------*/
void I2C_MasterRxOne(uint32_t u32Status)
{
	uint8_t  u8Ctrl = 0U;
	I2C0Err=0;
	switch (u32Status)
	{
		case 0x08:				
				I2C_SET_DATA(I2C0, (uint8_t)(SlaveAddr << 1U | 0x00U));    /* Write SLA+W to Register I2CDAT */
				u8Ctrl = I2C_CTL_SI;                             /* Clear SI */
				//printf("1 ");
				break;
		case 0x18:                                           /* Slave Address ACK */				
				I2C_SET_DATA(I2C0, RegAddr);                   /* Write Lo byte address of register */
				u8Ctrl = I2C_CTL_SI;
				//printf("2 ");
				break;
		case 0x20:                                           /* Slave Address NACK */
		case 0x30:                                           /* Master transmit data NACK */				
				u8Ctrl = I2C_CTL_STO_SI;                         /* Clear SI and send STOP */
				I2C0Err = 1U;
				//printf("2- ");
				break;
		case 0x28:				
				u8Ctrl = I2C_CTL_STA_SI;                         /* Send repeat START */
				//printf("3 ");
				break;
		case 0x10:		
				I2C_SET_DATA(I2C0, (uint8_t)((SlaveAddr << 1U) | 0x01U));  /* Write SLA+R to Register I2CDAT */
				u8Ctrl = I2C_CTL_SI;                             /* Clear SI */
				//printf("4 ");
				break;
		case 0x40:                                           /* Slave Address ACK */
				u8Ctrl = I2C_CTL_SI;                             /* Clear SI */
				//printf("5 ");
				break;
		case 0x48:                                           /* Slave Address NACK */				
				u8Ctrl = I2C_CTL_STO_SI;                         /* Clear SI and send STOP */
				I2C0Err = 1U;
				//printf("5- ");
				break;
		case 0x58:
				Rxdata = (uint8_t) I2C_GET_DATA(I2C0);             /* Receive Data */
				u8Ctrl = I2C_CTL_STO_SI;                         /* Clear SI and send STOP */
				I2C0MasterRxTxEndFlag = 1 ;		
				//printf("6\n");
				break;
		case 0x38:                                           /* Arbitration Lost */
		default:                                             /* Unknow status */				
				I2C_SET_CONTROL_REG(I2C0, I2C_CTL_STO_SI);        /* Clear SI and send STOP */
				u8Ctrl = I2C_CTL_SI;
				I2C0Err = 1U;
				//printf("error\n");
				break;
	}
	I2C_SET_CONTROL_REG(I2C0, u8Ctrl);                    /* Write controlbit to I2C_CTL register */
}

void I2C_MasterRxMulti(uint32_t u32Status)
{
	uint8_t u8Ctrl = 0U;	
	I2C0Err = 0U;
	switch (u32Status)
	{
		case 0x08:
				I2C_SET_DATA(I2C0, (uint8_t)(SlaveAddr << 1U | 0x00U));    /* Write SLA+W to Register I2CDAT */
				u8Ctrl = I2C_CTL_SI;                             /* Clear SI */
				//printf("1 ");
				break;
		case 0x18:                                           /* Slave Address ACK */
				I2C_SET_DATA(I2C0, RegAddr);                   /* Write Lo byte address of register */
				u8Ctrl = I2C_CTL_SI;
				//printf("2 ");
				break;
		case 0x20:                                           /* Slave Address NACK */
		case 0x30:                                           /* Master transmit data NACK */
				u8Ctrl = I2C_CTL_STO_SI;                         /* Clear SI and send STOP */
				I2C0Err = 1U;
				//printf("2- ");
				break;
		case 0x28:
				u8Ctrl = I2C_CTL_STA_SI;                         /* Send repeat START */
				//printf("3 ");
				break;
		case 0x10:
				I2C_SET_DATA(I2C0, (uint8_t)((SlaveAddr << 1U) | 0x01U));  /* Write SLA+R to Register I2CDAT */
				u8Ctrl = I2C_CTL_SI;                             /* Clear SI */
				//printf("4 ");
				break;
		case 0x40:                                           /* Slave Address ACK */
				u8Ctrl = I2C_CTL_SI_AA;                          /* Clear SI and set ACK */
				//printf("5 ");
				break;
		case 0x48:                                           /* Slave Address NACK */
				u8Ctrl = I2C_CTL_STO_SI;                         /* Clear SI and send STOP */
				I2C0Err = 1U;
				//printf("5- ");
				break;
		case 0x50:
				I2C0RxMultiDatas[I2C0RxMultiLen++] = (uint8_t) I2C_GET_DATA(I2C0); /* Receive Data */
				if (I2C0RxMultiLen < (I2C0RxRawLen - 1U))
						u8Ctrl = I2C_CTL_SI_AA;                      /* Clear SI and set ACK */
				else
						u8Ctrl = I2C_CTL_SI;                         /* Clear SI */
				//printf("6 ");
				break;
		case 0x58:
				I2C0RxMultiDatas[I2C0RxMultiLen++] = (uint8_t) I2C_GET_DATA(I2C0); /* Receive Data */
				u8Ctrl = I2C_CTL_STO_SI;                         /* Clear SI and send STOP */
				I2C0MasterRxTxEndFlag = 1U;
				//printf("7\n");
				break;
		case 0x38:                                           /* Arbitration Lost */
		default:                                             /* Unknow status */
				I2C_SET_CONTROL_REG(I2C0, I2C_CTL_STO_SI);        /* Clear SI and send STOP */
				u8Ctrl = I2C_CTL_SI;
				I2C0Err = 1U;
				//printf("error\n");
				break;
	}
	I2C_SET_CONTROL_REG(I2C0, u8Ctrl);                    /* Write controlbit to I2C_CTL register */
}

/*---------------------------------------------------------------------------------------------------------*/
/*  I2C Tx Callback Function                                                                               */
/*---------------------------------------------------------------------------------------------------------*/

void I2C_MasterTx(uint32_t u32Status)
{
	uint8_t u8Ctrl = 0U;
	I2C0Err = 0U;
	switch (u32Status)
	{
		case 0x08:
				I2C_SET_DATA(I2C0, (uint8_t)(SlaveAddr << 1U | 0x00U));  /* Send Slave address with write bit */
				u8Ctrl = I2C_CTL_SI;                           /* Clear SI */
				//printf("1 ");
				break;
		case 0x18:                                         /* Slave Address ACK */
				I2C_SET_DATA(I2C0, RegAddr);                 /* Write Lo byte address of register */
				u8Ctrl = I2C_CTL_SI;
				//printf("2 ");
				break;
		case 0x20:                                         /* Slave Address NACK */
		case 0x30:                                         /* Master transmit data NACK */
				u8Ctrl = I2C_CTL_STO_SI;                       /* Clear SI and send STOP */
				I2C0Err = 1U;
				//printf("2- ");
				break;
		case 0x28:
				if (u32txLen < 1U)
				{
						I2C_SET_DATA(I2C0, I2C0Txdata);
						u8Ctrl = I2C_CTL_SI;
						u32txLen++;
						//printf("3 ");
				}
				else
				{
						u8Ctrl = I2C_CTL_STO_SI;                   /* Clear SI and send STOP */
						I2C0MasterRxTxEndFlag = 1U;	
						//printf("4 \n");
				}
				break;
		case 0x00:                                         /*bus error*/				
		case 0x38:                                         /* Arbitration Lost */
		default:                                           /* Unknow status */
				I2C_SET_CONTROL_REG(I2C0, I2C_CTL_STO_SI);      /* Clear SI and send STOP */
				u8Ctrl = I2C_CTL_SI;
				I2C0Err = 1U;
				//printf("error\n");
				break;
	}
	I2C_SET_CONTROL_REG(I2C0, u8Ctrl);                  /* Write controlbit to I2C_CTL register */
}
/*---------------------------------------------------------------------------------------------------------*/
/*  I2C0 IRQ Handler                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
uint8_t funMode;
void I2C0_IRQHandler(void)
{
    uint8_t u8Status;
    //printf(" IN \n");
		u8Status = I2C_GET_STATUS(I2C0);
    if(I2C_GET_TIMEOUT_FLAG(I2C0))
    {
        /* Clear I2C0 Timeout Flag */
        I2C_ClearTimeoutFlag(I2C0);
    }
    else
    {
			switch(funMode)
			{
				case 0x01:		
					//printf(" ++\n");
					I2C_MasterTx(u8Status);
					//printf(" --\n");
					break;
				case 0x02:
					I2C_MasterRxOne(u8Status);
					break;
				case 0x03:
					I2C_MasterRxMulti(u8Status);
					break;
				default:
					//I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI);
					//error
					break;
			}
    }
		//printf(" OUT \n");
}





#define USE_I2C0_INT
#ifdef USE_I2C0_INT

void I2C0_Init(void)
{
    /* Open I2C module and set bus clock */
    I2C_Open(I2C0, 400000);
    //I2C_SetSlaveAddr(MASTER_I2C, 1, 0x35, 0);   /* Slave Address : 0x35 */
    /*Enable I2C interrupt */
    I2C_EnableInt(I2C0);
    NVIC_EnableIRQ(I2C0_IRQn);
		I2C_SET_CONTROL_REG(I2C0,I2C_CTL_SI ); 
		//NVIC_SetPriority(I2C0_IRQn,0);
		I2C_DisableTimeout(I2C0);
}

#define reTryTimes 20
uint8_t I2C_Write(uint8_t u8SlaveAddr, uint8_t u8DataAddr, uint8_t data)
{		
	uint8_t reTry=0,reReadTry=0,reReadData=0;	
	SlaveAddr=u8SlaveAddr;
	RegAddr=u8DataAddr;
	I2C0Txdata=data;
	
	funMode=0x01;
	do{
		do{
				I2C0MasterRxTxEndFlag = 0;
				I2C0Err=0;
				u32txLen=0;	  //已发送数据字节数：
				I2C_SET_CONTROL_REG(I2C0, I2C_CTL_STA);
				
				while( 1 )
				{
					if(I2C0MasterRxTxEndFlag | I2C0Err)
						break;
				}
				reTry++;
			}while(I2C0Err&&reTry<reTryTimes);
		reReadData=I2C_ReadOneByte(u8SlaveAddr, u8DataAddr);
		reReadTry++;
	}while((reReadData!=data)&&(reReadTry<reTryTimes));
		
	I2C0MasterRxTxEndFlag = 0;
	return I2C0Err;
}


uint8_t I2C_ReadOneByte(uint8_t u8SlaveAddr, uint8_t u8DataAddr)
{
	uint8_t reTry=0;
	SlaveAddr=u8SlaveAddr;
	RegAddr=u8DataAddr;
	
	funMode=0x02;
	do{
		I2C0Err=0;
		I2C0MasterRxTxEndFlag = 0;
		I2C_SET_CONTROL_REG(I2C0, I2C_CTL_STA);
		
		while( 1 )
		{
			if(I2C0MasterRxTxEndFlag | I2C0Err)
				break;
		}
		
		reTry++;
		
	}while(I2C0Err&&reTry<reTryTimes);
	return Rxdata;
}
uint32_t count=0;
uint32_t I2C_ReadMultiByte(uint8_t u8SlaveAddr, uint8_t u8DataAddr, uint8_t rdata[], uint32_t u32rLen)
{
	uint8_t reTry=0;
	SlaveAddr=u8SlaveAddr;
	RegAddr=u8DataAddr;
	I2C0RxMultiDatas = rdata;
	I2C0RxRawLen=u32rLen;
	funMode=0x03;
	do{
		I2C0MasterRxTxEndFlag = 0;
		I2C0Err=0;
		I2C0RxMultiLen=0;
		
		I2C_SET_CONTROL_REG(I2C0, I2C_CTL_STA);
		
		while( 1 )
		{
			if(I2C0MasterRxTxEndFlag | I2C0Err)
				break;
		}
		
		reTry++;
	}while(I2C0Err&&reTry<reTryTimes);
	return I2C0Err;
}

#else
void I2C0_Init(void)
{
    /* Open I2C module and set bus clock */
    I2C_Open(I2C0, 400000);
    /* Enable I2C interrupt */
    I2C_DisableInt(I2C0);
		//I2C_DisableTimeout(I2C0);
    NVIC_DisableIRQ(I2C0_IRQn);	
}


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
#endif
