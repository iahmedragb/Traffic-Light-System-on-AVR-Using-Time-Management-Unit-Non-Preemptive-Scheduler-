/*
 * main.c

 *
 *  Created on: May 9, 2022
 *      Author: Ahmed El-Gaafrawy
 */
#include <string.h>
#include "stdTypes.h"
#include "Bit_Math.h"
#include "errorState.h"

#include "DIO/DIO_int.h"
#include "TMU_int.h"
#include "UART_Interface.h"
#include "AVR_REG.h"
/*7 Segments*/
#define A	(1U<<0)
#define B	(1U<<1)
#define C	(1U<<2)
#define D	(1U<<3)
#define E	(1U<<4)
#define F	(1U<<5)
#define G	(1U<<6)
u8 sevseg[10]={
				A|B|C|D|E|F,	//0
				B|C,			//1
				A|B|G|E|D,		//2
				A|B|G|C|D,		//3
				F|G|B|C,		//4
				A|F|G|C|D,		//5
				A|C|D|E|F|G,	//6
				A|B|C,			//7
				A|B|C|D|E|F|G,	//8
				A|B|C|D|F|G};	//9

#define GREEN	5
#define RED		6

#define INITIAL_TIME_INTERVAL	11
typedef struct
{
	u8 LightColor;
	u8 Time;
}TrafficLight;
void Timer(void*p);
void SevSegRight(void*p);
void SevSegLeft(void*p);
void GreenLight(void*p);
void YellowLight(void*p);
void RedLight(void*p);
void UpCount(void*p);
void DownCount(void*p);
void Read(void*p);
void Check(void*p);
void MobileReceive(void*p);
u8 Interval = INITIAL_TIME_INTERVAL;
TrafficLight Traffic1 = {GREEN,INITIAL_TIME_INTERVAL};
char buffer[20];
u8 new;
u8 flag;
int
main()
{

	//UART_Init();
	/*7 Segement for Ones*/
	DDRC =0xff;

	/*7 Segement for Tenth*/
	DDRB =0xff;

	/*Buttons pull-up*/
	DIO_enuSetPinDirection(DIO_u8GROUP_A,DIO_u8PIN0,DIO_u8INPUT);
	DIO_enuSetPinDirection(DIO_u8GROUP_A,DIO_u8PIN1,DIO_u8INPUT);
	DIO_enuSetPinValue(DIO_u8GROUP_A,DIO_u8PIN0,DIO_u8HIGH);
	DIO_enuSetPinValue(DIO_u8GROUP_A,DIO_u8PIN1,DIO_u8HIGH);

	/*Lights*/
	/*Green*/
	DIO_enuSetPinDirection(DIO_u8GROUP_A,DIO_u8PIN2,DIO_u8OUTPUT);
	/*Yellow*/
	DIO_enuSetPinDirection(DIO_u8GROUP_A,DIO_u8PIN3,DIO_u8OUTPUT);
	/*Green*/
	DIO_enuSetPinDirection(DIO_u8GROUP_A,DIO_u8PIN4,DIO_u8OUTPUT);

	TMU_vidInit();

	TMU_vidCreateTask(Timer 		, NULL   , 50  , 0 , 0);
	TMU_vidCreateTask(SevSegRight 	, NULL   , 1  	, 1 , 0);
	TMU_vidCreateTask(SevSegLeft 	, NULL   , 1  	, 2 , 0);
	TMU_vidCreateTask(GreenLight 	, NULL   , 1  	, 3 , 0);
	TMU_vidCreateTask(RedLight 		, NULL   , 1  	, 4 , 0);
	TMU_vidCreateTask(YellowLight 	, NULL   , 1  	, 5 , 0);
	TMU_vidCreateTask(UpCount 		, NULL   , 1  	, 6 , 0);
	TMU_vidCreateTask(DownCount 	, NULL   , 1  	, 7 , 0);
	/*TMU_vidCreateTask(MobileReceive	, NULL   , 1  	, 6 , 0);
	TMU_vidCreateTask(Check		 	, NULL   , 1  	, 7 , 0);
	TMU_vidCreateTask(Read		 	, NULL   , 1  	, 8 , 0);*/
	TMU_vidStartScheduler();
}


