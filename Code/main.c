/**
 * 				Main.c For Automated Home Security System
 *									By Harry Yelland
 *
 * Main controlling class for system, calling for setup of pins,
 * functionality for GLCD panes and creating CMSIS_OS Threads.
 *
 *  						18/05/2021 - Added Comments
 */


//includes
#include <stdio.h>
#include "stm32f7xx_hal.h"
#include "GLCD_Config.h"
#include "Board_GLCD.h"
#include "Board_Touch.h"
#include "Led.h"
#include "Buzzer.h"
#include "Button.h"
#include "Flame.h"
#include "Touch.h"
#include "keypad.h"
#include "cmsis_os.h"

	
//setup for the stm32 device
#define wait_delay HAL_Delay
extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;
#ifdef __RTX
	extern uint32_t os_time;
	uint32_t HAL_GetTick(void) {
	return os_time;
}
#endif


//global variable definitions
TOUCH_STATE tsc_state;	//state of touchscreen
char buffer[128];				//message buffer
char warninglist[128];	//warnings array
int passcode[4];				//array for the passcode
int attempt[4];					//array for user's attempted passcode
int lineHeight = 24;		//height of each line on screen
int warnings = 0;				//warning counter
int loadPane = 0;				//number of pane being displayed
int lights = 0;					//represents if lights on/off
int touch = 0;					//represents if touch sensor pressed
int buzzer = 0;					//represents if buzzer pressed


/**
*		Reference - From Snippet.c Provided on BlackBoard 
*
*
* 				System Clock Configuration
*/
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	/* Enable Power Control clock */
	__HAL_RCC_PWR_CLK_ENABLE();
	/* The voltage scaling allows optimizing the power
	consumption when the device is clocked below the
	maximum system frequency. */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	/* Enable HSE Oscillator and activate PLL
	with HSE as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 25;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);
	/* Select PLL as system clock source and configure
	the HCLK, PCLK1 and PCLK2 clocks dividers */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | 
	RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}

/**
* End of snippet.c
**/


//Program Tidying Function Declarations
int main(void);
void loadMenu(void);
void loadWarnings(void);
void loadBuzzer(void);
void loadLights(void);
void lock(void);
void unlock(void);
void getPasscode(void);
int checkPasscode(void);
void loadLocked(void);


//REFERENCE IDS FOR PANES:
//1 main
//2 warnings
//4 alarm
//5 lights
//6 lock
//7 unlock
//8 unlock instructions

//Check the coordinates of where touchscreen has been pressed
void checkCoords(int x, int y){
	//Buzzer sounds to indicate screen pressed, this stops it
	resetBuzzer();
	
	//based on lineheight & pane number, loads relevant part of pane/function
	if(y >= 1.5*lineHeight && y <= (3.5*lineHeight)-1){
			//if unlock button pressed
			if (loadPane == 6){
					//go to unlock phase
					loadLocked();
					loadMenu();
			}
	} else if(y >= 3.5*lineHeight && y <= (5.5*lineHeight)-1){
			//main menu
			if(loadPane == 1){
					//go to test alarm/buzzer page
					loadBuzzer();
			} else if (loadPane == 6){
					//if lock page, go to menu
					loadMenu();
			}					
	} else if(y >= 5.5*lineHeight && y <= (7.5*lineHeight)-1){
			//main menu
			if(loadPane == 1){
					//load lights page
					loadLights();
			} else if (loadPane == 5){
				//if lights
					if(lights == 1){
							//turn lights off
							lights = 0;
							ledOff();
					} else {
							//turn lights on
							lights = 1;
							led();
					}
					//reload the page to reflect changes
					loadLights();
			} else if (loadPane == 4){
					//if alarm test page, turn on buzzer
					setBuzzer();
			}
	} else if(y >= 7.5*lineHeight && y <= (9.5*lineHeight)-1){
			//main menu
			if(loadPane == 1){
				//go to locking page
					lock();
			} else if (loadPane == 7){
				//if unlock page go to unlocking phase
					unlock();
			}
	} else if(y >= 9.5*lineHeight){
		//if main menu, load warnings
			if(loadPane == 1){
					loadWarnings();
			} else {
				//load main menu
					loadMenu();
			}
	}
	//error state, screen will go black to indicate this shortly if cannot force into an unlocking state
	if(loadPane == -1){
			loadPane = -1;
			while(!tsc_state.pressed){
					Touch_GetState(&tsc_state);
			}
			unlock();
	}
};

