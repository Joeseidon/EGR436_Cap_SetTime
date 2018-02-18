/*
 * sysTick.c
 *
 *  Created on: Feb 13, 2018
 *      Author: joe
 */

#include "sysTick.h"
#include "msp.h"

void sysTickInit(){
    SysTick -> CTRL = 0;                // disable SysTick during setup
    SysTick -> LOAD = 0x00FFFFFF;       // maximum reload value
    SysTick -> VAL = 0;                 // clears counter
    SysTick -> CTRL = 0x00000005;       // enalbe SysTick, CPU clk, no interrupts
}

void SysTick_delay(uint32_t delay){
    //SysTick has a 3MHz clock rate resulting in a count of 3000 per ms
    SysTick -> LOAD = ((delay*24000) - 1); //resulting in a delay of (delay)
    SysTick -> VAL = 0;                 // clear counter

    //wait for flag
    while((SysTick -> CTRL & 0x00010000) == 0);
}
