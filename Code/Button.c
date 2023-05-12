/**
 * 			Button.c For Automated Home Security System
 *									By Harry Yelland
 *
 * 		Controls Lighting of Home by reading switch input (pulldown).
 *
 *  						18/05/2021 - Added Comments
 */

//includes
#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_gpio.h"
#include "Led.h"

//get global variable for lights being on/off
extern int lights;

//initialises the button (run on startup) 
void initButton(){
	GPIO_InitTypeDef gpio;
	
	//sets up D4 on board (G7 below) with clock and input mode
	__HAL_RCC_GPIOG_CLK_ENABLE();
	
  // set mode as input, pulldown
  gpio.Mode = GPIO_MODE_INPUT; 
  gpio.Pull = GPIO_PULLDOWN; 
	gpio.Speed = GPIO_SPEED_HIGH;
	gpio.Pin = GPIO_PIN_7;
	
	// initialise the pin
	HAL_GPIO_Init(GPIOG, &gpio);
}

//gets the input from the button
void getButton(){
	//if button pressed
  if(HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_7) == 0)
    {
		//if lights on, turn off & update global variable
		if(lights == 1){
			led();
			lights = 0;
		} else {
			//turn off lights & update global variable
			led();
			lights = 1;
		}
  }
}
