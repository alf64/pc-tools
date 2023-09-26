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

#include <stdlib.h>
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#endif

#include "SerialDeviceTester.hpp"

using namespace std;

constexpr const uint8_t SerialDeviceTester::_sensxWrongPlisAgpFrame1[];
constexpr const uint8_t SerialDeviceTester::_sensxWrongPlisAgpFrame2[];
constexpr const uint8_t SerialDeviceTester::_sensxWrongPlisAgpFrame3[];
const uint8_t SerialDeviceTester::_sensxUnknownFunction;
constexpr const uint32_t SerialDeviceTester::_sensxRtcSetTestValues[];
constexpr const uint32_t SerialDeviceTester::_sensxHostWakeUpTimeSetTestValues[];

SerialDeviceTester::SerialDeviceTester(void):
        _initOk(false)
{
    if(Init() == EC_OK)
    {
        _initOk = true;
    }
}

SerialDeviceTester::~SerialDeviceTester(void)
{
    delete m_sensx;
    m_sensx = NULL;
    delete m_serial;
    m_serial = NULL;
}

ec_t SerialDeviceTester::Init(void)
{
    m_serial = new Serial();
    m_sensx = new alf64::devices::SensX();
    _dut = DUT_SENSX;
    _responseTimeoutMs = _sensxTimeoutMs;

    return EC_OK;
}

bool SerialDeviceTester::TestSelfPlis(void)
{
    RETURN_VAL_ON_FAIL(_initOk, false);

    RETURN_VAL_ON_FAIL((Plis::TestDecodeByte()), false);
    RETURN_VAL_ON_FAIL((Plis::TestDecode()), false);
    RETURN_VAL_ON_FAIL((Plis::TestEncode()), false);

    return true;
}

ec_t SerialDeviceTester::SetDut(dut_t dut)
{
    RETURN_VAL_ON_FAIL(_initOk, EC_FAIL);

    ec_t ec = EC_FAIL;

    switch(dut)
    {
        case DUT_SENSX:
        {
            _dut = dut;
            ec = EC_OK;
            break;
        }
        default:
        {
            break;
        }
    }

    return ec;
}

SerialDeviceTester::dut_t SerialDeviceTester::GetDut(void)
{
    return _dut;
}

void SerialDeviceTester::SetResponseTimeoutMs(uint32_t timeoutMs)
{
    RETURN_VOID_ON_FAIL(_initOk);

    _responseTimeoutMs = timeoutMs;
}

uint32_t SerialDeviceTester::GetResponseTimeoutMs(void)
{
    return _responseTimeoutMs;
}

bool SerialDeviceTester::TestFwVersionRead(void)
{
    RETURN_VAL_ON_FAIL(_initOk, false);
    RETURN_VAL_ON_FAIL(_dut == DUT_SENSX, false);

    m_sensx->ClearRequestQueue();
    GeneralDevice::Status status = m_sensx->ReadFwVersion(); // builds agp request
    RETURN_VAL_ON_FAIL(status == GeneralDevice::OK, false);

    return ProcessAgpRequest();
}

bool SerialDeviceTester::TestDeviceStatusRead(void)
{
    RETURN_VAL_ON_FAIL(_initOk, false);
    RETURN_VAL_ON_FAIL(_dut == DUT_SENSX, false);

    m_sensx->ClearRequestQueue();
    GeneralDevice::Status status = m_sensx->ReadDeviceStatus();
    RETURN_VAL_ON_FAIL(status == GeneralDevice::OK, false);

    return ProcessAgpRequest();
}

bool SerialDeviceTester::TestMeasurementRead(void)
{
    RETURN_VAL_ON_FAIL(_initOk, false);
    RETURN_VAL_ON_FAIL(_dut == DUT_SENSX, false);

    m_sensx->ClearRequestQueue();
    GeneralDevice::Status status = m_sensx->MeasurementsRead();
    RETURN_VAL_ON_FAIL(status == GeneralDevice::OK, false);

    return ProcessAgpRequest();
}

