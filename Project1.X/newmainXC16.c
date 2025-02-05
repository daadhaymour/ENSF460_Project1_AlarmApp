/*
 * File:   newmainXC16.c
 * Author: ENSF 460 B02 Group 33 
 *         Tanis Smith [30110039], Daad Haymour [30173064], Jabez Chowdhury [30122363]
 * Created on: October 15, 2024
 */

// FBS
#pragma config BWRP = OFF               // Table Write Protect Boot (Boot segment may be written)
#pragma config BSS = OFF                // Boot segment Protect (No boot program Flash segment)

// FGS
#pragma config GWRP = OFF               // General Segment Code Flash Write Protection bit (General segment may be written)
#pragma config GCP = OFF                // General Segment Code Flash Code Protection bit (No protection)

// FOSCSEL
#pragma config FNOSC = FRC              // Oscillator Select (Fast RC oscillator (FRC))
#pragma config IESO = OFF               // Internal External Switch Over bit (Internal External Switchover mode disabled (Two-Speed Start-up disabled))

// FOSC
#pragma config POSCMOD = NONE           // Primary Oscillator Configuration bits (Primary oscillator disabled)
#pragma config OSCIOFNC = ON            // CLKO Enable Configuration bit (CLKO output disabled; pin functions as port I/O)
#pragma config POSCFREQ = HS            // Primary Oscillator Frequency Range Configuration bits (Primary oscillator/external clock input frequency greater than 8 MHz)
#pragma config SOSCSEL = SOSCHP         // SOSC Power Selection Configuration bits (Secondary oscillator configured for high-power operation)
#pragma config FCKSM = CSECMD           // Clock Switching and Monitor Selection (Clock switching is enabled, Fail-Safe Clock Monitor is disabled)

// FWDT
#pragma config WDTPS = PS32768          // Watchdog Timer Postscale Select bits (1:32,768)
#pragma config FWPSA = PR128            // WDT Prescaler (WDT prescaler ratio of 1:128)
#pragma config WINDIS = OFF             // Windowed Watchdog Timer Disable bit (Standard WDT selected; windowed WDT disabled)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))

// FPOR
#pragma config BOREN = BOR3             // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware; SBOREN bit disabled)
#pragma config PWRTEN = ON              // Power-up Timer Enable bit (PWRT enabled)
#pragma config I2C1SEL = PRI            // Alternate I2C1 Pin Mapping bit (Default location for SCL1/SDA1 pins)
#pragma config BORV = V18               // Brown-out Reset Voltage bits (Brown-out Reset set to lowest voltage (1.8V))
#pragma config MCLRE = ON               // MCLR Pin Enable bit (MCLR pin enabled; RA5 input pin disabled)

// FICD
#pragma config ICS = PGx2               // ICD Pin Placement Select bits (PGC2/PGD2 are used for programming and debugging the device)

// FDS
#pragma config DSWDTPS = DSWDTPSF       // Deep Sleep Watchdog Timer Postscale Select bits (1:2,147,483,648 (25.7 Days))
#pragma config DSWDTOSC = LPRC          // DSWDT Reference Clock Select bit (DSWDT uses LPRC as reference clock)
#pragma config RTCOSC = SOSC            // RTCC Reference Clock Select bit (RTCC uses SOSC as reference clock)
#pragma config DSBOREN = ON             // Deep Sleep Zero-Power BOR Enable bit (Deep Sleep BOR enabled in Deep Sleep)
#pragma config DSWDTEN = ON             // Deep Sleep Watchdog Timer Enable bit (DSWDT enabled)

// #pragma config statements should precede project file includes.

#include <xc.h>
#include <p24F16KA101.h>
#include "clkChange.h"
#include "UART2.h"
#include "IOcontent.h"
#include "TimeDelay.h"


//list of states:
typedef enum {
    STATE_IDLE,                 //idle mode
    STATE_INCREMENT_MINUTES,    //increment minutes while pb1 pressed
    STATE_INCREMENT_SECONDS,    //increment seconds while pb2 pressed
    STATE_INCREMENT_SECONDS_5,  //increment seconds by 5 while pb2 pressed
    STATE_PB3_PRESS,            //count duration pb3 pressed to dermine if count down or reset
    STATE_COUNT_DOWN,           //start count down if pb3 pressed for less than 3 seconds
    STATE_RESET                 //reset time if pb3 pressed for 3 seconds or more
} State;

static State currentState = STATE_IDLE; //initialize the state to start in idle

//global event tracker
//  0 = no event to check
//  1 = event, check which one!
static uint16_t event_flag = 0;

//flag for auto increment/decrementing minute and second count values in IOcontent.c
static uint16_t button_type = 0;
// 1 = minute increment
// 2 = second increment/ssecond increment by 5
// 3 = count pb3 duration

