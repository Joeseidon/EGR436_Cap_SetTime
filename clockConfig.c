/*
 * clockConfig.c
 *
 *  Created on: Feb 4, 2018
 *      Author: joe
 */

#include "clockConfig.h"
#include "driverlib.h"

volatile int MCLKfreq, SMCLKfreq;

int getMCLKfreq(void){
    return MCLKfreq;
}

int getSMCLKfreq(void){
    return SMCLKfreq;
}

void clockStartUp(void){
    clockInit48MHzXTL();  // set up the clock to use the crystal oscillator on the Launchpad
    MAP_CS_initClockSignal(CS_MCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);
    MAP_CS_initClockSignal(CS_SMCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_4);
    //    MAP_CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);  // use this if the crystal oscillator does not respond
    //    MAP_CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    //    MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_4);  // set SMCLK to 12 MHz
    SMCLKfreq=MAP_CS_getSMCLK();  // get SMCLK value to verify it was set correctly
    MCLKfreq=MAP_CS_getMCLK();  // get MCLK value
}

void clockInit48MHzXTL(void) {  // sets the clock module to use the external 48 MHz crystal

    /* Configuring pins for peripheral/crystal usage */
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ,
                                                    GPIO_PIN3 | GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);

    CS_setExternalClockSourceFrequency(32000,48000000); // enables getMCLK, getSMCLK to know externally set frequencies

    /* Starting HFXT in non-bypass mode without a timeout. Before we start
     * we have to change VCORE to 1 to support the 48MHz frequency */
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);
    MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
    MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);
    CS_startHFXT(false);  // false means that there are no timeouts set, will return when stable

    /* Initializing MCLK to HFXT (effectively 48MHz) */
    MAP_CS_initClockSignal(CS_MCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);
}