bool SerialDeviceTester::TestRtcSet(void)
{
    RETURN_VAL_ON_FAIL(_initOk, false);
    RETURN_VAL_ON_FAIL(_dut == DUT_SENSX, false);

    bool result = false;
    for(uint8_t i = 0; i < ARRAY_LENGTH(_sensxRtcSetTestValues); i++)
    {
        m_sensx->ClearRequestQueue();
        GeneralDevice::Status status = m_sensx->RtcSet(_sensxRtcSetTestValues[i]);
        RETURN_VAL_ON_FAIL(status == GeneralDevice::OK, false);
        result = ProcessAgpRequest();
        RETURN_VAL_ON_FAIL(result, result);
    }

    return result;
}

bool SerialDeviceTester::TestHostWakeUpTimeSet(void)
{
    RETURN_VAL_ON_FAIL(_initOk, false);
    RETURN_VAL_ON_FAIL(_dut == DUT_SENSX, false);

    bool result = false;
    uint8_t testValsLength = ARRAY_LENGTH(_sensxHostWakeUpTimeSetTestValues);
    for(uint8_t i = 0; i < testValsLength; i++)
    {
        if(i == 0)
        {
            m_sensx->ClearRequestQueue();
            GeneralDevice::Status status = m_sensx->RtcSet(0);
            RETURN_VAL_ON_FAIL(status == GeneralDevice::OK, false);
            result = ProcessAgpRequest();
            RETURN_VAL_ON_FAIL(result, result);
        }

        m_sensx->ClearRequestQueue();
        GeneralDevice::Status status =
                m_sensx->HostWakeTimeSet(_sensxHostWakeUpTimeSetTestValues[i]);
        RETURN_VAL_ON_FAIL(status == GeneralDevice::OK, false);
        result = ProcessAgpRequest();

        if(i < (testValsLength - 1))
        {
            RETURN_VAL_ON_FAIL(result, result);
        }
        else
        {
            RETURN_VAL_ON_FAIL(!result, result); // error on last one is expected
            result = true;
        }
    }

    return result;
}

bool SerialDeviceTester::TestFuncStress(void)
{
    RETURN_VAL_ON_FAIL(_initOk, false);
    RETURN_VAL_ON_FAIL(_dut == DUT_SENSX, false);

    bool retval = true;
    m_sensx->ClearRequestQueue();

    GeneralDevice::Status status = GeneralDevice::HwFault;
    uint32_t framesPerVariant = _sensxTestFuncStressFrames / _sensxTestFuncStressVariants;
    uint32_t framesLastVariant =
            framesPerVariant + (_sensxTestFuncStressFrames % _sensxTestFuncStressVariants);
    const uint32_t totalFrames =
            (framesPerVariant * (_sensxTestFuncStressVariants-1)) + framesLastVariant;
    RETURN_VAL_ON_FAIL(
            (totalFrames == _sensxTestFuncStressFrames),
            false);

    uint32_t timeout = GetResponseTimeoutMs();
    SetResponseTimeoutMs(_sensxStressTimeoutsMs);

    uint32_t framesCnt = 0;
    for(uint32_t variant = 1, loopBr = 0, frameLimit = 0;
            (variant <= _sensxTestFuncStressVariants) && (loopBr == 0);
            variant++)
    {
        switch(variant)
        {
            case 1:
            {
                frameLimit += framesPerVariant;
                while(framesCnt < frameLimit)
                {
                    status = m_sensx->ReadFwVersion();
                    BREAK_ON_FAIL(status == GeneralDevice::OK);
                    BREAK_ON_FAIL(ProcessAgpRequest());
                    framesCnt++;
                }
                if(framesCnt != frameLimit)
                {
                    loopBr = 1;
                }

                break;
            }
            case 2:
            {
                frameLimit += framesPerVariant;
                while(framesCnt < frameLimit)
                {
                    status = m_sensx->ReadDeviceStatus();
                    BREAK_ON_FAIL(status == GeneralDevice::OK);
                    BREAK_ON_FAIL(ProcessAgpRequest());
                    framesCnt++;
                }
                if(framesCnt != frameLimit)
                {
                    loopBr = 1;
                }

                break;
            }
            case 3:
            {
                frameLimit += framesPerVariant;
                while(framesCnt < frameLimit)
                {
                    status = m_sensx->MeasurementsRead();
                    BREAK_ON_FAIL(status == GeneralDevice::OK);
                    BREAK_ON_FAIL(ProcessAgpRequest());
                    framesCnt++;
                }
                if(framesCnt != frameLimit)
                {
                    loopBr = 1;
                }

                break;
            }
            case 4:
            {
                uint32_t queueCnt = framesLastVariant;
                while(queueCnt)
                {
                    GeneralDevice::Status status = m_sensx->ReadFwVersion();
                    BREAK_ON_FAIL(status == GeneralDevice::OK);
                    BREAK_ON_FAIL((--queueCnt));
                    status = m_sensx->ReadDeviceStatus();
                    BREAK_ON_FAIL(status == GeneralDevice::OK);
                    BREAK_ON_FAIL((--queueCnt));
                    status = m_sensx->MeasurementsRead();
                    BREAK_ON_FAIL(status == GeneralDevice::OK);
                    queueCnt--;
                }
                if(queueCnt > 0)
                {
                    loopBr = 1;
                    break;
                }

                frameLimit += framesLastVariant;
                while(framesCnt < frameLimit)
                {
                    BREAK_ON_FAIL(ProcessAgpRequest());
                    framesCnt++;
                }
                if(framesCnt != frameLimit)
                {
                    loopBr = 1;
                }

                break;
            }
            default:
            {
                loopBr = 1;

                break;
            }
        }
    }
    if(framesCnt != totalFrames)
    {
        printf("Processed frames: %d / %d \n", framesCnt, totalFrames);
        retval = false;
    }

    SetResponseTimeoutMs(timeout);

    return retval;
}

