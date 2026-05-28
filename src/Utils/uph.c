#include "uph.h"

s32	uph_cal(uph_def *p)
{
	p->SampleIdx++;
	if(p->SampleIdx >= SAMPLENUM)
	{
		p->SampleIdx = 0;
	}
	p->SampleSum = p->SampleSum - p->Sample[p->SampleIdx] + ((sys_get_tick()-p->TimeStamp));
	p->Sample[p->SampleIdx] = ((sys_get_tick()-p->TimeStamp));
	p->TimeStamp = sys_get_tick();
	p->Cycle = p->SampleSum/SAMPLENUM;
	p->Uph = 3600000/p->Cycle;
	p->Upm = 60000/p->Cycle;
	return p->Uph;
}

