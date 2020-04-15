#include <stdint.h>
#define ip5328_slave_adress 0x75

extern void IP5328Init(void);
extern void IP5328Test(void);

extern void I2C1readPower(uint8_t* data); 
extern void I2C1readVout1_2_A(uint8_t* data);
extern void I2C1readBAT_V_I(uint8_t* data);
extern uint8_t PowerOn(void);
extern uint8_t PowerOff(void);
extern void OpenVout2(void);
extern void CloseVout2(void);