bool SerialDeviceTester::TestNegUnknownFunc(void)
{
    RETURN_VAL_ON_FAIL(_initOk, false);
    RETURN_VAL_ON_FAIL(_dut == DUT_SENSX, false);

    m_sensx->ClearRequestQueue();

    // build proper agp frame
    GeneralDevice::Status status = m_sensx->ReadFwVersion();
    RETURN_VAL_ON_FAIL(status == GeneralDevice::OK, false);

    // distort frame function field
    ByteVector agpRequest;
    ByteVector agpResponse;
    agpRequest.clear();
    agpResponse.clear();
    int expRespSize = m_sensx->GetNextRequest(agpRequest);
    RETURN_VAL_ON_FAIL(expRespSize > 0, false);
    RETURN_VAL_ON_FAIL(agpRequest.size() > DEVICE_CRC_SIZE, false);
    expRespSize = _sensxErrorRespLen + DEVICE_CRC_SIZE;
    agpRequest.resize(agpRequest.size() - DEVICE_CRC_SIZE);
    agpRequest.shrink_to_fit();
    agpRequest.at(0) = _sensxUnknownFunction;
    uint16_t crc = GeneralDevice::Crc16(agpRequest);
    agpRequest.push_back(static_cast<uint8_t>((crc & 0xFF)));
    agpRequest.push_back(static_cast<uint8_t>(((crc >> 8) & 0xFF)));

    ec_t ec = TriggerComm(agpRequest, agpResponse, static_cast<unsigned int>(expRespSize));
    if(m_serial->isComPortOpened())
    {
        m_serial->CloseComPort();
    }
    RETURN_VAL_ON_FAIL(ec == EC_OK, false);

    /*
     * Additional manual check of the status,
     * because for unknown function ParseResponse() always
     * returns UnsupportedCmd, no matter what the status field
     * in the agp frame was.
     */
    status = static_cast<GeneralDevice::Status>(agpResponse.at(0));
    RETURN_VAL_ON_FAIL(status == GeneralDevice::UnsupportedCmd, false);
    status = m_sensx->ParseResponse(agpResponse);
    RETURN_VAL_ON_FAIL(status == GeneralDevice::UnsupportedCmd, false);

    return true;
}

