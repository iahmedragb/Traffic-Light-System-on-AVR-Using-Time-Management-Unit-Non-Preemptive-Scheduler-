/*
 * TMU_priv.h
 *
 *  Created on: May 9, 2022
 *      Author: Ahmed El-Gaafrawy
 */

#ifndef TMU_PRIV_H_
#define TMU_PRIV_H_

#define TIMER0			62
#define TIMER1			96
#define TIMER2			75


typedef struct
{
	void (*fun)(void*);
	void * Parameter;
	u16 periodicity;
	u8 state;
	u8 offset;
}TCB_t;


#define READY			56
#define PAUSED			34
#define KILLED			90

#endif /* TMU_PRIV_H_ */
