#include <stdint.h>
/**\name	I2C ADDRESS DEFINITIONS        */
/***************************************************************/
#define BMM050_I2C_ADDRESS                 (0x10)
/***************************************************************/
/**\name	REGISTER ADDRESS DEFINITION        */
/***************************************************************/
/********************************************/
/**\name	CHIP ID       */
/********************************************/
/* Fixed Data Registers */
#define BMM050_CHIP_ID                     (0x40)
/********************************************/
/**\name	DATA REGISTERS       */
/********************************************/
/* Data Registers*/
#define BMM050_DATA_X_LSB                   (0x42)
#define BMM050_DATA_X_MSB                   (0x43)
#define BMM050_DATA_Y_LSB                   (0x44)
#define BMM050_DATA_Y_MSB                   (0x45)
#define BMM050_DATA_Z_LSB                   (0x46)
#define BMM050_DATA_Z_MSB                   (0x47)
#define BMM050_DATA_R_LSB                   (0x48)
#define BMM050_DATA_R_MSB                   (0x49)
/********************************************/
/**\name	REMAPPED DATA REGISTERS      */
/********************************************/
/* Data Registers for remapped axis(XandY)
 * this only applicable for BMX055 */
#define BMM050_REMAPPED_BMX055_DATA_Y_LSB      (0x42)
#define BMM050_REMAPPED_BMX055_DATA_Y_MSB      (0x43)
#define BMM050_REMAPPED_BMX055_DATA_X_LSB      (0x44)
#define BMM050_REMAPPED_BMX055_DATA_X_MSB      (0x45)
/********************************************/
/**\name	INTERRUPT STATUS      */
/********************************************/
/* Status Registers */
#define BMM050_INT_STAT_REG                    (0x4A)
/********************************************/
/**\name	POWER MODE DEFINITIONS      */
/********************************************/
/* Control Registers */
#define BMM050_POWER_CONTROL               (0x4B)
#define BMM050_CONTROL                     (0x4C)
#define BMM050_INT_CONTROL                 (0x4D)
#define BMM050_SENS_CONTROL                (0x4E)
#define BMM050_LOW_THRES                   (0x4F)
#define BMM050_HIGH_THRES                  (0x50)
/********************************************/
/**\name XY AND Z REPETITIONS DEFINITIONS  */
/********************************************/
#define BMM050_REP_XY                      (0x51)
#define BMM050_REP_Z                       (0x52)
/********************************************/
/**\name	TRIM REGISTERS      */
/********************************************/
/* Trim Extended Registers */
#define BMM050_DIG_X1                      (0x5D)
#define BMM050_DIG_Y1                      (0x5E)
#define BMM050_DIG_Z4_LSB                  (0x62)
#define BMM050_DIG_Z4_MSB                  (0x63)
#define BMM050_DIG_X2                      (0x64)
#define BMM050_DIG_Y2                      (0x65)
#define BMM050_DIG_Z2_LSB                  (0x68)
#define BMM050_DIG_Z2_MSB                  (0x69)
#define BMM050_DIG_Z1_LSB                  (0x6A)
#define BMM050_DIG_Z1_MSB                  (0x6B)
#define BMM050_DIG_XYZ1_LSB                (0x6C)
#define BMM050_DIG_XYZ1_MSB                (0x6D)
#define BMM050_DIG_Z3_LSB                  (0x6E)
#define BMM050_DIG_Z3_MSB                  (0x6F)
#define BMM050_DIG_XY2                     (0x70)
#define BMM050_DIG_XY1                     (0x71)

extern void Init_BMM150(void);

extern int16_t Read_BMM150_X(void);

extern int16_t Read_BMM150_Y(void);

extern int16_t Read_BMM150_Z(void);

extern void	BMM150Test();