bool SerialDeviceTester::TestNegWrongCrc(void)
{
    RETURN_VAL_ON_FAIL(_initOk, false);
    RETURN_VAL_ON_FAIL(_dut == DUT_SENSX, false);

    m_sensx->ClearRequestQueue();

    GeneralDevice::Status status = m_sensx->ReadFwVersion();
    RETURN_VAL_ON_FAIL(status == GeneralDevice::OK, false);

    // alter crc
    ByteVector agpRequest;
    ByteVector agpResponse;
    agpRequest.clear();
    agpResponse.clear();
    int expRespSize = m_sensx->GetNextRequest(agpRequest);
    RETURN_VAL_ON_FAIL(expRespSize > 0, false);
    RETURN_VAL_ON_FAIL(agpRequest.size() > DEVICE_CRC_SIZE, false);
    expRespSize = _sensxErrorRespLen + DEVICE_CRC_SIZE;
    agpRequest.at(agpRequest.size() - DEVICE_CRC_SIZE) =
            static_cast<uint8_t>((_sensxWrongCrc >> 8) & 0xFF);
    agpRequest.at(agpRequest.size() - (DEVICE_CRC_SIZE+1)) =
            static_cast<uint8_t>(_sensxWrongCrc & 0xFF);

    ec_t ec = TriggerComm(agpRequest, agpResponse, static_cast<unsigned int>(expRespSize));
    if(m_serial->isComPortOpened())
    {
        m_serial->CloseComPort();
    }
    RETURN_VAL_ON_FAIL(ec == EC_OK, false);

    status = m_sensx->ParseResponse(agpResponse);
    RETURN_VAL_ON_FAIL(status == GeneralDevice::CrcError, false);

    return true;
}

bool SerialDeviceTester::TestNegWrongPlis(void)
{
    RETURN_VAL_ON_FAIL(_initOk, false);

    unsigned int expRespSize = _sensxErrorRespLen + DEVICE_CRC_SIZE;

    for(uint8_t i = 0; i < 3; i++)
    {
        ByteVector agpRequest;
        ByteVector agpResponse;
        agpRequest.clear();
        agpResponse.clear();

        switch(i)
        {
            case 0:
            {
                agpRequest.assign(
                        _sensxWrongPlisAgpFrame1,
                        _sensxWrongPlisAgpFrame1 + sizeof(_sensxWrongPlisAgpFrame1));
                RETURN_VAL_ON_FAIL((agpRequest.size() == sizeof(_sensxWrongPlisAgpFrame1)), false);

                break;
            }
            case 1:
            {
                agpRequest.assign(
                        _sensxWrongPlisAgpFrame2,
                        _sensxWrongPlisAgpFrame2 + sizeof(_sensxWrongPlisAgpFrame2));
                RETURN_VAL_ON_FAIL((agpRequest.size() == sizeof(_sensxWrongPlisAgpFrame2)), false);

                break;
            }
            case 2:
            {
                agpRequest.assign(
                        _sensxWrongPlisAgpFrame3,
                        _sensxWrongPlisAgpFrame3 + sizeof(_sensxWrongPlisAgpFrame3));
                RETURN_VAL_ON_FAIL((agpRequest.size() == sizeof(_sensxWrongPlisAgpFrame3)), false);

                break;
            }
            default:
            {
                break;
            }
        }

        ec_t ec = TriggerComm(agpRequest, agpResponse, expRespSize, PLIS_DECODE);
        if(m_serial->isComPortOpened())
        {
            m_serial->CloseComPort();
        }
        RETURN_VAL_ON_FAIL(ec == EC_OK, false);

        GeneralDevice::Status status = static_cast<GeneralDevice::Status>(agpResponse.at(0));
        RETURN_VAL_ON_FAIL(status == GeneralDevice::DecodeError, false);
    }

    return true;
}

