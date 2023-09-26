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

#include <devices/SensX.hpp>
using namespace std;

#define RES_MEASUREMENT_READ_LENGTH \
    (DEVICE_STATUS_SIZE + sizeof(MeasurementData) + DEVICE_CRC_SIZE)
#define RES_RTC_SET_LENGTH (DEVICE_STATUS_SIZE + DEVICE_CRC_SIZE)
#define RES_HOST_WAKE_UP_TIME_SET_LENGTH (DEVICE_STATUS_SIZE + DEVICE_CRC_SIZE)

namespace alf64
{
namespace devices
{
SensX::SensX(void):
        GeneralDevice(),
        initOk(false)
{
    if(Init() == EC_OK)
    {
        initOk = true;
    }
}

SensX::~SensX(void)
{
    return;
}

ec_t SensX::Init(void)
{
    emptyReq.data.clear();
    emptyReq.data.emplace(emptyReq.data.begin(), MeasurementRead);
    uint16_t crc = Crc16(emptyReq.data);
    emptyReq.data.push_back(static_cast<uint8_t>((crc & 0xFF)));
    emptyReq.data.push_back(static_cast<uint8_t>(((crc >> 8) & 0xFF)));
    emptyReq.resLength = GetResLength(MeasurementRead);

    return EC_OK;
}

void SensX::PushU32ToByteVector(ByteVector &vector, uint32_t u32)
{
    uint8_t* value_ptr = reinterpret_cast<uint8_t*>(&u32);
    for(uint8_t i = 0; i < sizeof(u32); i++)
    {
        vector.push_back(*value_ptr++);
    }
}

GeneralDevice::Status SensX::MeasurementsRead()
{
    RETURN_VAL_ON_FAIL(initOk, NotInit);

    AgpMessage msg;

    msg.function = MeasurementRead;
    msg.data.clear();

    return AddQueueRequest(msg);
}

SensX::Measurements SensX::MeasurementsGet()
{
    return measurements;
}

GeneralDevice::Status SensX::MeasurementsGetConfig()
{
    RETURN_VAL_ON_FAIL(initOk, NotInit);

    AgpMessage msg;

    msg.function = MeasurementGetConfig;
    msg.data.clear();

    return AddQueueRequest(msg);
}

GeneralDevice::Status SensX::MeasurementsSetInterval(uint16_t interval)
{
    RETURN_VAL_ON_FAIL(initOk, NotInit);

    AgpMessage msg;

    msg.function = MeasurementSetInterval;
    msg.data.clear();
    emptyReq.data.push_back(static_cast<uint8_t>((interval & 0xFF)));
    emptyReq.data.push_back(static_cast<uint8_t>(((interval >> 8) & 0xFF)));

    return AddQueueRequest(msg);
}

GeneralDevice::Status SensX::MeasurementsSetAlarms(MeasurementsAlarm &alarms)
{
    RETURN_VAL_ON_FAIL(initOk, NotInit);

    AgpMessage msg;

    msg.function = MeasurementSetAlarms;
    msg.data.clear();
    for(uint16_t i = 0; i < sizeof(alarms); i++)
    {
        uint8_t* alarms_ptr = reinterpret_cast<uint8_t*>(&alarms);
        msg.data.push_back(*alarms_ptr);
        alarms_ptr++;
    }

    return AddQueueRequest(msg);
}

GeneralDevice::Status SensX::RtcSet(uint32_t value)
{
    RETURN_VAL_ON_FAIL(initOk, NotInit);

    AgpMessage msg;

    msg.function = RTCSet;
    msg.data.clear();
    uint32_t revval = REVERSE_U32_BYTE(value);
    PushU32ToByteVector(msg.data, revval);

    return AddQueueRequest(msg);
}

GeneralDevice::Status SensX::HostWakeTimeSet(uint32_t value)
{
    RETURN_VAL_ON_FAIL(initOk, NotInit);

    AgpMessage msg;

    msg.function = HostWakeUpTimeSet;
    msg.data.clear();
    uint32_t revval = REVERSE_U32_BYTE(value);
    PushU32ToByteVector(msg.data, revval);

    return AddQueueRequest(msg);
}

GeneralDevice::Status SensX::HostSetConfig(HostConfig &config)
{
    RETURN_VAL_ON_FAIL(initOk, NotInit);

    GeneralDevice::Status status;
    AgpMessage msg;

    msg.function = HostSetSleep;
    msg.data.clear();
    for(uint8_t i = 0; i < sizeof(config.sleep); i++)
    {
        uint8_t* sleep_ptr = reinterpret_cast<uint8_t*>(&(config.sleep));
        msg.data.push_back(*sleep_ptr);
        sleep_ptr++;
    }

    status = AddQueueRequest(msg);

    if (status == OK)
    {
        msg.function = HostSetIrqOutConf;
        msg.data.clear();
        msg.data.push_back(config.irqOut);

        status = AddQueueRequest(msg);
    }

    if (status == OK)
    {
        msg.function = HostSetIrqInConf;
        msg.data.clear();
        msg.data.push_back(config.irqIn);

        status = AddQueueRequest(msg);
    }

    return status;
}

GeneralDevice::Status SensX::HostSetStatus(uint8_t status)
{
    RETURN_VAL_ON_FAIL(initOk, NotInit);

    AgpMessage msg;

    msg.function = HostSetStat;
    msg.data.clear();
    msg.data.push_back(status);

    return AddQueueRequest(msg);
}

GeneralDevice::Status SensX::LogRead(LogRange &range)
{
    RETURN_VAL_ON_FAIL(initOk, NotInit);

    AgpMessage msg;

    msg.function = LogReadRange;
    msg.data.clear();
    for(uint8_t i = 0; i < sizeof(range); i++)
    {
        uint8_t* range_ptr = reinterpret_cast<uint8_t*>(&range);
        msg.data.push_back(*range_ptr);
        range_ptr++;
    }

    return AddQueueRequest(msg);
}

GeneralDevice::Status SensX::ValidateSpecificRequest(const AgpMessage &msg)
{
    RETURN_VAL_ON_FAIL(initOk, NotInit);

    Status status = OK;

    switch (static_cast<SensXCmd>(msg.function))
    {
        case MeasurementRead:
        case MeasurementGetConfig:
        case RswGetQueueFreeSpace:
        case RswReadRes:
        {
            if (msg.data.size() != 0)
            {
                status = InvalidArg;
            }
            break;
        }

        case RTCSet:
        case HostWakeUpTimeSet:
        {
            if (msg.data.size() != 4)
            {
                status = InvalidArg;
            }
            break;
        }

        case MeasurementSetInterval:
        {
            const uint16_t *interval = reinterpret_cast<const uint16_t*>(msg.data.data());

            if (*interval > SENSX_MAX_MEASUREMENT_INTERVAL)
            {
                status = InvalidArg;
            }
            break;
        }

        case MeasurementSetAlarms:
        {
            const MeasurementsAlarm *config = reinterpret_cast<const MeasurementsAlarm*>(msg.data.data());

            if (msg.data.size() != sizeof(MeasurementsAlarm)
                || (config->lowT1.timeout < SENSX_MEASUREMENT_ALARM_TIMEOUT_OFF
                    && config->highT1.timeout < SENSX_MEASUREMENT_ALARM_TIMEOUT_OFF
                    && config->lowT1.value >= config->highT1.value)
                || (config->lowT2.timeout < SENSX_MEASUREMENT_ALARM_TIMEOUT_OFF
                    && config->highT2.timeout < SENSX_MEASUREMENT_ALARM_TIMEOUT_OFF
                    && config->lowT2.value >= config->highT2.value))
            {
                status = InvalidArg;
            }
            break;
        }

        case HostSetSleep:
        {
            const HostSleep *config = reinterpret_cast<const HostSleep*>(msg.data.data());

            if (msg.data.size() != sizeof(HostSleep) || config->mask > SENSX_HOST_WAKEUP_TRIGG_MASK
                || (config->mask & SENSX_HOST_WAKEUP_TRIGG_PERIODIC
                    && (config->period < SENSX_MIN_HOST_SLEEP_TIME
                        || config->period > SENSX_MAX_HOST_SLEEP_TIME)))
            {
                status = InvalidArg;
            }
            break;
        }

        case HostSetStat:
        {
            if (msg.data.size() != 1)
            {
                status = InvalidArg;
            }
            break;
        }

        case HostSetIrqOutConf:
        {
            const uint8_t *config = msg.data.data();

            if (msg.data.size() != 1 && *config > SENSX_HOST_IRQ_OUT_MASK)
            {
                status = InvalidArg;
            }
            break;
        }

        case HostSetIrqInConf:
        {
            const uint8_t *config = msg.data.data();

            if (msg.data.size() != 1 && *config > SENSX_HOST_IRQ_IN_MASK)
            {
                status = InvalidArg;
            }
            break;
        }

        case RswAddReq:
        {
            break;
        }

        case LogReadRange:
        {
            const LogRange *range = reinterpret_cast<const LogRange*>(msg.data.data());
            if (msg.data.size() != sizeof(LogRange) || range->end <= range->begin)
            {
                status = InvalidArg;
            }
            break;
        }

        default:
        {
            status = UnsupportedCmd;
        }
    }

    return status;
}

GeneralDevice::Status SensX::ParseSpecificResponse(const uint8_t funct, const ByteVector &data)
{
    RETURN_VAL_ON_FAIL(initOk, NotInit);

    Status status = static_cast<Status>(data.at(0));
    uint16_t length = static_cast<uint16_t>(data.size() - 1);

    switch (static_cast<SensXCmd>(funct))
    {
        case MeasurementRead:
        {
            if (status == OK)
            {
                if (length != sizeof(MeasurementData))
                {
                    status = WrongCmdLength;
                }
                else
                {
                    const MeasurementData *read = reinterpret_cast<const MeasurementData*>(&(data.data()[1]));
                    int16_t t1 =
                            static_cast<int16_t>(REVERSE_U16_BYTE(static_cast<uint16_t>(read->t1)));
                    int16_t t2 =
                            static_cast<int16_t>(REVERSE_U16_BYTE(static_cast<uint16_t>(read->t2)));

                    if (measurements.t1.value != t1)
                    {
                        measurements.t1.value = t1;

                        MeasurementStatus status = ParseMeasurementStatus(t1);

                        if (measurements.t1.status != status)
                        {
                            measurements.t1.status = status;
                        }
                    }

                    if (measurements.t2.value != t2)
                    {
                        measurements.t2.value = t2;

                        MeasurementStatus status = ParseMeasurementStatus(t2);

                        if (measurements.t2.status != status)
                        {
                            measurements.t2.status = status;
                        }
                    }
                }
            }
            break;
        }

        case HostWakeUpTimeSet:
        case RTCSet:
        {
            if(length != 0)
            {
                status = WrongCmdLength;
            }

            break;
        }

        default:
        {
            status = UnsupportedCmd;
        }
    }

    return status;
}

int SensX::GetSpecificResLength(const uint8_t funct)
{
    RETURN_VAL_ON_FAIL(initOk, -1);

    int resLength = 0;

    switch (static_cast<SensXCmd>(funct))
    {
        case MeasurementRead:
        {
            resLength = RES_MEASUREMENT_READ_LENGTH;
            break;
        }

        case HostWakeUpTimeSet:
        {
            resLength = RES_HOST_WAKE_UP_TIME_SET_LENGTH;
            break;
        }

        case RTCSet:
        {
            resLength = RES_RTC_SET_LENGTH;
            break;
        }

        default:
        {
            break;
        }
    }

    return resLength;
}

int SensX::EmptyReqListMsg(ByteVector &data)
{
    data = emptyReq.data;
    return emptyReq.resLength;
}

SensX::MeasurementStatus SensX::ParseMeasurementStatus(int16_t value)
{
    if (value == SENSX_MIN_MEASUREMENT_SHORTED)
    {
        return RTD_SHORTED;
    }

    if (value == SENSX_MIN_MEASUREMENT_DISCONECTED)
    {
        return RTD_DISCON;
    }

    return RTD_OK;
}

} // namespace devices
} // namespace alf64
