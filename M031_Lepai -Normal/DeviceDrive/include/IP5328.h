#include <stdint.h>
#define ip5328_slave_adress 0x75

extern void IP5328Init();
extern void IP5328Test();

extern void I2C1readPower(); 
extern void I2C1readVout1_2_A();
extern void I2C1readBAT_V_I();
extern uint8_t PowerOn();
extern uint8_t PowerOff();
extern void OpenVout2();
extern void CloseVout2();