bool SerialDeviceTester::TestNegWrongDataSize(void)
{
    RETURN_VAL_ON_FAIL(_initOk, false);
    RETURN_VAL_ON_FAIL(_dut == DUT_SENSX, false);

    m_sensx->ClearRequestQueue();

    GeneralDevice::Status status = GeneralDevice::NotInit;
    for(uint8_t i = 0; i < (WRONGDATA_FUNC_MAXVAL+1); i++)
    {
        switch(i)
        {
            case WRONGDATA_FUNC_RTC_SET:
            {
                // build proper agp frame
                status = m_sensx->RtcSet(0);
                break;
            }
            case WRONGDATA_FUNC_HOST_WAKE_UP_TIME_SET:
            {
                // build proper agp frame
                status = m_sensx->HostWakeTimeSet(0x7080ul);
                break;
            }
            default:
            {
                return false;
            }
        }
        RETURN_VAL_ON_FAIL(status == GeneralDevice::OK, false);

        // reduce the datasize
        const uint8_t dataBytesToRemove = 1;
        ByteVector agpRequest;
        ByteVector agpResponse;
        agpRequest.clear();
        agpResponse.clear();
        int expRespSize = m_sensx->GetNextRequest(agpRequest);
        RETURN_VAL_ON_FAIL(expRespSize > 0, false);
        RETURN_VAL_ON_FAIL(agpRequest.size() > DEVICE_CRC_SIZE, false);
        expRespSize = _sensxErrorRespLen + DEVICE_CRC_SIZE;
        agpRequest.resize(agpRequest.size() - DEVICE_CRC_SIZE - dataBytesToRemove);
        agpRequest.shrink_to_fit();
        uint16_t crc = GeneralDevice::Crc16(agpRequest);
        agpRequest.push_back(static_cast<uint8_t>((crc & 0xFF)));
        agpRequest.push_back(static_cast<uint8_t>(((crc >> 8) & 0xFF)));

        ec_t ec = TriggerComm(agpRequest, agpResponse, static_cast<unsigned int>(expRespSize));
        if(m_serial->isComPortOpened())
        {
            m_serial->CloseComPort();
        }
        RETURN_VAL_ON_FAIL(ec == EC_OK, false);

        //Additional manual check of the status
        status = static_cast<GeneralDevice::Status>(agpResponse.at(0));
        RETURN_VAL_ON_FAIL(status == GeneralDevice::WrongCmdLength, false);
        // standard check
        status = m_sensx->ParseResponse(agpResponse);
        RETURN_VAL_ON_FAIL(status == GeneralDevice::WrongCmdLength, false);
    }

    return true;
}

