#include "I2c1Dev.h"

void I2C1_GPIO_Init(void)
{
		CLK_EnableModuleClock(I2C1_MODULE);
		SYS->GPB_MFPL = (SYS->GPB_MFPL & ~(SYS_GPB_MFPL_PB2MFP_Msk | SYS_GPB_MFPL_PB3MFP_Msk)) |
                    (SYS_GPB_MFPL_PB2MFP_I2C1_SDA | SYS_GPB_MFPL_PB3MFP_I2C1_SCL);
}

void I2C1_Init(void)
{
    /* Open I2C module and set bus clock */
    I2C_Open(I2C1, 100000);
    /* Get I2C1 Bus Clock */
    /* Set I2C Slave Addresses */
    I2C_SetSlaveAddr(I2C1, 0, 0x15, 0);   /* Slave Address : 0x15 */
		//I2C_EnableInt(I2C1);
    //NVIC_EnableIRQ(I2C1_IRQn);	
		//I2C_EnableWakeup  ( I2C1 );
		/* I2C enter no address SLV mode */
    I2C_SET_CONTROL_REG(I2C1, I2C_CTL_SI_AA);
		//I2C_EnableTimeout(I2C1,1);
}


void I2C1_Close(void)
{
    /* Disable I2C1 interrupt and clear corresponding NVIC bit */
    I2C_DisableInt(I2C1);
    NVIC_DisableIRQ(I2C1_IRQn);
    /* Disable I2C1 and close I2C1 clock */
    I2C_Close(I2C1);
    CLK_DisableModuleClock(I2C1_MODULE);
}


/*extern uint8_t AccP,GyroP,MagnP; //当在I2C1中断外读取九轴数据时使用
extern uint8_t AccData[2][6];
extern uint8_t GyroData[2][6];
extern uint8_t MagnData[2][6];*/

extern uint8_t btnStatus[9];

static uint8_t Order=0;

uint8_t data[9]={0};
static uint8_t datapoint=0;


void ReadOrderHandler(uint8_t Order)
{
	uint8_t i=0;	
	switch(Order){
		case 0x82:
			I2C_SET_DATA(I2C1, NowBtn);//button read
			if((NowBtn&0xF0)==0x10||(NowBtn&0xF0)==0x00)
				NowBtn=0;
			break;
		case 0x83:
			I2C1readAcc(data);       //Acc read
			//data=AccData[AccP];
			datapoint=0;
			I2C_SET_DATA(I2C1, data[datapoint++]);
			break;
		case 0x84:                   //Gyro read
			I2C1readGyro(data);
			//data=GyroData[GyroP];
			datapoint=0;
			I2C_SET_DATA(I2C1, data[datapoint++]);
			break;
		case 0x85:                    //Magn read
			I2C1readMagn(data);
			//data=MagnData[MagnP];
			datapoint=0;
			I2C_SET_DATA(I2C1, data[datapoint++]);
			break;
		case 0x86:                    //Nine Sensor ONOff Read
			I2C_SET_DATA(I2C1, NineSensorOnOff);
			break;
		case 0x87:                   //BtnStatus
			for(i=0;i<9;i++)
				data[i]=btnStatus[i];
			datapoint=0;
			I2C_SET_DATA(I2C1, data[datapoint++]);
			break;
		case 0x8A:                   //Battery Powerdata read  
			I2C1readPower(data);
			datapoint=0;
			I2C_SET_DATA(I2C1, data[datapoint++]);
			break;
		case 0x8B:                   //vout1 and vout2 I read, 
			I2C1readVout1_2_A(data);
			datapoint=0;
			I2C_SET_DATA(I2C1, data[datapoint++]);
			break;
		case 0x8C:                  //
			I2C1readBAT_V_I(data);
			datapoint=0;
			I2C_SET_DATA(I2C1, data[datapoint++]);
			break;
		default:
			I2C_SET_DATA(I2C1, 0x00);
			break;		
	}	
}


void WriteOrderHandler(uint8_t Order,uint8_t u8data)
{
	switch(Order)
	{
		case 0x41:
			PowerHandler(u8data);
			break;
		case 0x46:
			SensoODR_ONOFF_Handler(u8data); //地址0x46管理传感器的开关
			break;
		default:
			break;
	}	
}

uint8_t i2c1InUseFlag=0;


void I2C_SlaveTxRxHandler()
{
	uint8_t u8data;
	uint8_t I2C1Status;
	I2C1Status=I2C_GET_STATUS(I2C1);
	switch(I2C1Status)
	{
		case 0x60:		 // Own SLA+W has been receive; ACK has been return 
		case 0xC0:		 // Data byte or last data in I2CDAT has been transmitted Not ACK has been received 
		case 0x88:		 // Previously addressed with own SLA address; NOT ACK has been returned 
			I2C_SET_CONTROL_REG(I2C1, I2C_CTL_SI_AA);
			break;
		case 0xA0:    // A STOP or repeated START has been received while still addressed as Slave/Receiver
			I2C_SET_CONTROL_REG(I2C1, I2C_CTL_SI_AA);
			break;
		case 0x80:		 //Previously address with own SLA address Data has been received; ACK has been returned
			u8data = (unsigned char) I2C_GET_DATA(I2C1);
			I2C_SET_CONTROL_REG(I2C1, I2C_CTL_SI_AA);
			if((Order&0xC0)==0x40)
			{
				WriteOrderHandler(Order,u8data);
				Order=0x00;
			}
			else
			{
				Order=u8data;
			}
			break;		
		case 0xA8:		 // Own SLA+R has been receive; ACK has been return 
			if((Order&0xC0)==0x80)
			{
				ReadOrderHandler(Order);
			}
			else
			{
				I2C_SET_DATA(I2C1, 0x00);
			}
			I2C_SET_CONTROL_REG(I2C1, I2C_CTL_SI_AA);
			break;
		case 0xB8:		 // Data byte in I2CDAT has been transmitted ACK has been received 
			I2C_SET_DATA(I2C1, data[datapoint++]);
			I2C_SET_CONTROL_REG(I2C1, I2C_CTL_SI_AA);
			break;		
		default:
			break;			
	}
}




/*void I2C1_IRQHandler(void)
{
	 if(I2C_GET_TIMEOUT_FLAG(I2C1))
	 {
			// Clear I2C1 Timeout Flag 
			I2C_ClearTimeoutFlag(I2C1);
	 }
	 else
	 {
		i2c1InUseFlag=1;
		I2C_SlaveTxRxHandler();
		i2c1InUseFlag=0;		
	 }
}*/
