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

#include <math.h>
#include <serial/Plis.hpp>
#include <string.h>


using namespace std;

static const unsigned char encodeTable[2][3] = {
        {PLIS_END, PLIS_ESC, PLIS_ESC_END},
        {PLIS_ESC, PLIS_ESC, PLIS_ESC_ESC}
};

Plis::Plis()
{
}

Plis::~Plis()
{
}

void Plis::encode(ByteVector &data)
{
    ByteVector::iterator it = data.begin();
    while(it != data.end())
    {
        for(int j = 0; j < 2; j++)
        {
            if(*it == encodeTable[j][0])
            {
                it = data.erase(it);
                it = data.emplace(it, encodeTable[j][2]);
                it = data.emplace(it, encodeTable[j][1]);
                break;
            }
        }
        it++;
    }

    data.push_back(PLIS_END);
}

void Plis::decode(ByteVector &data)
{
    const auto dt_start = data.begin();

    uint8_t lastByte = 0;

    size_t i = 0;
    while(i < data.size())
    {
        plis_status_t status = Plis::decodeByte(&(data.at(i)), lastByte);
        switch (status)
        {
            case PLIS_STATUS_ERROR:
            {
                data.clear();
                break;
            }

            case PLIS_STATUS_ESC:
            {
                lastByte = data.at(i);
                data.erase(dt_start + static_cast<long long int>(i));
                break;
            }

            case PLIS_STATUS_END:
            {
                data.erase(dt_start + static_cast<long long int>(i));
                return;
            }

            default:
            {
                if(data.at(i) == PLIS_ESC)
                {
                    lastByte = 0;
                }
                else
                {
                    lastByte = data.at(i);
                }

                i++;

                break;
            }
        }
    }
}

int Plis::overhead(int buffLength)
{
    return static_cast<int>(ceil(static_cast<float>(buffLength) * PLIS_OVERHEAD / 100) + 1);
}

Plis::plis_status_t Plis::decodeByte(uint8_t *byte, uint8_t lastByte)
{
    plis_status_t status = PLIS_STATUS_OK;
    if (lastByte == PLIS_ESC)
    {
        switch (*byte)
        {
            case PLIS_ESC_END:
            {
                *byte = PLIS_END;
                break;
            }

            case PLIS_ESC_ESC:
            {
                *byte = PLIS_ESC;
                break;
            }
            default:
            {
                status = PLIS_STATUS_ERROR;
                break;
            }
        }
    }
    else
    {
        switch (*byte)
        {
            case PLIS_END:
            {
                status = PLIS_STATUS_END;
                break;
            }

            case PLIS_ESC:
            {
                status = PLIS_STATUS_ESC;
                break;
            }
            default:
            {
                break;
            }
        }
    }

    return status;
}

bool Plis::TestDecodeByte(void)
{
    static const uint8_t iters = 18;
    static const uint8_t buffSize = iters + 1;

    /*
     * this buffer is expected to be altered during the test in the following manner:
     * PLIS_ESC_ESC characters changed to: PLIS_ESC
     * PLIS_ESC_END characters changed to: PLIS_END
     */
    uint8_t testBuffer[buffSize] = {
            0, 0, 1, PLIS_ESC, PLIS_ESC_ESC, 4, 5, PLIS_ESC, PLIS_ESC_END, 8, 9, 10, PLIS_ESC_END, 12,
            PLIS_ESC, PLIS_ESC_ESC, PLIS_END, 13, PLIS_END
    };
    static const uint8_t refBuffer[buffSize] = {
            0, 0, 1, PLIS_ESC, PLIS_ESC, 4, 5, PLIS_ESC, PLIS_END, 8, 9, 10, PLIS_ESC_END, 12,
            PLIS_ESC, PLIS_ESC, PLIS_END, 13, PLIS_END
    };
    plis_status_t testStatuses[iters] = {
            PLIS_STATUS_OK, PLIS_STATUS_OK, PLIS_STATUS_ESC, PLIS_STATUS_OK, PLIS_STATUS_ERROR,
            PLIS_STATUS_OK, PLIS_STATUS_ESC, PLIS_STATUS_OK, PLIS_STATUS_OK, PLIS_STATUS_OK,
            PLIS_STATUS_OK, PLIS_STATUS_OK, PLIS_STATUS_OK, PLIS_STATUS_ESC, PLIS_STATUS_OK,
            PLIS_STATUS_ERROR, PLIS_STATUS_OK, PLIS_STATUS_END
    };

    plis_status_t status = PLIS_STATUS_OK;
    for (uint8_t i = 0; i < iters; i++)
    {
        status = decodeByte(&(testBuffer[i+1]), testBuffer[i]);
        if (status != testStatuses[i])
        {
            return false;
        }
    }

    RETURN_VAL_ON_FAIL(memcmp(testBuffer, refBuffer, buffSize) == 0, false);

    return true;
}