bool SerialDeviceTester::TestNegIntFrame(void)
{
    RETURN_VAL_ON_FAIL(_initOk, false);
    RETURN_VAL_ON_FAIL(_dut == DUT_SENSX, false);

    GeneralDevice::Status status = GeneralDevice::HwFault;
    ByteVector agpRequest;
    ByteVector agpResponse;
    int expRespSize;

    for(uint8_t variant = 1; variant <= _sensxIntFrameVariants; variant++)
    {
        m_sensx->ClearRequestQueue();
        agpRequest.clear();
        agpResponse.clear();

        switch(variant)
        {
            case 1:
            {
                agpRequest.assign(1, PLIS_END);

                break;
            }
            case 2:
            case 3:
            {
                agpRequest.push_back(_sensxUnknownFunction);
                if(variant == 2)
                {
                    uint16_t crc = GeneralDevice::Crc16(agpRequest);
                    agpRequest.push_back(static_cast<uint8_t>((crc & 0xFF)));
                }

                break;
            }
            case 4:
            case 5:
            {
                status = m_sensx->ReadFwVersion();
                RETURN_VAL_ON_FAIL(status == GeneralDevice::OK, false);

                expRespSize = m_sensx->GetNextRequest(agpRequest);
                RETURN_VAL_ON_FAIL(expRespSize > 0, false);
                RETURN_VAL_ON_FAIL(agpRequest.size() > DEVICE_CRC_SIZE, false);

                agpRequest.erase(agpRequest.end() - 1);
                if(variant == 5)
                {
                    agpRequest.erase(agpRequest.end() - 2);
                }

                break;
            }
            case 6:
            case 7:
            {
                status = m_sensx->ReadDeviceStatus();
                RETURN_VAL_ON_FAIL(status == GeneralDevice::OK, false);

                expRespSize = m_sensx->GetNextRequest(agpRequest);
                RETURN_VAL_ON_FAIL(expRespSize > 0, false);
                RETURN_VAL_ON_FAIL(agpRequest.size() > DEVICE_CRC_SIZE, false);

                agpRequest.erase(agpRequest.end() - 1);
                if(variant == 7)
                {
                    agpRequest.erase(agpRequest.end() - 2);
                }

                break;
            }
            case 8:
            case 9:
            {
                status = m_sensx->MeasurementsRead();
                RETURN_VAL_ON_FAIL(status == GeneralDevice::OK, false);

                expRespSize = m_sensx->GetNextRequest(agpRequest);
                RETURN_VAL_ON_FAIL(expRespSize > 0, false);
                RETURN_VAL_ON_FAIL(agpRequest.size() > DEVICE_CRC_SIZE, false);

                agpRequest.erase(agpRequest.end()- 1);
                if(variant == 9)
                {
                    agpRequest.erase(agpRequest.end() - 2);
                }

                break;
            }
            default:
            {
                break;
            }
        }

        expRespSize = _sensxErrorRespLen + DEVICE_CRC_SIZE;
        ec_t ec = TriggerComm(agpRequest, agpResponse, static_cast<unsigned int>(expRespSize));
        if(m_serial->isComPortOpened())
        {
            m_serial->CloseComPort();
        }
        RETURN_VAL_ON_FAIL(ec == EC_OK, false);

        switch(variant)
        {
            case 1:
            case 2:
            case 3:
            {
                /*
                 * Manual status check, since these agp frames were not created
                 * via m_sensx methods, which means they do not exists in the
                 * m_sensx->reqList and ParseResponse() would fail.
                 */
                status = static_cast<GeneralDevice::Status>(agpResponse.at(0));
                RETURN_VAL_ON_FAIL(status == GeneralDevice::WrongCmdLength, false);

                break;
            }
            default:
            {
                status = m_sensx->ParseResponse(agpResponse);
                RETURN_VAL_ON_FAIL(status == GeneralDevice::WrongCmdLength, false);

                break;
            }
        }
    }

    return true;
}

void SerialDeviceTester::TestMeasStability(void)
{
    RETURN_VOID_ON_FAIL(_initOk);
    RETURN_VOID_ON_FAIL(_dut == DUT_SENSX);

    printf("##### Displaying measurement stability statistics #####\n");

    uint32_t timeout = GetResponseTimeoutMs();
    SetResponseTimeoutMs(_sensxStressTimeoutsMs);

    const uint16_t channels = sizeof(SensX::Measurements) / sizeof(SensX::Measurement);
    printf("Detected RTD temperature channels:\t\t%d\n", channels);
    printf("Number of measurements readings to perform:\t%d\n", _sensxTestMeasStabilityIters);
    printf("Please wait, performing measurement stability test...\n");

    TestMeasurementRead();
    SensX::Measurements measures = m_sensx->MeasurementsGet();
    SensX::Measurement* measChPtr = &(measures.t1);
    std::vector<measstb_t> stbStats;
    stbStats.assign(
            static_cast<size_t>(channels),
            {.minVal = 0, .maxVal = 0, .changeCnt = 0, .maxLocalDeltaVal = 0});

    for(uint16_t i = 0; i < channels; i++, measChPtr++)
    {
        stbStats.at(i).minVal = measChPtr->value;
        stbStats.at(i).maxVal = stbStats.at(i).minVal;
    }

    measChPtr = &(measures.t1);
    for(uint16_t i = 0; i < channels; i++, measChPtr++)
    {
        int32_t deltaVal = 0;
        for(uint16_t j = 0; j < _sensxTestMeasStabilityIters; j++)
        {
            int16_t lastVal = measChPtr->value;
            TestMeasurementRead();
            measures = m_sensx->MeasurementsGet();
            int16_t neuVal = measChPtr->value;
            if(neuVal != lastVal)
            {
                if(neuVal > lastVal)
                {
                    deltaVal = neuVal - lastVal;
                }
                else
                {
                    deltaVal = lastVal - neuVal;
                }

                if(deltaVal > stbStats.at(i).maxLocalDeltaVal)
                {
                    stbStats.at(i).maxLocalDeltaVal = deltaVal;
                }

                stbStats.at(i).changeCnt++;

                if(neuVal > stbStats.at(i).maxVal)
                {
                    stbStats.at(i).maxVal = neuVal;
                }
                else if(neuVal < stbStats.at(i).minVal)
                {
                    stbStats.at(i).minVal = neuVal;
                }
                else
                {
                    // do nothing
                }
            }
        }
    }

    for(uint16_t i = 0; i < channels; i++)
    {
        printf("RTD[%d] temperature value changed:\t\t%d / %d\n",
                i+1, stbStats.at(i).changeCnt, _sensxTestMeasStabilityIters);

        float stbRatio =
                 1.00f -
                 (static_cast<float>(stbStats.at(i).changeCnt) / _sensxTestMeasStabilityIters);

        printf("RTD[%d] stability ratio:\t\t\t\t%.2f%%\n", i+1, stbRatio * 100);

        printf("RTD[%d] max temperature local deviation:\t\t%.2f%cC\n",
                i+1,
                static_cast<float>(stbStats.at(i).maxLocalDeltaVal) / 100,
                eAsciiDegree);
        printf("RTD[%d] temperature deviation range:\t\t%.2f%cC\n",
                i+1,
                static_cast<float>(stbStats.at(i).maxVal - stbStats.at(i).minVal) / 100,
                eAsciiDegree);
    }

    printf("#######################################################\n");

    SetResponseTimeoutMs(timeout);
}

