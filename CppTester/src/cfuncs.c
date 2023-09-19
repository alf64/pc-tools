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
 * using + or -.
 */
uint32_t CFUNCS_Sum(uint16_t x, uint16_t y)
{
    bool carryOver = false;
    uint32_t result = 0;

    uint8_t bitWidth = (sizeof(uint16_t) * 8);

    for(uint8_t i = 0; i < bitWidth; i++)
    {
        uint8_t xp = (x >> i) & 0x1;
        uint8_t yp = (y >> i) & 0x1;

        uint8_t res1 = xp ^ yp;

        if(carryOver)
        {
            carryOver = false;
            res1 ^= 1;

            if(res1 == 0)
            {
                carryOver = true;
            }
        }

        if((xp == 1 && yp == 1))
        {
            carryOver = true;
        }

        if(res1)
        {
            result |= (1<<i);
        }
        else
        {
            result &= ~(1<<i);
        }

        // carryOver last bit if needed (17th)
        if((i == (bitWidth - 1)) && carryOver)
        {
            result |= (1<<(i+1));
        }
    }

    return result;
}
