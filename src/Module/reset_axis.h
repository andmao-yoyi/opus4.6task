
// Copyright (c) vendor. 2023. All rights reserved.
// Author: Application Team
// Date: 2023-03-12

#ifndef __RESETAXIS_H_
#define __RESETAXIS_H_

#include "Logic.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
    u32 axis_num;
    float* ready_pos;
} reset_axis;

void axis_to_ready_pos(logic_para_t *LG, reset_axis *pAxis);

#ifdef __cplusplus
}
#endif

#endif  //__RESETAXIS_H_
