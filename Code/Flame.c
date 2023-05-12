/**
 * 			Flame.c For Automated Home Security System
 *									By Harry Yelland
 *
 * 		Detects Fire within Home by reading input (pulldown).
 *
 *  						18/05/2021 - Added Comments
 */

//includes
#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_gpio.h"
#include "Buzzer.h"

//global variables for the warnings counter and the warnings strings (chararrays)
extern int warnings;
extern char warninglist[128];

//initialises the flame sensor (run at startup)
void initFlame(){
	GPIO_InitTypeDef gpio;
	
	//sets up D2 on board (G6 below) with clock and input mode
	__HAL_RCC_GPIOG_CLK_ENABLE();
  // set mode as input, pulldown
  gpio.Mode = GPIO_MODE_INPUT; 
  gpio.Pull = GPIO_PULLDOWN; 
	gpio.Speed = GPIO_SPEED_HIGH;
	gpio.Pin = GPIO_PIN_6;
	
	// initialise the pin
	HAL_GPIO_Init(GPIOG, &gpio);
}

//gets the input from the flame sensor
void getFlame(){
	//if flame is detected
  if(HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_6) == 1)
  {
		//turn buzzer on and set warnings counter to 1
    setBuzzer();
		warnings = 1;
		return;
   } else {
		 //if no flame, turn off buzzer and set warnings counter to 0
		resetBuzzer();
		warnings = 0;
    return;
    }
}
