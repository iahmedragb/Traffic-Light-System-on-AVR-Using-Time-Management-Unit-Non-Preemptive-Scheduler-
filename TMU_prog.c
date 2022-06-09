/*
 * TMU_prog.c
 *
 *  Created on: May 9, 2022
 *      Author: Ahmed El-Gaafrawy
 */
#include "stdTypes.h"
#include "AVR_REG.h"

#include "TMU_config.h"
#include "TMU_priv.h"

static u32 OS_Ticks = 0;

static u8 Num_CTC = 0;
static u8 CTC_Count = 0;

static u16 Num_OVF = 0;
static u16 OVF_Count = 0;

static u8 Preload=0;

static TCB_t All_Tasks[ MAX_NUM_TASKS ];


void TMU_vidInit(void)
{
#if TIMER_CH == TIMER0
	// ctc, dis oc0 pin , 1024 pres
	TCCR0 = 0x0D;
	//max time of OVF
	#if OS_TICK > 0 && OS_TICK <= 16

		OCR0 = (u8) ((OS_TICK * CLOCK_FREQ) / 1024ul);
		Num_CTC = 1;
		CTC_Count = Num_CTC;
		TIMSK |= (1<<1);

	#elif OS_TICK > 16 && OS_TICK <= 1500

		u8 max;
		// for(u8 i=1; i<=16 ; i++)
		for (u8 i=16; i>0;i--)
		{
			if (OS_TICK % i ==0)
			{
				max = i;
				// clear -break- statement if ascending algorithm is used
				break;
			}
		}
		if (max >1)
		{
			OCR0 = (u8) ((max * CLOCK_FREQ) / 1024ul);
			Num_CTC = OS_TICK / max;
			CTC_Count = Num_CTC;
			TIMSK |= (1<<1);
		}
		else if (max == 1)
		{
			// OVF, dis oc0, 1024
			TCCR0 = 0x05;
			f32 ovf = (OS_TICK/16.384);
			if ( ovf - (u32)ovf > 0 )
			{
				Num_OVF = (u16)ovf + 1;
				OVF_Count =  Num_OVF;
				Preload = (u8)(256ul * (1-  (ovf - (u32)ovf)  ) );
				TCNT0 = Preload;
				TIMSK |= (1<<0);
			}
			else
			{
				Num_OVF = (u16)ovf;
				TIMSK |= (1<<0);
			}
		}
	#else
	#error"OS Tick is bigger than 1500 mS"

	#endif

#elif TIMER_CH == TIMER1
	//ctc , dis oc1a-oc1b , 1024
	TCCR1A = 0x00;
	TCCR1B = 0x0D;
	#if OS_TICK <=1500 && OS_TICK > 0

		u16 counts = (u16)((OS_TICK * CLOCK_FREQ)/1024ul);
		OCR1AH = (u8) (counts>>8);
		OCR1AL = (u8) counts;

		TIMSK |= (1<<4);

	#else
	#error"OS Tick is bigger than 1500 mS"

	#endif


#elif TIMER_CH == TIMER2
	// ctc, dis oc2 pin , 1024 pres
	TCCR2 = 0x0F;
	//max time of OVF
	#if OS_TICK > 0 && OS_TICK <= 16

		OCR2 = (u8) ((OS_TICK * CLOCK_FREQ) / 1024ul);
		Num_CTC = 1;
		CTC_Count = Num_CTC;
		TIMSK |= (1<<7);

	#elif OS_TICK > 16 && OS_TICK <= 1500

		u8 max;
		// for(u8 i=1; i<=16 ; i++)
		for (u8 i=16; i>0;i--)
		{
			if (OS_TICK % i ==0)
			{
				max = i;
				// clear -break- statement if ascending algorithm is used
				break;
			}
		}
		if (max >1)
		{
			OCR2 = (u8) ((max * CLOCK_FREQ) / 1024ul);
			Num_CTC = OS_TICK / max;
			CTC_Count = Num_CTC;
			TIMSK |= (1<<7);
		}
		else if (max == 1)
		{
			// OVF, dis oc2, 1024
			TCCR2 = 0x07;
			f32 ovf = (OS_TICK/16.384);
			if ( ovf - (u32)ovf > 0 )
			{
				Num_OVF = (u16)ovf + 1;
				OVF_Count =  Num_OVF;
				Preload = (u8)(256ul * (1-  (ovf - (u32)ovf)  ) );
				TCNT2 = Preload;
				TIMSK |= (1<<6);
			}
			else
			{
				Num_OVF = (u16)ovf;
				TIMSK |= (1<<6);
			}
		}
	#else
	#error"OS Tick is bigger than 1500 mS"

	#endif
#else
#error"Timer channel configuration is wrong"
#endif
}


