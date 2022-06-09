/*
 * TMU_int.h
 *
 *  Created on: May 9, 2022
 *      Author: Ahmed El-Gaafrawy
 */

#ifndef TMU_INT_H_
#define TMU_INT_H_

void TMU_vidInit(void);

void TMU_vidCreateTask(void (*pfun)(void*), void * Parameter ,u16 Periodicity, u8 Priority, u8 Offset);

void TMU_vidStartScheduler(void);

void TMU_vidSuspend( u8 Priority );

void TMU_vidResume ( u8 Priority );

void TMU_vidDelete ( u8 Priority );



#define READY			56
#define PAUSED			34
#define KILLED			90

#endif /* TMU_INT_H_ */
