/*
 * File:   IOcontent.h
 * Author: ENSF 460 B02 Group 33 
 *         Tanis Smith, Daad Haymour, Jabez Chowdhury
 * Created on: October 15, 2024
 */

#ifndef IOS_H
#define IOS_H

#include <xc.h>
#include "TimeDelay.h"

// Function declarations:
void IOinit();
int IOcheck();
void minute_increment();
void second_increment();
void minute_decrement();
void second_decrement();
void count_down();
void reset_timer();
    

#endif 

