/*
 * cfuncs.c
 *
 *  Created on: 31 sie 2023
 *      Author: Bartosz
 */

#include "cfuncs.h"
#include "stdbool.h"

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

/*
 * @brief Sums two uint16_t values without
 * using + or - operators.
 *
 * @param x First parameter
 * @param y Second parameter
 *
 * @returns total value as uint32_t
 */
uint32_t CFUNCS_Sum(uint16_t x, uint16_t y)
{
    uint32_t result = 0;

    uint32_t jobToDo = 0x1FFFF;
    uint8_t carryOver = 0;

    while(jobToDo)
    {
        // get the bits
        uint8_t xb = x & 0x1;
        uint8_t yb = y & 0x1;

        // calculate bit result and subsequent carryOver
        uint8_t zb = carryOver ^ xb ^ yb;
        carryOver = (xb & yb) | (carryOver & xb) | (carryOver & yb);

        // update the bit in the result
        result |= (zb << 17);
        result >>= 1;

        // shift parameters and loop conditioner
        x >>= 1;
        y >>= 1;
        jobToDo >>= 1;
    }

    return result;
}