//Sets up template for pane
void paneSetup(char title[]){
	//while screen pressed, buzzer goes off to reflect this
	while(tsc_state.pressed){
		setBuzzer();
		Touch_GetState(&tsc_state);
	}
	
	//clear the screen
	GLCD_ClearScreen();
	//set the font
	GLCD_SetFont(&GLCD_Font_16x24);
	//if not warnings page, blue colour scheme, if it is warnings page then red colour scheme
	if(warnings < 1){
		GLCD_SetBackgroundColor(GLCD_COLOR_BLUE);
	} else {
		GLCD_SetBackgroundColor(GLCD_COLOR_RED);
	}
	GLCD_SetForegroundColor(GLCD_COLOR_WHITE);
	//sets title of the page based on parameters passed to function
	sprintf(buffer, "           %s             ", title);
	GLCD_DrawString(0, 0, buffer);
	//adds a return to main menu button at bottom
	GLCD_DrawString(0, 272-lineHeight, "          Main Menu           ");
	GLCD_SetBackgroundColor(GLCD_COLOR_WHITE);
	//if not warnings page, blue colour scheme, if it is warnings page then red colour scheme
	if(warnings < 1 && loadPane != 8){
		GLCD_SetForegroundColor(GLCD_COLOR_BLUE);
	} else {
		GLCD_SetForegroundColor(GLCD_COLOR_RED);
	}
};


//setup for locked page
void loadLocked(void){
	//stop buzzer
	resetBuzzer();
	//sets id of pane
	loadPane = 8;
	//sets title of pane to locked
	paneSetup("Locked");
	//give instructions to user on page
	GLCD_DrawString(0, 2*lineHeight, "To Start Unlock, Hold Touch,");
	GLCD_DrawString(0, 4*lineHeight, "Then Just Enter The Passcode");
	
	//awaits unlock process
	while(touch != 1){
		getTouch();
	};
	while(1){
		getPasscode();
		if(checkPasscode() == 1){
			break;
		}
	}
};


//setup for warnings page
void loadWarnings(){
	//set pane id
	loadPane = 2;
	//set title of pane to warnings
	paneSetup("Warnings");
	//display current warnings
	GLCD_DrawString(0, 2*lineHeight, warninglist);
};
		
//setup for alarm testing page
void loadBuzzer(){
	//waits for user to stop pressing screen
	while(tsc_state.pressed){
		Touch_GetState(&tsc_state);
	}
	//sets id for pane
	loadPane = 4;
	//sets title to "Test Alarm"
	paneSetup("Test Alarm");
	//sets colour scheme
	GLCD_SetBackgroundColor(GLCD_COLOR_WHITE);
	GLCD_SetForegroundColor(GLCD_COLOR_BLUE);
	//setup button to test if buzzer working
	GLCD_DrawString(0, 2*lineHeight, "        Alarm Test      ");
	GLCD_DrawString(0, 6*lineHeight, "     > Press To Test    ");
};

//setup for the lights page
void loadLights(){
	//waits for user to stop pressing screen
	while(tsc_state.pressed){
		setBuzzer();
		Touch_GetState(&tsc_state);
	}
	//sets id for pane
	loadPane = 5;
	//sets title of pane
	paneSetup("Lights");
	//sets colour scheme
	GLCD_SetBackgroundColor(GLCD_COLOR_WHITE);
	GLCD_SetForegroundColor(GLCD_COLOR_BLUE);
	//setups up buttons respective of if lights on/off	
	if(lights == 0){
		GLCD_DrawString(0, 2*lineHeight, "        Lights Off      ");
		GLCD_DrawString(0, 6*lineHeight, "     > Turn Lights On   ");
	} else {
		GLCD_DrawString(0, 2*lineHeight, "        Lights On       ");
		GLCD_DrawString(0, 6*lineHeight, "     > Turn Lights Off  ");
	}	
};

//locking pane
void lock(){
	//set pane id
	loadPane = 6;
	//set pane title
	paneSetup("Turn On Lock?");
	//set locking buttons
	GLCD_DrawString(0, 2*lineHeight, "             Yes        ");
	GLCD_DrawString(0, 4*lineHeight, "              No         ");
}

//unlocking pane
void unlock(){
	//set pane id
	loadPane = 7;
	//set pane title
	paneSetup("Turn Off Lock?");
	//set unlocking buttons
	GLCD_DrawString(0, 2*lineHeight, "             Yes        ");
	GLCD_DrawString(0, 4*lineHeight, "              No         ");
}
		

