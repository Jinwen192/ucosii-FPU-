#include "roller.h"
#include "main.h"
#include "gpio.h"
#include "oled.h"
#include <stdio.h>
#include <stdarg.h>

void get_roller_stat (struct Roller* roller)
{
	uint8_t str[20];

	if (HAL_GPIO_ReadPin (roll1_GPIO_Port, roll1_Pin))
		roller->newstat = 0X02;
	else
		roller->newstat = 0;
	
	
	if (HAL_GPIO_ReadPin (roll2_GPIO_Port, roll2_Pin))
		roller->newstat = roller->newstat|BIT_0;
	else
		roller->newstat = roller->newstat;
}

static void Add_roller (struct Roller* roller)
{
	roller->cnt++;
	roller->oldstat = roller->newstat;
}
static void Sub_roller (struct Roller* roller)
{
	roller->cnt--;
	if (roller->cnt < 0)
		roller->cnt = 0;
	roller->oldstat = roller->newstat;
}

void run_roller	(struct Roller* roller)
{
	if (roller->newstat == roller->oldstat)	
		return;
	switch (roller->oldstat)
	{
		case STAT1:	
					if (roller->newstat == STAT2)
						Add_roller (roller);
					else if (roller->newstat == STAT3)
						Sub_roller (roller);
					break;
		case STAT2: 
				    if (roller->newstat == STAT3)
						Add_roller (roller);
					else if (roller->newstat == STAT1)
						Sub_roller (roller);
					break;
		case STAT3: 
					if (roller->newstat == STAT1)
						Add_roller (roller);
					else if (roller->newstat == STAT2)
						Sub_roller (roller);
					break;
		default:break;
	}	
}

void show_roller (struct Roller* roller) 
{     
	//float distance = 0;
	uint8_t str[20];
	roller->distance = 0.287 * roller->cnt;
	sprintf ((char*)str, "%0.2fcm   ", roller->distance);
	OLED_ShowStr (0,0, str, 2);
}

void roller_init (struct Roller* roller)
{	
	roller->cnt 	  	= 0;
	roller->newstat	   	= 0;
	roller->oldstat    	= 0;
}
