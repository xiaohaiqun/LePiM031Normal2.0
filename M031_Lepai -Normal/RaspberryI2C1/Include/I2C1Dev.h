#include <i2c.h>
#include <stdio.h>

extern uint8_t NowBtn;

extern void I2C1readAcc(uint8_t* data);
extern void I2C1readGyro(uint8_t* data);
extern void I2C1readMagn(uint8_t* data);
extern void SensoODR_ONOFF_Handler(uint8_t u8data);
extern uint8_t NineSensorOnOff;

extern void I2C1readPower(uint8_t* data);
extern void I2C1readVout1_2_A(uint8_t* data);
extern void I2C1readBAT_V_I(uint8_t* data);

extern void PowerHandler(uint8_t u8data);




extern void I2C1_IRQHandler(void);
extern void I2C1_GPIO_Init(void);
extern void I2C1_Init(void); 
extern void I2C1_Close(void);
extern void OrderHandler(void);

extern void PWRWU_IRQHandler(void);
