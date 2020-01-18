#include <stdint.h>
//////LED pin////////////////////
#define redpin1   4
#define greenpin1 3
#define bluepin1  5 
#define redpin2   1
#define greenpin2 0
#define bluepin2  2
//////LED state////////////////
#define dark   0
#define lightgreen 1
#define red    2
#define green  3
#define blue   4
#define yellow 5
#define LedON  6
#define LedOFF 7

extern void PWM_GPIO_Init();
extern void powerOnLight();
extern void powerOffLight();
extern void Pwm_light_test();
void LEDChange(uint8_t Ledstate);
void LED1Change(uint8_t Ledstate);
