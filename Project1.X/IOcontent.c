/*
 * File:   IOcontent.c
 * Author: ENSF 460 B02 Group 33 
 *         Tanis Smith [30110039], Daad Haymour [30173064], Jabez Chowdhury [30122363]
 * Created on: October 15, 2024
 */

#include "xc.h"
#include "IOcontent.h"
#include "UART2.h"
#include "clkChange.h"
#include <stdio.h> //sprintf for printing times to terminal in proper format

//global counters for minute and second
static uint16_t minute_count = 0; //between 0-59
static uint16_t second_count = 0; //between 0-59

//flag for which button pressed
// 1 = PB1: minute increment
// 2 = PB2: second increment/second increment by 5
// 3 = PB3: start count down/reset
static uint16_t button_type = 0;

void IOinit() {
     
    //LED
    TRISBbits.TRISB8 = 0;
    
    //PB3
    TRISAbits.TRISA4 = 1;
    CNPU1bits.CN0PUE = 1;
    CNEN1bits.CN0IE = 1;
    
    //PB2
    TRISBbits.TRISB4 = 1;
    CNPU1bits.CN1PUE = 1;
    CNEN1bits.CN1IE = 1;
    
    //PB1
    TRISAbits.TRISA2 = 1;
    CNPU2bits.CN30PUE = 1;
    CNEN2bits.CN30IE = 1;
    
    //set up clock and timers
    //for 500 kHz clock, baud rate is 4800
    newClk(500);
        
    //use T3 and T2 as 16 bit timers
    
    //Timer3:
    T2CONbits.T32 = 0; // operate timer 2 as 16 bit timer
    T3CONbits.TCKPS = 2; // set prescaler to 1:64
    T3CONbits.TCS = 0; // use internal clock
    T3CONbits.TSIDL = 0; //operate in idle mode
    
    IPC2bits.T3IP = 2; //7 is highest and 1 is lowest pri.
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 1; //enable timer interrupt
    
    TMR3 = 0;
    
    //Timer 2:
    T2CONbits.TSIDL = 0; //operate in idle mode
    T2CONbits.T32 = 0; // operate timer 2 as 16 bit timer
    T2CONbits.TCKPS = 2; //Prescaler=1:64
    T2CONbits.TCS = 0; // use internal clock
    
    TMR2 = 0; // Clear TMR2 register
       
    IPC1bits.T2IP = 2; //7 is highest priority, 1 is lowest
    IFS0bits.T2IF = 0; //Clear Timer 2 flag
    IEC0bits.T2IE = 1; //enable timer interrupt

}

int IOcheck(){
  
    //button check for CN ISR:
    
    //PB1
    if (PORTAbits.RA2 == 0){
        
        if(button_type == 0){ //use flag to increment while button held down
            button_type = 1; //for auto minute
                                
            //set PR3 and start timer3:
            PR3 = 750<<2;  //PR3 value for 0.75s interval with 500 khz clock, prescalar is 1:64 
            TMR3 = 0;             //reset timer3 counter
            T3CONbits.TON = 1;    //restart timer3
        }
        
    }
   
    //PB2
    else if (PORTBbits.RB4 == 0){   
        
        if(button_type == 0){ //use flag to increment while button held down
            button_type = 2; //for auto minute
                                
            //set PR3 and start timer3:
            PR3 = 750<<2;  //PR3 value for 0.75s interval with 500 khz clock, prescalar is 1:64 //3906?
            TMR3 = 0;             //reset timer3 counter
            T3CONbits.TON = 1;    //restart timer3
        }
        
    }
    
    //PB3
    else if (PORTAbits.RA4 == 0){   
        
        if(button_type == 0){ //use flag to increment while button held down
            button_type = 3; //for auto minute                   
            //set PR3 and start timer3:
            PR3 = 1000<<2;  //PR3 value for 0.5s interval with 500 khz clock, prescalar is 1:64 //3906?
            TMR3 = 0;             //reset timer3 counter
            T3CONbits.TON = 1;    //restart timer3
            
        }
        
    }
    
    //FIX:
    else{
        
        button_type = 0;       //no buttons pushed
        T3CONbits.TON = 0;    //stop Timer3
        LATBbits.LATB8 = 0; //keep LED off

    }
     
    return button_type;
   
}


void minute_increment(){
    //minute ranges btwn 0-59
    if(minute_count < 59){
        minute_count+=1;
    }
    else minute_count = 0;
    
    //display: "SET (MINUTE)m :(SECOND)s\n\r");
    char time_str[20];
    sprintf(time_str, "SET %02dm:%02ds\r", minute_count, second_count);
    Disp2String("\033[2J");
    Disp2String(time_str);
    
}

void second_increment(){
    //second ranges btwn 0-59
    if(second_count < 59){
        second_count+=1;
    }
    else second_count = 0;

    //Disp2String("SET (MINUTE)m :(SECOND)s\n\r");
    char time_str[20];
    sprintf(time_str, "SET %02dm:%02ds\r", minute_count, second_count);
    Disp2String("\033[2J");
    Disp2String(time_str);
}

void second_increment_5(){
    second_count+=5;
    if (second_count >59){
        second_count = second_count-60;
    }

    //Disp2String("SET (MINUTE)m :(SECOND)s\n\r");
    char time_str[20];
    sprintf(time_str, "SET %02dm:%02ds\r", minute_count, second_count);
    Disp2String("\033[2J");
    Disp2String(time_str);
}


void minute_decrement(){
    //minute ranges btwn 0-59
    if(minute_count>0){
        minute_count-=1;
    }
    else{
        minute_count=59;
    }
}

void second_decrement(){
    //second ranges btwn 0-59
    if(second_count>0){
        second_count-=1;
    }
    else{
        second_count = 59;
    }
}

void count_down(){
        
    if(minute_count!=0 || second_count!=0){ //check if time setting is not 00:00
        second_decrement();
        if(second_count == 59){             //if second goes from 0 --> 59, decrement minute too
            minute_decrement();
        }
        if(minute_count!=0 || second_count!=0){     //prevent 00:00 from printing twice
            //start timer 2 for blinking upon countdown start (put here to try and line up with messages to terminal)
            if(T2CONbits.TON == 0){
            PR2 = 1000<<2;
            TMR2 = 0;
            T2CONbits.TON = 1; //start timer 2
            }
            
            //display count down message
            char time_str[20];
            sprintf(time_str, "CNT %02dm:%02ds\r", minute_count, second_count);
            Disp2String("\033[2J");
            Disp2String(time_str);
           
        }
    }
    else{
        //display alarm message
        char time_str[28];
        sprintf(time_str, "FIN %02dm:%02ds --> ALARM\r", minute_count, second_count); //alarm message
        Disp2String("\033[2J");
        Disp2String(time_str);
        T3CONbits.TON = 0;    //stop Timer3 after alarm reached
        T2CONbits.TON = 0; //stop timer 2
        LATBbits.LATB8 = 1; //turn on LED (no blink)
    }
   
}


void reset_timer(){
    minute_count =0;
    second_count=0;
    
    //display clear/reset message
    char time_str[20];
    sprintf(time_str, "CLR %02dm:%02ds\r", minute_count, second_count);
    Disp2String("\033[2J");
    Disp2String(time_str);
    
    //make sure both timers turned off
    T3CONbits.TON = 0;    //stop timer3
    T2CONbits.TON = 0;    //stop timer2
    LATBbits.LATB8 = 0; //turn off LED!
}



