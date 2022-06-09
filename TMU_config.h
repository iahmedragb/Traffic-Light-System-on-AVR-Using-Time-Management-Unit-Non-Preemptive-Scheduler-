/*
 * TMU_config.h
 *
 *  Created on: May 9, 2022
 *      Author: Ahmed El-Gaafrawy
 */

#ifndef TMU_CONFIG_H_
#define TMU_CONFIG_H_

// enter your os tick time in ms
// it must not exceed 1500 ms
#define OS_TICK				20ul

// select your timer channel [  TIMER0 / TIMER1 / TIMER2  ]
#define TIMER_CH			TIMER0

#define CLOCK_FREQ			16000UL

// enter the num of your tasks in your system
// Don't exceed 10 tasks
#define MAX_NUM_TASKS			10

#endif /* TMU_CONFIG_H_ */
