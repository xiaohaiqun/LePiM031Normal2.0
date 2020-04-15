#include <stdint.h>
#include <stdbool.h>

extern uint8_t PowerOn(void);
extern uint8_t PowerOff(void);
#ifndef BUTTONPROCESS_H
#define BUTTONPROCESS_H

 
 void Button_GPIO_Init(void);
 static struct{
  bool btn1;
	bool btn2;
	bool btn3;
	bool btn4;
	bool btn5;
	bool btn6;
	bool btn7;
	bool btn8;
	bool btn9;
}ISButtonPressed={0,0,0,0,0,0,0,0,0};

 static struct{
  bool btn1;
	bool btn2;
	bool btn3;
	bool btn4;
	bool btn5;
	bool btn6;
	bool btn7;
	bool btn8;
	bool btn9;
	bool ONOFF;
}GPIOINTFlag={0,0,0,0,0,0,0,0,0,0};


static struct {
	uint8_t btn1;
	uint8_t btn2;
	uint8_t btn3;
	uint8_t btn4;
	uint8_t btn5;
	uint8_t btn6;
	uint8_t btn7;
	uint8_t btn8;
	uint8_t btn9;
}BtnTimer={0,0,0,0,0,0,0,0,0};

#endif

