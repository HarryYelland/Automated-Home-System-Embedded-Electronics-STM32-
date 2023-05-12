/**
 * 				Led.c For Automated Home Security System
 *									By Harry Yelland
 *
 * 		Controls Lighting of Home, Either via Toggle.
 * Added functionality for setting to off incase issue with toggle.
 *
 *  						18/05/2021 - Added Comments
 */
 
 //includes
#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_gpio.h"
#include "Flame.h"
#include "cmsis_os.h"


//initialise the Led function (called on startup)
void initLed(void){
		GPIO_InitTypeDef gpio;
	
		//sets up D0 on board (C7 below) with clock and output mode
		__HAL_RCC_GPIOC_CLK_ENABLE();   
		// set mode as output, pulldown
		gpio.Mode = GPIO_MODE_OUTPUT_PP; 
		gpio.Pull = GPIO_PULLDOWN; 
		gpio.Speed = GPIO_SPEED_HIGH;
		// initialise the pin
		gpio.Pin = GPIO_PIN_7;
	
		//initialises the pin
		HAL_GPIO_Init(GPIOC, &gpio);
}

//turns led on
void led(void){
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_7);
}

//turns led off
void ledOff(void){
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
}