//flag for counting how long PB3 pressed:
static uint8_t pb3_count = 0;

//flag for counting how long PB2 pressed:
static uint8_t pb2_count = 0;

//main function
int main(void) {
   
    AD1PCFG = 0xFFFF; /* keep this line as it sets I/O pins that can also be analog to be digital */
    
    //IO initialization:
    IOinit();
    
    //set priorities and enable CN interrupt:
    IPC4bits.CNIP = 6;
    IFS1bits.CNIF = 0;
    IEC1bits.CNIE = 1;  //enables the interrupt, set to 0 to disable
    
    /* Let's set up our UART */    
    InitUART2();
    
    while(1) {
        
        Idle(); //wait in idle for interrupts to save resources
        
        //check for determining state upon CN interrupt
        if (event_flag !=0){    //entered from CN interrupt
            //delay_ms(10);     //did not solve issue with pb3?
            button_type = IOcheck();  //after CN interrupt occurs, check which event/button pressed
            event_flag = 0; //reset event_flag
        }
        
        switch (currentState) { 
            case STATE_IDLE:                  //from idle, change state based on return from IOcheck()
                if (button_type == 1) {
                    currentState = STATE_INCREMENT_MINUTES;
                } 
                else if (button_type == 2) {
                    currentState = STATE_INCREMENT_SECONDS;
                } 
                else if (button_type == 3) {
                    currentState = STATE_PB3_PRESS;
                }
                else if (button_type ==4){
                    currentState = STATE_COUNT_DOWN;
                }

                break;

            case STATE_INCREMENT_MINUTES:
        
                if (button_type == 0) {           //if state still increment minutes, but return from IOcheck() is 0, switch to idle
                    currentState = STATE_IDLE; 
                    break;
                }
                minute_increment();
                break;

            case STATE_INCREMENT_SECONDS:
                
                if (button_type == 0) {           //if state still increment seconds, but return from IOcheck() is 0, switch to idle
                    currentState = STATE_IDLE; 
                    pb2_count =0;               //reset pb2_count when button released/switching back to idle
                    break;
                }
                
                if (pb2_count >4){
                    currentState = STATE_INCREMENT_SECONDS_5; //switch to increment by 5 after pb2 pressed longer than 5 seconds
                    break;
                }
                
                second_increment();
                pb2_count +=1;
                
                break;

            case STATE_INCREMENT_SECONDS_5:
                            
                if (button_type == 0) {           //if state still increment seconds by 5, but return from IOcheck() is 0, switch to idle
                    currentState = STATE_IDLE; 
                    pb2_count =0;               //reset pb2_count when button released/switching back to idle
                    break;
                }
                second_increment_5();
                break;
            
            case STATE_PB3_PRESS:
                                
                if (button_type == 0) {       //if code entered after button is released, check value of pb3_count
                    if (pb3_count > 3) {
                        currentState = STATE_RESET;    //if pressed for more than 3 seconds, switch state to reset
                    } else {
                        currentState = STATE_COUNT_DOWN;    //if pressed for 3 seconds or less, switch state to count down
                        button_type = 4;   //update auto_type to repeat countdown for timer interrupts instead of CN interrupt
                    }
                    pb3_count = 0;  //reset pb3_count
                }
                pb3_count +=1;           
                break;

                
            case STATE_COUNT_DOWN:
                
                count_down();   
                
                if (T2CONbits.TON == 0){        //if timer2 off after count_down() returns, change state back to idle
                    currentState = STATE_IDLE;
                }
                else if (button_type ==3){        //if pb3 pressed while state is count down, switch back to state pb3 press
                    currentState = STATE_PB3_PRESS;
    
                }
                   
                break;

            case STATE_RESET:
                reset_timer(); 
                currentState = STATE_IDLE;      //change to idle after reset
                button_type=0; //make sure this is zero!
                break;
        }
    }
    
    return 0;
}


//ISRs:

// Timer 2 interrupt subroutine: use for LED blinking
void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void){
    //Don't forget to clear the timer 2 interrupt flag!
    IFS0bits.T2IF = 0;
    TMR2 = 0;
    LATBbits.LATB8 ^= 1;
    
}

//timer3 interrupt subroutine: use for incrementing and decrementing minutes and seconds
void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void){
    //Don't forget to clear the timer 3 interrupt flag!   
    IFS0bits.T3IF = 0; //Clear timer 3 interrupt flag
    TMR3 = 0;
    
}

void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void){
    //Don't forget to clear the CN interrupt flag!
    IFS1bits.CNIF = 0;
    IEC1bits.CNIE = 0; //disable interrupt 
    event_flag = 1;
    IEC1bits.CNIE = 1; //re-enable interrupt?
    
}
