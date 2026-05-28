#ifndef	_UPH_H_
#define	_UPH_H_


#include "data_define.h"

#define	SAMPLENUM	1

typedef struct
{
	s32	Cycle;					
	s32	Uph;
	s32	Upm;
	s32	Sample[SAMPLENUM];
	s32	SampleSum;
	s32	SampleIdx;
	u32	TimeStamp;
}uph_def;

extern s32	uph_cal(uph_def *p);

#endif
