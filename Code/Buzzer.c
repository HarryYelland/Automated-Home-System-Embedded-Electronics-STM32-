/**
 * 			Buzzer.c For Automated Home Security System
 *									By Harry Yelland
 *
 * 		Acts as Alarm of Home, set and reset functions setup.
 *
 *  						18/05/2021 - Added Comments
 */

//includes
#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_gpio.h"

//global variable for status of buzzer (on/off)
extern int buzzer;

//initialises Buzzer (run on startup)
void initBuzzer(void){
	GPIO_InitTypeDef gpio;
	
	//sets up D1 on board (C6 below) with clock and output mode
	__HAL_RCC_GPIOC_CLK_ENABLE();   
	
  // set mode as output, pulldown
  gpio.Mode = GPIO_MODE_OUTPUT_PP; 
  gpio.Pull = GPIO_PULLDOWN; 
	gpio.Speed = GPIO_SPEED_HIGH;
	gpio.Pin = GPIO_PIN_6;
	
	// initialise the pin
	HAL_GPIO_Init(GPIOC, &gpio);
}

//turn buzzer on
int setBuzzer(void){
	//turns on buzzer, sets global variable to status
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
	buzzer = 1;
	return 1;
}

//reset/turn off buzzer
int resetBuzzer(void){
	//turns off buzzer, sets global variable to status
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
	buzzer = 0;
	return 1;
}

