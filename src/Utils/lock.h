#ifndef __LOCK_H
#define __LOCK_H
#include "stdlib.h"
#include "string.h"
#include "ctype.h"
#include "string.h"

typedef struct
{
    u8 year;  //0-99
    u8 month; //1-12
    u8 day;   //1-31
    u8 week;  //这个值没用，禁止使用
} DateType;   //434

extern void LockCheckModul(void);

#endif
