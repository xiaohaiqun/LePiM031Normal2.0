#include <stdio.h>
#include <sys.h>
#include "SYS_init.h"
#include <LSM6DSL.h>
#include <BMM150.h>
#include <IP5328.h>
#include <ButtonProcess.h>
#include <pwm_light.h>
#include "I2C1Dev.h"
#include "I2C0Dev.h"
#include "TimerPeriodic.h"
//DEBUG_ENABLE_SEMIHOST
/////////////////////////////////////////////////////////

void device_GPIO_Init(){
	Button_GPIO_Init();
	PWM_GPIO_Init();
	I2C0_GPIO_Init();
	I2C1_GPIO_Init();
	TIMER_GPIO_Init();
}

void reset(){
		SYS_Init();
	  powerOnLight();		
		I2C0_Init();
		Init_BMM150();
    Init_LSM6DSL();	
		IP5328Init();	
		TIMER_Init();//time0 start after ip5328 is ready!	
		I2C1_Init();	
		
}
extern uint8_t PowerState;

extern void BtnLongPressHandler(void);

extern void OneSecRound(void);
extern void TenMicSecRound(void);
int32_t main()
{
		reset();
		PowerState=1;
		while(1)
		{
			TenMicSecRound();
			
			OneSecRound();
			
			BtnLongPressHandler();
		}
}