//setup load menu
void loadMenu(){
	//waits for user to stop pressing the screen
	while(tsc_state.pressed){
		Touch_GetState(&tsc_state);
	}
	//sets id of pane
	loadPane = 1;
	//clears the screen
	GLCD_ClearScreen();
	//runs first time setup of setting font etc 
	GLCD_SetFont(&GLCD_Font_16x24);
	GLCD_SetBackgroundColor(GLCD_COLOR_BLUE);
	GLCD_SetForegroundColor(GLCD_COLOR_WHITE);
	
	//sets title
	GLCD_DrawString(0, 0*lineHeight, "     Home Security System     ");
	
	//displays current amount of warnings
	sprintf(buffer, "  %i Current System Warning(s) ", warnings);
	GLCD_DrawString(0, 272-lineHeight, buffer);
	
	//sets colour scheme
	GLCD_SetBackgroundColor(GLCD_COLOR_WHITE);
	GLCD_SetForegroundColor(GLCD_COLOR_BLUE);
	
	//sets buttons
	GLCD_DrawString(0, 2*lineHeight, "       Choose Option:       ");
	GLCD_DrawString(0, 4*lineHeight, "       > Check Alarm        ");
	GLCD_DrawString(0, 6*lineHeight, "         > Lights           ");
	GLCD_DrawString(0, 8*lineHeight, "     > Lock The System      ");
};

//scans users entered password and checks against system passcode
int checkPasscode(void){
	int i;
	int correct = 0;
	//check if 1st position correct
	if(passcode[0] == attempt[0]){
		correct = 1;
	}
	//check if correct and previous was correct then return
	for(i=1; i<4; i++){
		if(passcode[i] == attempt[i] && correct == 1){
			correct = 1;
		} else {
			correct = 0;
		}
	}
	return correct;
}


//gets input from membrane keypad and adds to array
void getPasscode(void){
	int i;
	//check input valid
	if(getInput() != -1){
		attempt[0] = getInput();
	}
	//check input valid and not the same as previous input number
	for(i=1; i<4; i++){
		while(getInput() == attempt[i-1] || getInput() == -1);
		attempt[i] = getInput();
	}
}


//sets the passcode
void setupPasscode(void){
	passcode[0] = 8;
	passcode[1] = 6;
	passcode[2] = 4;
	passcode[3] = 2;
}


//initialises all gpio pins that have been used by the system
void initPins(){
	initTouch();
	initButton();
	initFlame();
	initBuzzer();
	initLed();
	initializeMembranePins();
}

//creates a thread for flame
void Thread_flame (void const *arg){
	//awaits start signal
	osSignalWait(0x0001, osWaitForever);
	//forever checks if flame
	for(;;){
		getFlame();
	}
}

//creates a thread for button
void Thread_button (void const *arg){
	//awaits a start signal
	osSignalWait(0x0001, osWaitForever);
	//forever checks if button
	for(;;){
		getButton();
	}
}

//creates a thread for main processes
void Thread_main (void const *arg){
	//awaits a start signal
	osSignalWait(0x0001, osWaitForever);
	//while panes are valid
	while (loadPane > 0){
		//get touchscreen inputs
		Touch_GetState(&tsc_state);
		//get inputs from touchsensor
		getTouch();
		//set system warnings
		sprintf(warninglist, "%s", "    No Current Warnings");
		while(warnings > 0 && loadPane != 2){
			sprintf(warninglist, "%s", " FIRE DETECTED - LEAVE ");
			loadWarnings();
		}
		//check if touchscreen pressed
		if(tsc_state.pressed){
			checkCoords(tsc_state.x, tsc_state.y);
		}
	}
	//clear screen if error/page id = -1
	GLCD_SetBackgroundColor(GLCD_COLOR_BLACK);
	GLCD_ClearScreen();
}

//Definitions of threads, setting priorities
osThreadDef (Thread_main, osPriorityHigh, 1, 0);
osThreadDef (Thread_flame, osPriorityRealtime, 1, 0);
osThreadDef (Thread_button, osPriorityRealtime, 1, 0);
 
//start function to create threads
static void start (void)  {
	//create id's for threads
  int32_t signals;
  osThreadId main_id;
	osThreadId flame_id;
	osThreadId button_id;

	//while no threads created/thread fail to create
  while (main_id == NULL || flame_id == NULL || button_id == NULL)  {
    //Creates all threads
		flame_id = osThreadCreate (osThread(Thread_flame), NULL);
		main_id = osThreadCreate (osThread(Thread_main), NULL);
		button_id = osThreadCreate (osThread(Thread_button), NULL);
  }
	
	//set signals to turn on
  signals = osSignalSet (flame_id, 0x0001);
	signals = osSignalSet (main_id, 0x0001);
	signals = osSignalSet (button_id, 0x0001);
	
	//go forever
	osDelay(osWaitForever);
	while(1);
}


//setup system
int main(void){
	//HAL & System Config
	HAL_Init();
	SystemClock_Config();
	//Initialise Touchscreen/GLCD
	Touch_Initialize();
	GLCD_Initialize();
	//Create passcode for device
  setupPasscode();
	//loads the main menu
	loadMenu();
	//initialises the pins used by gpio
	initPins();
	//updates system core clock for cmsis threads
	SystemCoreClockUpdate();
	//start threads
	start();
};