bool SerialDeviceTester::ProcessAgpRequest(void)
{
    RETURN_VAL_ON_FAIL(_initOk, false);
    RETURN_VAL_ON_FAIL(_dut == DUT_SENSX, false); // only sensx supported at the moment

    ByteVector agpRequest;
    ByteVector agpResponse;
    agpRequest.clear();
    agpResponse.clear();

    int expRespSize = m_sensx->GetNextRequest(agpRequest);
    RETURN_VAL_ON_FAIL(expRespSize > 0, false);
    RETURN_VAL_ON_FAIL(agpRequest.size() > 0, false);

    ec_t ec = TriggerComm(agpRequest, agpResponse, static_cast<unsigned int>(expRespSize));
    if(m_serial->isComPortOpened())
    {
        m_serial->CloseComPort();
    }
    RETURN_VAL_ON_FAIL(ec == EC_OK, false);

    GeneralDevice::Status status = m_sensx->ParseResponse(agpResponse);
    RETURN_VAL_ON_FAIL(status == GeneralDevice::OK, false);

    return true;
}

ec_t SerialDeviceTester::TriggerComm(
        ByteVector& request,
        ByteVector& response,
        unsigned int expRespSize,
        plis_en_t plisEnable)
{
    RETURN_VAL_ON_FAIL(_initOk, EC_FAIL);
    RETURN_VAL_ON_FAIL(((request.size() > 0) && (response.size() == 0)), EC_FAIL);

    bool plisEncode = static_cast<bool>(plisEnable & PLIS_ENCODE);
    bool plisDecode = static_cast<bool>(plisEnable & PLIS_DECODE);

    ec_t ec = m_serial->OpenComPort();
    RETURN_EC_ON_ERROR(ec);
    ec = m_serial->FlushRX();
    RETURN_EC_ON_ERROR(ec);

    if(plisEncode)
    {
        Plis::encode(request);
    }

    int bts_sent =
            m_serial->DumpBuffToPort(request.data(), static_cast<unsigned int>(request.size()));
    RETURN_VAL_ON_FAIL(
            static_cast<unsigned int>(bts_sent) == static_cast<unsigned int>(request.size()),
            EC_FAIL);

    int maxRespSize = static_cast<int>(expRespSize);
    if(plisDecode)
    {
        maxRespSize = Plis::overhead(static_cast<int>(expRespSize));
    }
    response.assign(static_cast<size_t>(maxRespSize), 0);

    Sleep(_responseTimeoutMs);

    int bts_read =
            m_serial->ReadDataFromPort(
                    response.data(),
                    static_cast<unsigned int>(maxRespSize));
    if(expRespSize > 0)
    {
        RETURN_VAL_ON_FAIL(bts_read > 0, EC_FAIL);
    }
    RETURN_VAL_ON_FAIL(bts_read <= maxRespSize, EC_FAIL);

    response.resize(static_cast<size_t>(bts_read));
    response.shrink_to_fit();
    if(plisDecode)
    {
        Plis::decode(response);
    }
    if(expRespSize > 0)
    {
        RETURN_VAL_ON_FAIL(response.size() > 0, EC_FAIL);
    }
    RETURN_VAL_ON_FAIL(response.size() == expRespSize, EC_FAIL);

    m_serial->CloseComPort();
    return EC_OK;
}

