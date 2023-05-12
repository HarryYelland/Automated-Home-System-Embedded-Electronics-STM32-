/**
 * 			Touch.c For Automated Home Security System
 *									By Harry Yelland
 *
 * 		Detects Touch to unlock Home by reading input (pulldown).
 *
 *  						18/05/2021 - Added Comments
 */

//includes
#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_gpio.h"

//global variable for status of capactive touch sensor (on/off)
extern int touch;

//initialises the touch sensor (run on startup) 
void initTouch(){
	GPIO_InitTypeDef gpio;
	
  //sets up D5 on board (I0 below) with clock and input mode
	__HAL_RCC_GPIOI_CLK_ENABLE();
	
  // set mode as input, pulldown
  gpio.Mode = GPIO_MODE_INPUT; 
  gpio.Pull = GPIO_PULLDOWN; 
	gpio.Speed = GPIO_SPEED_HIGH;
	gpio.Pin = GPIO_PIN_0;
	
	// initialise the pin
	HAL_GPIO_Init(GPIOI, &gpio);
}

//gets input from the touch sensor
void getTouch(){
	//sets global status variable to input of the touch sensor
	touch = HAL_GPIO_ReadPin(GPIOI, GPIO_PIN_0);
}
