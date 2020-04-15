#include<i2c.h>

#ifndef I2C0DEV_H
#define I2C0DEV_H

void I2C0_GPIO_Init(void);
void I2C0_Init(void);
uint8_t I2C_Write(uint8_t u8SlaveAddr, uint8_t u8DataAddr, uint8_t data);
uint8_t I2C_ReadOneByte(uint8_t u8SlaveAddr, uint8_t u8DataAddr);
uint32_t I2C_ReadMultiByte(uint8_t u8SlaveAddr, uint8_t u8DataAddr, uint8_t rdata[], uint32_t u32rLen);

#endif


