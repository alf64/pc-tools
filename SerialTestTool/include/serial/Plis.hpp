/*
***************************************************************************
*
* Author: alf64
*
* Copyright (C) 2019 alf64
*
* Email: alf64gordon@gmail.com
*
***************************************************************************
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* See <http://www.gnu.org/licenses/>.
*
***************************************************************************
*/

/*
 * PLIS -
 * a way of coding stream of data where specific value of byte is being replaced
 * because they are reserved (cannot be used), due to higher level needs.
 *
 * Typical higher level need: specific value in the data stream is indicating end of frame.
 * Therefore such value should not be present in actual data in this frame.
 *
 * In this implementation such reserved value that's being replaced is PLIS_END.
 * So if in the stream there is PLIS_END found, it is being replaced by: PLIS_ESC PLIS_ESC_END.
 * Example: 0xC0 -> 0xDB 0xDC
 * So one character is replaced by two. The first character of this two is called an 'escape' character.
 * This is a special character telling that the subsequent character should be treated as an actual replacement.
 * Therefore 'escape' character is also reserved and should also be replaced in actual stream.
 * So if in the stream there is PLIS_ESC found, it is being replaced by: PLIS_ESC PLIS_ESC_ESC.
 *
 * PLIS has a disadvantage:
 * It adds an overhead to the actual stream, depending on how many bytes are being replaced.
 * If the whole stream contains only PLIS_END characters, then the outcome of PLIS encoding
 * would be a 2x greater stream (because each character would be replaced by 2 characters).
 *
 * Also, at the end of an encoded stream, PLIS_END character is being added to indicate the end of
 * an encoded stream. So in the end user would need buffer of size 2x + 1 (where x = stream size)
 * to safely hold an encoded stream.
 */

#ifndef SERIAL_PLIS_HPP_
#define SERIAL_PLIS_HPP_

#include <iostream>
#include <vector>
#include "ec.h"

typedef std::vector<uint8_t> ByteVector;

#define PLIS_OVERHEAD 200 // %

#define PLIS_END 0xC0
#define PLIS_ESC 0xDB
#define PLIS_ESC_END 0xDC
#define PLIS_ESC_ESC 0xDD

class Plis
{
public:
    Plis();
    ~Plis();

    static void encode(ByteVector &data);
    static void decode(ByteVector &data);
    static int overhead(int buffLength);

    /*
     * Tests Plis::decodeByte() functionality.
     * Returns true if test passes.
     * Returns false if test fails.
     */
    static bool TestDecodeByte(void);

    /*
     * Tests Plis::encode() functionality.
     * Returns true if test passes.
     * Returns false if test fails.
     */
    static bool TestEncode(void);

    /*
     * Tests Plis::decode() functionality.
     * Returns true if test passes.
     * Returns false if test fails.
     */
    static bool TestDecode(void);

private:
    typedef enum __attribute__((packed))
    {
        PLIS_STATUS_OK,
        PLIS_STATUS_ESC,
        PLIS_STATUS_END,
        PLIS_STATUS_ERROR,
    } plis_status_t;

    static plis_status_t decodeByte(uint8_t *byte, uint8_t lastByte);
};



#endif /* SERIAL_PLIS_HPP_ */