void TMU_vidCreateTask(void (*pfun)(void*), void * Parameter , u16 Periodicity, u8 Priority , u8 Offset)
{
#if MAX_NUM_TASKS == 0 || MAX_NUM_TASKS > 10
#error " Max num of tasks exceeds 10, it must be below 10"
#endif

	if (pfun != NULL && Periodicity > 0 && Priority < MAX_NUM_TASKS)
	{
		All_Tasks [ Priority ].fun = pfun;
		All_Tasks [ Priority ].periodicity = Periodicity;
		All_Tasks [ Priority ].state = READY;
		All_Tasks [ Priority ].offset = Offset;
		All_Tasks [ Priority ].Parameter = Parameter;
	}
}


void TMU_vidStartScheduler(void)
{
	u32 Temp_Ticks=0;
	asm("SEI");
	while(1)
	{
		if (OS_Ticks > Temp_Ticks)
		{
			Temp_Ticks = OS_Ticks;

			//Descending [highest = max-1]
			//for (s8 i=MAX_NUM_TASKS-1 ; i>=0 ; i--)
			//Ascending  [highest = 0]
			for (u8 i=0 ; i< MAX_NUM_TASKS; i++)
			{
				if (All_Tasks[i].fun != NULL)
				{
					if (
							(
							(
							(
							 (OS_Ticks - All_Tasks[i].offset) % All_Tasks[i].periodicity == 0)
							 && OS_Ticks >= All_Tasks[i].offset
							)
							||
							OS_Ticks == All_Tasks[i].offset
							)
							&& All_Tasks[i].state == READY )
					{
						//Dispatcher
						All_Tasks[i].fun( All_Tasks[i].Parameter );
					}
				}
			}
		}
	}
}


void TMU_vidSuspend( u8 Priority )
{
	if (Priority < MAX_NUM_TASKS)
	{
		All_Tasks[Priority].state = PAUSED;
	}
}


void TMU_vidResume ( u8 Priority )
{
	if (Priority < MAX_NUM_TASKS && All_Tasks[Priority].state == PAUSED)
	{
		All_Tasks[Priority].state = READY;
	}
}


void TMU_vidDelete ( u8 Priority )
{
	if (Priority < MAX_NUM_TASKS)
	{
		All_Tasks[Priority].state = KILLED;
		All_Tasks[Priority].fun =NULL;
	}
}



#if TIMER_CH == TIMER0

void __vector_10(void)__attribute__((signal));
void __vector_10(void)
{
	CTC_Count --;
	if (CTC_Count == 0)
	{
		OS_Ticks++;
		CTC_Count = Num_CTC;
	}

}

void __vector_11(void)__attribute__((signal));
void __vector_11(void)
{
	OVF_Count --;
	if (OVF_Count == 0)
	{
		TCNT0 = Preload;

		OS_Ticks++;

		OVF_Count = Num_OVF;
	}
}

#elif TIMER_CH == TIMER1

void __vector_7(void)__attribute__((signal));
void __vector_7(void)
{
	OS_Ticks++;
}



#elif TIMER_CH == TIMER2

void __vector_4(void)__attribute__((signal));
void __vector_4(void)
{
	CTC_Count --;
	if (CTC_Count == 0)
	{
		OS_Ticks++;
		CTC_Count = Num_CTC;
	}
}

void __vector_5(void)__attribute__((signal));
void __vector_5(void)
{
	OVF_Count --;
	if (OVF_Count == 0)
	{
		TCNT2 = Preload;

		OS_Ticks++;

		OVF_Count = Num_OVF;
	}
}

#endif
