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

#ifndef DEVICES_SENSX_HPP_
#define DEVICES_SENSX_HPP_

#include <devices/General.hpp>
#include <iostream>

#include "ec.h"

namespace alf64
{
namespace devices
{
#define SENSX_MIN_MEASUREMENT_SHORTED INT16_MIN
#define SENSX_MIN_MEASUREMENT_DISCONECTED INT16_MAX
#define SENSX_MIN_MEASUREMENT_INTERVAL 0
#define SENSX_MAX_MEASUREMENT_INTERVAL 43200 // s = 12h

#define SENSX_MEASUREMENT_ALARM_TIMEOUT_OFF 0xFFFF

#define SENSX_HOST_WAKEUP_TRIGG_PERIODIC 0x01
#define SENSX_HOST_WAKEUP_TRIGG_MEASUREMET_ALARM 0x02
#define SENSX_HOST_WAKEUP_TRIGG_SCREEN_TOUCH 0x04
#define SENSX_HOST_WAKEUP_TRIGG_MASK 0x07

#define SENSX_MIN_HOST_SLEEP_TIME 1
#define SENSX_MAX_HOST_SLEEP_TIME 43200 // s = 12h

#define SENSX_HOST_IRQ_OUT_MEASUREMENT_READY 0x01
#define SENSX_HOST_IRQ_OUT_MEASUREMENT_ALARM 0x02
#define SENSX_HOST_IRQ_OUT_RSW_EMPTY 0x04
#define SENSX_HOST_IRQ_OUT_RSW_RES_READY 0x10
#define SENSX_HOST_IRQ_OUT_MASK 0x1F

#define SENSX_HOST_IRQ_IN_TRIGG_MEASUREMENT 0x01
#define SENSX_HOST_IRQ_IN_TRIGG_RESET 0x02
#define SENSX_HOST_IRQ_IN_MASK 0x03

class SensX: public GeneralDevice
{
public:
    enum MeasurementStatus
    {
        RTD_OK,
        RTD_SHORTED,
        RTD_DISCON
    };

    struct Measurement
    {
        int16_t value;
        MeasurementStatus status;
    } __attribute__((packed));

    struct Measurements
    {
        Measurement t1;
        Measurement t2;
    } __attribute__((packed));

    struct MeasurementAlarm
    {
        uint16_t timeout;
        int16_t value;
    } __attribute__((packed));

    struct MeasurementsAlarm
    {
        MeasurementAlarm lowT1;
        MeasurementAlarm highT1;
        MeasurementAlarm lowT2;
        MeasurementAlarm highT2;
    } __attribute__((packed));

    struct HostSleep
    {
        uint8_t mask;
        uint16_t period;
    } __attribute__((packed));

    struct HostConfig
    {
        HostSleep sleep;
        uint8_t irqIn;
        uint8_t irqOut;
    } __attribute__((packed));

    struct LogRange
    {
        uint32_t begin;
        uint32_t end;
    } __attribute__((packed));

    SensX(void);
    ~SensX(void);

    GeneralDevice::Status MeasurementsRead();
    Measurements MeasurementsGet();
    GeneralDevice::Status MeasurementsGetConfig();
    GeneralDevice::Status MeasurementsSetInterval(uint16_t interval);
    GeneralDevice::Status MeasurementsSetAlarms(MeasurementsAlarm &alarms);

    GeneralDevice::Status RtcSet(uint32_t value);
    GeneralDevice::Status HostWakeTimeSet(uint32_t value);

    GeneralDevice::Status HostSetConfig(HostConfig &config);
    GeneralDevice::Status HostSetStatus(uint8_t status);

    GeneralDevice::Status LogRead(LogRange &range);

protected:
    GeneralDevice::Status ValidateSpecificRequest(const AgpMessage &msg) override;
    GeneralDevice::Status ParseSpecificResponse(
        const uint8_t funct, const ByteVector &data) override;
    int GetSpecificResLength(const uint8_t funct) override;
    int EmptyReqListMsg(ByteVector &data) override;

private:
    enum SensXCmd
    {
        MeasurementRead = 0x00,
        MeasurementGetConfig,
        MeasurementSetInterval,
        MeasurementSetAlarms,

        RTCSet = 0x10,
        HostWakeUpTimeSet = 0x21,

        HostSetSleep = 0x2A,
        HostSetStat,
        HostSetIrqOutConf,
        HostSetIrqInConf,

        RswGetQueueFreeSpace = 0x30,
        RswAddReq,
        RswReadRes,

        LogReadRange = 0x40,
    } __attribute__((packed));

    struct MeasurementData
    {
        int16_t t1;
        int16_t t2;
    } __attribute__((packed));

    static MeasurementStatus ParseMeasurementStatus(int16_t value);

    Request emptyReq;

    Measurements measurements;

    bool initOk;
    ec_t Init(void);
    static void PushU32ToByteVector(ByteVector &vector, uint32_t u32);
};
} // namespace devices
} // namespace alf64



#endif /* DEVICES_SENSX_HPP_ */