bool Plis::TestEncode(void)
{
    static const uint8_t testBuffSize = 16;
    static const uint8_t refBuffSize = 19;
    ByteVector testData;
    ByteVector refData;
    testData.clear();
    refData.clear();

    uint8_t testBuffer[16] = {
        0, 1, 2, PLIS_ESC, 4, 5, 6, PLIS_END, 8, 9, 10, PLIS_ESC_END, 12, 13, 14, PLIS_ESC_ESC
    };

    const uint8_t refBuffer[19] = {
            0, 1, 2, PLIS_ESC, PLIS_ESC_ESC, 4, 5, 6, PLIS_ESC, PLIS_ESC_END, 8, 9, 10,
            PLIS_ESC_END, 12, 13, 14, PLIS_ESC_ESC, PLIS_END
    };

    for(uint8_t i = 0; i < testBuffSize; i++)
    {
        testData.push_back(testBuffer[i]);
    }
    for(uint8_t i = 0; i < refBuffSize; i++)
    {
        refData.push_back(refBuffer[i]);
    }

    const size_t refDataSize = refData.size();

    encode(testData);

    const size_t testDataSize = testData.size();

    RETURN_VAL_ON_FAIL((testDataSize > 0), false);
    RETURN_VAL_ON_FAIL((testDataSize == refDataSize), false);

    uint8_t i;
    for(i = 0; i < testDataSize; i++)
    {
        if(testData.at(i) == refData.at(i))
        {
            continue;
        }
        else
        {
            break;
        }
    }

    RETURN_VAL_ON_FAIL((i == testDataSize), false);

    return true;
}

bool Plis::TestDecode(void)
{
    static const uint8_t testBuffSize = 19;
    static const uint8_t refBuffSize = 16;
    ByteVector testData;
    ByteVector refData;
    testData.clear();
    refData.clear();

    uint8_t testBuffer[testBuffSize] = {
            0, 1, 2, PLIS_ESC, PLIS_ESC_ESC, 4, 5, 6, PLIS_ESC, PLIS_ESC_END, 8, 9, 10, PLIS_ESC_END,
            12, 13, 14, PLIS_ESC_ESC, PLIS_END
    };

    const uint8_t refBuffer[refBuffSize] = {
        0, 1, 2, PLIS_ESC, 4, 5, 6, PLIS_END, 8, 9, 10, PLIS_ESC_END, 12, 13, 14, PLIS_ESC_ESC
    };

    for(uint8_t i = 0; i < testBuffSize; i++)
    {
        testData.push_back(testBuffer[i]);
    }
    for(uint8_t i = 0; i < refBuffSize; i++)
    {
        refData.push_back(refBuffer[i]);
    }

    const size_t refDataSize = refData.size();

    decode(testData);

    const size_t testDataSize = testData.size();

    RETURN_VAL_ON_FAIL((testDataSize > 0), false);
    RETURN_VAL_ON_FAIL((testDataSize == refDataSize), false);

    uint8_t i;
    for(i = 0; i < testDataSize; i++)
    {
        if(testData.at(i) == refData.at(i))
        {
            continue;
        }
        else
        {
            break;
        }
    }

    RETURN_VAL_ON_FAIL((i == testDataSize), false);

    return true;
}
