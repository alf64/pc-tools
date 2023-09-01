/*
 * cfuncs.c
 *
 *  Created on: 31 sie 2023
 *      Author: Bartosz
 */

#include "cfuncs.h"

uint32_t CFUNCS_Add(uint16_t a, uint16_t b)
{
    return (a + b);
}

void CFUNCS_Fn(uint16_t x)
{
    printf("%d\n", x);
}

void CFUNCS_FnCall(cfuncs_fn_t func)
{
    func(CFUNCS_FN_U16_ARG);
}
