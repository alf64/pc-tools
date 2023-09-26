/*
 * cfuncs.h
 *
 *  Created on: 31 sie 2023
 *      Author: Bartosz
 */

#ifndef CFUNCS_H_
#define CFUNCS_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define CFUNCS_FN_U16_ARG 0xFFFF
typedef void(*cfuncs_fn_t)(uint16_t x);

uint32_t CFUNCS_Add(uint16_t a, uint16_t b);

void CFUNCS_FnCall(cfuncs_fn_t func);

void CFUNCS_Fn(uint16_t x);

uint32_t CFUNCS_Sum(uint16_t x, uint16_t y);
int CFUNCS_SumV2(int x, int y);
void* CFUNCS_malloc0(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CFUNCS_H_ */
