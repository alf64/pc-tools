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

#ifndef SERIALDEVICETESTER_HPP_
#define SERIALDEVICETESTER_HPP_

#include <devices/SensX.hpp>
#include <serial/Plis.hpp>
#include <iostream>
#include "serial/Serial.hpp"


using namespace alf64::devices;

class SerialDeviceTester
{
public:
    /*
     * Supported DUTs (Device(s) Under Test)
     */
    typedef enum
    {
        DUT_SENSX = 0
    }dut_t;

    //!< Extended Ascii 'degree' character.
    static const unsigned char eAsciiDegree = 248;

    alf64::devices::SensX* m_sensx;
    Serial* m_serial;

    SerialDeviceTester();
    ~SerialDeviceTester();

    ec_t SetDut(dut_t dut);
    dut_t GetDut(void);
    void SetResponseTimeoutMs(uint32_t timeoutMs);
    uint32_t GetResponseTimeoutMs(void);

    /*
     * Tests implemented PLIS functionality by calling:
     *  - PlisTestDecodeByte()
     *  - PlisTestDecodeByte()
     * Returns true if test passes.
     * Returns false if test fails.
     */
    bool TestSelfPlis(void);

    //!< Performs FwVersionRead functionality test.
    bool TestFwVersionRead(void);

    //!< Performs DeviceStatusRead functionality test.
    bool TestDeviceStatusRead(void);

    //!< Performs MeasurementRead functionality test.
    bool TestMeasurementRead(void);

    //!< Performs RTCSet functionality test.
    bool TestRtcSet(void);

    //!< Performs HostSleepSet functionality test.
    bool TestHostWakeUpTimeSet(void);

    //!< Performs functionality tests in a stressful manner.
    bool TestFuncStress(void);

    //!< Performs negative test: agp unknown function send.
    bool TestNegUnknownFunc(void);

    //!< Performs negative test: wrong crc send.
    bool TestNegWrongCrc(void);

    //!< Performs negative test: wrong plis encoded send.
    bool TestNegWrongPlis(void);

    //!< Performs negative test: a function with wrong data size send.
    bool TestNegWrongDataSize(void);

    //!< Performs negative test: interrupted (not complete) agp frame send.
    bool TestNegIntFrame(void);

    //!< Performs measurements stability check.
    void TestMeasStability(void);

    //!< Displays device fields (fw version, status, measurements).
    void DispDutFields(void);

private:
    bool _initOk;

    typedef enum
    {
        PLIS_DISABLED = 0,
        PLIS_ENCODE = 1,
        PLIS_DECODE = 2,
        PLIS_ENCODE_DECODE = 3
    }plis_en_t;

    typedef struct
    {
        int16_t minVal;
        int16_t maxVal;
        uint16_t changeCnt;
        int32_t maxLocalDeltaVal;
    }measstb_t;

    typedef enum
    {
        WRONGDATA_FUNC_RTC_SET = 0,
        WRONGDATA_FUNC_HOST_WAKE_UP_TIME_SET = 1,
        WRONGDATA_FUNC_MAXVAL = WRONGDATA_FUNC_HOST_WAKE_UP_TIME_SET
    }wrongdata_func_t;

    static const uint32_t _sensxTimeoutMs = 100;
    static const uint32_t _sensxStressTimeoutsMs = 60;
    static const uint32_t _sensxTestFuncStressFrames = 1000;
    static const uint8_t _sensxTestFuncStressVariants = 4;
    static const uint8_t _sensxUnknownFunction = 0x80;
    static const uint8_t _sensxErrorRespLen = 1;
    static const uint16_t _sensxWrongCrc = 0xEDAA;
    static const uint16_t _sensxTestMeasStabilityIters = 150;
    static constexpr const uint8_t _sensxWrongPlisAgpFrame1[] = {
            PLIS_ESC, 0xAA, PLIS_END
    };
    static constexpr const uint8_t _sensxWrongPlisAgpFrame2[] = {
            0xAA, PLIS_ESC, PLIS_END
    };
    static constexpr const uint8_t _sensxWrongPlisAgpFrame3[] = {
            0xAA, 0xAB, 0xAC, 0xAD, 0xAE
    };
    static const uint8_t _sensxIntFrameVariants = 9;
    static constexpr const uint32_t _sensxRtcSetTestValues[] = {
            0xEDul, 0xFFCAul, 0xFFFFFFul, 0xABBACDEF, 0xDDDDDDDD
    };
    static const uint32_t _sensxHostWakeUpTimeInvalid = 0;
    static constexpr const uint32_t _sensxHostWakeUpTimeSetTestValues[] = {
            0xACDFul, 0xFFFFFFFF, 0xCACAul, 0xFEDCBAAB, _sensxHostWakeUpTimeInvalid
    };

    dut_t _dut;

    //!< The time (in milliseconds) of waiting for agp response.
    uint32_t _responseTimeoutMs;

    ec_t Init(void);

    bool ProcessAgpRequest(void);

    /*
     * @brief Triggers communication over serial port.
     *
     * @details Sends request and gets response.
     * This function handles PLIS
     * (performs PLIS encode on request and decode on response).
     *
     * @attention
     * This function opens and closes com port.
     * However if it returns with error the port is not being closed.
     * User is obligated to check if com port is still opened after returning
     * from this function and close it if needed.
     *
     * @param request A reference to prepared AGP request to send.
     * @param response A reference to a place where response will be created by this function.
     * This parameter must be uninitialized (reponse.size() == 0)
     * @param expRespSize Expected size (in bytes) for response.
     * Function will return error if obtained response size is different than this parameter.
     * @param plisEnable Enables/disables Plis encoding for request and Plis decoding for response.
     *
     * @returns ec_t
     * @retval EC_OK If succeeded to send request and obtain response.
     * @retval EC_FAIL If failed.
     */
    ec_t TriggerComm(
            ByteVector& request,
            ByteVector& response,
            unsigned int expRespSize,
            plis_en_t plisEnable = PLIS_ENCODE_DECODE);

    //!< Displays info about Dut (its Info field)
    void DispDutInfo(void);

    //!< Displays measurements fields of Dut
    void DispDutMeasurements(void);

    /*
     * @brief Displays integer data as float with 2 digit precision.
     *
     * @details
     * Displays integer data as data with 2 digit precision upon decimal point.
     *
     * @param data Data to be displayed.
     * @param noneuline If true there will be no newline character put.
     *
     * @returns void
     * Function will not display anything if error condition occurs.
     */
    static void DispPreciseData2Pts(int data, bool noneuline=true);
};


#endif /* SERIALDEVICETESTER_HPP_ */
