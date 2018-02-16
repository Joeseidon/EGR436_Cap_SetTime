/*
 * -------------------------------------------
 *    MSP432 DriverLib - v3_21_00_05 
 * -------------------------------------------
 *
 * --COPYRIGHT--,BSD,BSD
 * Copyright (c) 2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
/******************************************************************************
 * MSP432 Empty Project
 *
 * Description: An empty project that uses DriverLib
 *
 *                MSP432P401
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST               |
 *            |                  |
 *            |                  |
 *            |                  |
 *            |                  |
 *            |                  |
 * Author: 
*******************************************************************************/


/* NOTE */

//IF this code is used in another program, make sure you import the startup_msp432p401r_css.c file


/* DriverLib Includes */
#include "driverlib.h"

// --- Standard Includes ---
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "msp.h"
#include "structure.h"
#include "CTS_Layer.h"
#include "CTS_HAL.h"

// --- UART Configuration Parameter ---
const eUSCI_UART_Config uartConfig =
{
        //Configured for 9600 Baud Rate
        EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
        312,                                     // BRDIV = ???
        0,                                       // UCxBRF = ?
        0,                                       // UCxBRS = ?
        EUSCI_A_UART_NO_PARITY,                  // No Parity
        EUSCI_A_UART_LSB_FIRST,                  // MSB First
        EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
        EUSCI_A_UART_MODE,                       // UART mode
        EUSCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION  // Low Frequency Mode
};

int fputc(int _c, register FILE *_fp)
{
    while(!(UCA0IFG&UCTXIFG));
    UCA0TXBUF = (unsigned char) _c;
    return((unsigned char)_c);
}
int fputs(const char *_ptr, register FILE *_fp)
{
    unsigned int i, len;
    len = strlen(_ptr);
    for(i=0 ; i<len ; i++)
    {
        while(!(UCA0IFG&UCTXIFG));
        UCA0TXBUF = (unsigned char) _ptr[i];
    }
return len;
}

void UART_init(void){

    // Selecting P1.2 and P1.3 in UART mode.
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig);         // Configuring UART Module
    MAP_UART_enableModule(EUSCI_A0_BASE);               // Enable UART module
}

uint16_t raw_count[3];
uint16_t delta_count[3];

/* Statics */
 static volatile RTC_C_Calendar newTime;

 /* Time is November 12th 1955 10:03:00 PM */
 const RTC_C_Calendar currentTime =
 {
         0x00,
         0x03,
         0x22,
         0x12,
         0x11,
         0x1955
 };
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    P2DIR |= BIT0;
    P2OUT &= ~BIT0;
    P2DIR |= BIT1;
    P2OUT &= ~BIT1;
    P2DIR |= BIT2;
    P2OUT &= ~BIT2;

    UART_init();


    /* Config RTC */
    /* Configuring pins for peripheral/crystal usage and LED for output */
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ,
            GPIO_PIN0 | GPIO_PIN1, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Setting the external clock frequency. This API is optional, but will
     * come in handy if the user ever wants to use the getMCLK/getACLK/etc
     * functions
     */
    CS_setExternalClockSourceFrequency(32000,48000000);

    /* Starting LFXT in non-bypass mode without a timeout. */
    CS_startLFXT(false);

    /* Initializing RTC with current time as described in time in
     * definitions section */
    MAP_RTC_C_initCalendar(&currentTime, RTC_C_FORMAT_BCD);

    /* Specify an interrupt to assert every minute */
    MAP_RTC_C_setCalendarEvent(RTC_C_CALENDAREVENT_MINUTECHANGE);

    /* Enable interrupt for RTC Ready Status, which asserts when the RTC
     * Calendar registers are ready to read.
     * Also, enable interrupts for the Calendar alarm and Calendar event. */
    MAP_RTC_C_clearInterruptFlag(
            RTC_C_CLOCK_READ_READY_INTERRUPT | RTC_C_TIME_EVENT_INTERRUPT);
    MAP_RTC_C_enableInterrupt(
            RTC_C_CLOCK_READ_READY_INTERRUPT | RTC_C_TIME_EVENT_INTERRUPT);



    //initalize baseline measurement
    TI_CAPT_Init_Baseline(&my_keys);

    //update baseline measurement (average 30 measurements)
    TI_CAPT_Update_Baseline(&my_keys, 30);

    /* Start RTC Clock */
    MAP_RTC_C_startClock();

    /* Enable interrupts and go to sleep. */
    MAP_Interrupt_enableInterrupt(INT_RTC_C);
    MAP_Interrupt_enableSleepOnIsrExit();
    MAP_Interrupt_enableMaster();

    while(1)
    {
        //Record element delta and raw counts
        TI_CAPT_Custom(&my_keys, delta_count);
        TI_CAPT_Raw(&my_keys, raw_count);

        //Inquire if a button has been pressed
        const struct Element *tmp = TI_CAPT_Buttons(&my_keys);

        //Determine if a button has been pressed
        if(&select_element == tmp)
        {
            //Turn on only Red LED
            P2OUT |= BIT0;
            P2OUT &= ~BIT2;
            P2OUT &= ~BIT1;
        }else if(&down_element == tmp){
            //Turn on only Blue LED
            P2OUT |= BIT2;
            P2OUT &= ~BIT0;
            P2OUT &= ~BIT1;
        }else if(&up_element == tmp){
            //Turn on only Green LED
            P2OUT |= BIT1;
            P2OUT &= ~BIT2;
            P2OUT &= ~BIT0;
        }else{
            //Turn all three off
            P2OUT &= ~BIT1;
            P2OUT &= ~BIT2;
            P2OUT &= ~BIT0;
        }
    }
}