void SevSegRight(void*p)
{
	PORTC = sevseg[Traffic1.Time%10];
}
void SevSegLeft(void*p)
{
	PORTB = sevseg[Traffic1.Time/10];
}
void GreenLight(void*p)
{
	if(Traffic1.LightColor == GREEN && Traffic1.Time>1)
	{
		DIO_enuSetPinValue(DIO_u8GROUP_A,DIO_u8PIN4,DIO_u8HIGH);
	}else if(Traffic1.LightColor == GREEN && Traffic1.Time == 0)
	{
		DIO_enuSetPinValue(DIO_u8GROUP_A,DIO_u8PIN4,DIO_u8LOW);
		Traffic1.LightColor = RED;
		Traffic1.Time = Interval;
	}

}
void YellowLight(void*p)
{
	if(Traffic1.Time<4 && Traffic1.Time>0)
		{
			DIO_enuSetPinValue(DIO_u8GROUP_A,DIO_u8PIN3,DIO_u8HIGH);
		}else
		{
			DIO_enuSetPinValue(DIO_u8GROUP_A,DIO_u8PIN3,DIO_u8LOW);
		}
}
void RedLight(void*p)
{
	if(Traffic1.LightColor == RED && Traffic1.Time>1)
		{
			DIO_enuSetPinValue(DIO_u8GROUP_A,DIO_u8PIN2,DIO_u8HIGH);
		}else if(Traffic1.LightColor == RED && Traffic1.Time == 0)
		{
			DIO_enuSetPinValue(DIO_u8GROUP_A,DIO_u8PIN2,DIO_u8LOW);
			Traffic1.LightColor = GREEN;
			Traffic1.Time = Interval;
		}
}
void Timer(void*p)
{
	/*Decrement Timer by 1 every 1 sec*/
	Traffic1.Time--;
}
void UpCount(void*p)
{
	static u8 hold;
	u8 state;
		DIO_enuGetPinValue(DIO_u8GROUP_A,DIO_u8PIN0, &state);
		if(state ==0 && hold ==0)
		{
			hold=1;
			if(Interval < 99)
			{
				Interval++;
			}
		}else if(state == 1 && hold ==1)
		{
			hold=0;
		}

}
void DownCount(void*p)
{
	static u8 hold;
		u8 state;
			DIO_enuGetPinValue(DIO_u8GROUP_A,DIO_u8PIN1, &state);
			if(state ==0 && hold ==0)
			{
				hold=1;
				if(Interval > 0)
				{
					Interval--;
				}
			}else if(state == 1 && hold ==1)
			{
				hold=0;
			}
}
void Read(void*p)
{
	if(flag == 'r')
	{
		if(strncmp("red =",buffer,5) ==0)
		{
			u8 i= strlen(buffer);
			if(i==6)
			{

				Interval = buffer[5] - '0';

			}else if(i==7)
			{
				Interval = (10*(buffer[5] - '0')) +( buffer[6] - '0');
			}

		}
	}

}
void Check(void*p)
{

	if(buffer[0] == 'r')
	{
		flag = 'r';
	}else if(buffer[0] == 'g')
	{
		flag='g';
	}else
	{
		new=1;
		flag=0;
	}
}
void MobileReceive(void*p)
{
	static u8 L_iter;
	if(new)
	{
		new=0;
		L_iter=0;
	}
	if(GET_BIT(UCSRA,7))
	{
		buffer[L_iter] = UART_Receive();
		L_iter++;
	}
}
/*
 void task1(void)
{
	u8 val;
	DIO_enuGetPinValue(DIO_u8GROUP_C , DIO_u8PIN1, &val);
	if ( (val==0) && press ==0 && unpress ==0 )
		press++;
	else if (val==0 && press>0 && unpress == 0)
		hold=1;
	else if (val == 1 && press >= 1 && unpress == 0)
	{
		unpress ++;
		press =0;
	}
	else if ( (val==0) && press ==0 && unpress > 0 )
		press++;
	else if (val==0 && press>0 && unpress > 0)
		hold=1;
	else if (val==1 && press>0 && unpress > 0)
	{
		press=0,hold=0,unpress=0;
	}
}
void task2(void)
{
	if (press > 0)
	{
		DIO_enuSetPinValue(DIO_u8GROUP_C , DIO_u8PIN5, DIO_u8HIGH);


	}
	else if(unpress==0)
	{
		DIO_enuSetPinValue(DIO_u8GROUP_C , DIO_u8PIN5, DIO_u8LOW);
	}
}
 */
