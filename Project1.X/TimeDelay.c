/*
 * File:   TimeDelay.c
 * Author: ENSF 460 B02 Group 33 
 *         Tanis Smith [30110039], Daad Haymour [30173064], Jabez Chowdhury [30122363]
 * Created on: October 15, 2024
 */

#include "xc.h"
#include "TimeDelay.h"

//delay function:
void delay_ms(uint16_t time_ms) {
    
    //PR3 = time_delay_in_ms*fclk/2 *1/1000*1/prescalar from lecture slides
    //PR3 = time_delay_in_ms*(500000 Hz)/2*(0.001)*1*(1/64) = time_delay_ms*3.906
    PR3 = time_ms<<2; //prescalar 1:64
    //T3CONbits.TON = 1;    //start timer3
    Idle();
    //T3CONbits.TON = 0;    //stop timer3
    //IFS0bits.T3IF = 0;
    //TMR3 = 0;
    
}

//tried using to solve pb3 issue, did not work?