void SerialDeviceTester::DispDutFields(void)
{
    RETURN_VOID_ON_FAIL(_initOk);

    printf("############## Displaying DUT summary #################\n");
    DispDutInfo();
    DispDutMeasurements();
    printf("#######################################################\n");
}

void SerialDeviceTester::DispDutInfo(void)
{
    RETURN_VOID_ON_FAIL(_initOk);
    RETURN_VOID_ON_FAIL(_dut == DUT_SENSX);

    GeneralDevice::Info info = m_sensx->GetInfo();

    printf("DeviceStatus is: \t\t\t\t");
    switch(info.status)
    {
        case GeneralDevice::App:
        {
            printf("App\n");
            break;
        }
        case GeneralDevice::Bootloader:
        {
            printf("BootLoader\n");
            break;
        }
        case GeneralDevice::NoFirmware:
        {
            printf("NoFirmware\n");
            break;
        }
        case GeneralDevice::Unknown:
        {
            printf("Unknown\n");
            break;
        }
        case GeneralDevice::Error:
        {
            printf("Error\n");
            break;
        }
        default:
        {
            printf("<N/A>\n");
            break;
        }
    }

    printf("fwVersion is:\t\t\t\t\t%d.%d.%d\n",
            info.fwVersion.major, info.fwVersion.minor, info.fwVersion.patch);
}

void SerialDeviceTester::DispDutMeasurements(void)
{
    RETURN_VOID_ON_FAIL(_initOk);
    RETURN_VOID_ON_FAIL(_dut == DUT_SENSX);

    SensX::Measurements measures = m_sensx->MeasurementsGet();
    SensX::Measurement* measPtr = &(measures.t1);
    const uint16_t channels = sizeof(SensX::Measurements) / sizeof(SensX::Measurement);
    printf("Detected RTD temperature channels:\t\t%d\n", channels);
    for(uint16_t i = 0; i < channels; i++, measPtr++)
    {
        printf("RTD[%d] status is: \t\t\t\t", (i+1));
        switch(measPtr->status)
        {
            case SensX::RTD_OK:
            {
                printf("RTD_OK\n");
                break;
            }
            case SensX::RTD_SHORTED:
            {
                printf("RTD_SHORTED\n");
                break;
            }
            case SensX::RTD_DISCON:
            {
                printf("RTD_DISCON\n");
                break;
            }
            default:
            {
                printf("<N/A>\n");
                break;
            }
        }
        /*
         * note to self:
         * float can accurately represent int if int value does not exceed 2^24 - 1
         * that's because float mantissa is 24-bit. Above this value
         * float makes rounds up/down of the total part.
         */
        printf("RTD[%d] temperature is:\t\t\t\t%.2f%cC\n",
                (i+1),
                static_cast<float>(measPtr->value) / 100,
                eAsciiDegree);
    }
}

void SerialDeviceTester::DispPreciseData2Pts(int data, bool noneuline)
{
    bool isNegative = data < 0 ? true : false;
    uint32_t adata = static_cast<uint32_t>(abs(data));
    uint32_t hVal = adata / 100;
    uint32_t lVal = adata % 100;
    if(isNegative)
    {
        printf("-");
    }
    printf("%d,", hVal);
    if(lVal < 10)
    {
        printf("0");
    }
    printf("%d", lVal);
    if(!noneuline)
    {
        printf("\n");
    }
}
