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
 * My own implementation for B3n3t3l.
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

/*
 * @brief
 * Sums two int values without
 * using + or - operators.
 *
 * Not my own implementation.
 */
int CFUNCS_SumV2(int x, int y)
{
    // Iterate till there is no carry
    while (y != 0)
    {
        // carry should be unsigned to
        // deal with -ve numbers
        // carry now contains common
        //set bits of x and y
        unsigned carry = x & y;

        // Sum of bits of x and y where at
        //least one of the bits is not set
        x = x ^ y;

        // Carry is shifted by one so that adding
        // it to x gives the required sum
        y = carry << 1;
    }

    return x;
}

/*
 * @brief
 * Calls malloc(0) to see what is the returned value by malloc in this case.
 *
 * @details
 * Depending on the environment returned value may be NULL or it may be actual pointer.
 * If actual pointer is returned, user should not use memory pointed by it because
 * there is probably nothing allocated there.
 */
void* CFUNCS_malloc0(void)
{
    return malloc(0);
}
