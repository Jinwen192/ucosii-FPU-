#ifndef __ROLLER_H
#define __ROLLER_H

#include <stdint.h>

#define 	STAT1 	0
#define 	STAT2   1
#define     STAT3	3
#define     BIT_0	0x01
#define 	BIT_1 	0x02

struct Roller
{
	uint8_t newstat;
	uint8_t oldstat;
	int32_t cnt;

};

void get_roller_stat (struct Roller* roller);
void run_roller	(struct Roller* roller);
void show_roller (struct Roller* roller);
void roller_init (struct Roller* roller);


#endif
