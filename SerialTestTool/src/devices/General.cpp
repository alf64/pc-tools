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

#include <devices/General.hpp>
#include <string.h>

using namespace std;

#define GENERAL_TIMEOUT 500
#define RES_RESET_LENGTH 1
#define RES_FW_VERSION_LENGTH 4
#define RWS_DEVICE_STATUS_LENGTH 2

namespace alf64
{
namespace devices
{
GeneralDevice::GeneralDevice(void):
        initOk(false)
{
    if(Init() == EC_OK)
    {
        initOk = true;
    }
}

GeneralDevice::~GeneralDevice(void)
{
    return;
}

ec_t GeneralDevice::Init(void)
{
    info.status = Unknown;
    info.fwVersion.major = 0;
    info.fwVersion.minor = 0;
    info.fwVersion.patch = 0;
    reqList.clear();

    return EC_OK;
}

GeneralDevice::Info GeneralDevice::GetInfo(void)
{
    return info;
}

GeneralDevice::Status GeneralDevice::ForceReset(const bool fwUpg)
{
    RETURN_VAL_ON_FAIL(initOk, NotInit);

    Status status = OK;
    AgpMessage msg;

    if (fwUpg)
    {
        msg.function = SetForceFwUpgrade;
    }
    else
    {
        msg.function = SetReset;
    }
    msg.data.clear();

    status = AddQueueRequest(msg);

    if (status == OK)
    {
        info.status = Unknown;
    }

    return status;
}

GeneralDevice::Status GeneralDevice::ReadFwVersion(void)
{
    RETURN_VAL_ON_FAIL(initOk, NotInit);

    AgpMessage msg;

    msg.function = GetFwVersion;
    msg.data.clear();

    return AddQueueRequest(msg);
}

GeneralDevice::Status GeneralDevice::ReadDeviceStatus(void)
{
    RETURN_VAL_ON_FAIL(initOk, NotInit);

    AgpMessage msg;

    msg.function = GetDeviceStatus;
    msg.data.clear();

    return AddQueueRequest(msg);
}

GeneralDevice::Status GeneralDevice::AddQueueRequest(AgpMessage &msg)
{
    RETURN_VAL_ON_FAIL(initOk, NotInit);

    Status status = ValidateRequest(msg);

    if (status == OK)
    {
        Request req;
        req.data = msg.data;
        req.resLength = GetResLength(msg.data[0]);
        reqList.push_back(req);
    }

    return status;
}

void GeneralDevice::ClearRequestQueue(void)
{
    reqList.clear();
}

int GeneralDevice::GetResLength(const uint8_t funct)
{
    RETURN_VAL_ON_FAIL(initOk, -1);

    int resLength = 0;

    switch (static_cast<CommonFunctions>(funct))
    {
        case SetReset:
        case SetForceFwUpgrade:
        {
            resLength = RES_RESET_LENGTH + DEVICE_CRC_SIZE;
            break;
        }

        case GetFwVersion:
        {
            resLength = RES_FW_VERSION_LENGTH + DEVICE_CRC_SIZE;
            break;
        }

        case GetDeviceStatus:
        {
            resLength = RWS_DEVICE_STATUS_LENGTH + DEVICE_CRC_SIZE;
            break;
        }

        default:
        {
            resLength = GetSpecificResLength(funct);
            break;
        }
    }

    return resLength;
}

int GeneralDevice::GetNextRequest(ByteVector &data)
{
    RETURN_VAL_ON_FAIL(initOk, -1);

    int resLength = 0;

    if (reqList.empty())
    {
        resLength = EmptyReqListMsg(data);
    }
    else
    {
        data = reqList.front().data;
        resLength = reqList.front().resLength;
    }

    return resLength;
}

GeneralDevice::Status GeneralDevice::ValidateRequest(AgpMessage &msg)
{
    RETURN_VAL_ON_FAIL(initOk, NotInit);

    Status status = OK;

    switch (static_cast<CommonFunctions>(msg.function))
    {
        case SetReset:
        case SetForceFwUpgrade:
        case GetFwVersion:
        case GetDeviceStatus:
        {
            if (msg.data.size() > 0)
            {
                status = InvalidArg;
            }
            break;
        }

        default:
        {
            status = ValidateSpecificRequest(msg);
            break;
        }
    }

    if (status == OK)
    {
        msg.data.emplace(msg.data.begin(), msg.function);
        uint16_t crc = Crc16(msg.data);
        msg.data.push_back(static_cast<uint8_t>((crc & 0xFF)));
        msg.data.push_back(static_cast<uint8_t>(((crc >> 8) & 0xFF)));
    }

    return status;
}

GeneralDevice::Status GeneralDevice::ParseResponse(ByteVector &data)
{
    RETURN_VAL_ON_FAIL(initOk, NotInit);
    RETURN_VAL_ON_FAIL(reqList.size() > 0, InvalidArg);
    RETURN_VAL_ON_FAIL((data.size() >= (DEVICE_STATUS_SIZE + DEVICE_CRC_SIZE)), InvalidArg);

    uint16_t crc_hi = static_cast<uint16_t>(data.at(data.size() - 1) << 8);
    uint16_t crc_lo = static_cast<uint16_t>(data.at(data.size() - 2)) & 0xFF;
    uint16_t crc = crc_hi | crc_lo;
    data.resize(data.size() - DEVICE_CRC_SIZE);
    data.shrink_to_fit();

    if (crc != Crc16(data))
    {
        return CrcError;
    }

    Status status = static_cast<Status>(data.at(0));
    CommonFunctions funct = static_cast<CommonFunctions>(reqList.front().data.at(0));

    switch (funct)
    {
        case SetReset:
        {
            if (status == OK)
            {
                if (data.size() != RES_RESET_LENGTH)
                {
                    status = WrongCmdLength;
                }
            }
            break;
        }

        case SetForceFwUpgrade:
        {
            if (status == OK)
            {
                if (data.size() != RES_RESET_LENGTH)
                {
                    status = WrongCmdLength;
                }
            }
            break;
        }

        case GetFwVersion:
        {
            if (status == OK)
            {
                if (data.size() != RES_FW_VERSION_LENGTH)
                {
                    status = WrongCmdLength;
                }
                else
                {
                    memcpy(&info.fwVersion, &(data.data()[1]), sizeof(info.fwVersion));
                }
            }
            break;
        }

        case GetDeviceStatus:
        {
            if (status == OK)
            {
                if (data.size() != RWS_DEVICE_STATUS_LENGTH)
                {
                    status = WrongCmdLength;
                }
                else
                {
                    info.status = static_cast<DeviceStatus>(data.at(1));
                }
            }
            break;
        }

        default:
        {
            status = ParseSpecificResponse(funct, data);
            break;
        }
    }

    if (status != Busy)
    {
        reqList.erase(reqList.begin());
    }

    return status;
}

int GeneralDevice::EmptyReqListMsg(ByteVector &data)
{
    RETURN_VAL_ON_FAIL(initOk, -1);
    data.clear();
    return 0;
}

uint16_t GeneralDevice::Crc16(const ByteVector &data)
{
    unsigned int length = static_cast<unsigned int>(data.size());

    uint16_t crc = 0xffff;
    const unsigned char *pData = data.data();

    while (length--)
    {
        crc = static_cast<uint16_t>(crc ^ (*pData++ << 8));

        for (uint8_t i = 0; i < 8; i++)
        {
            crc = static_cast<uint16_t>(crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1);
        }
    }

    return static_cast<uint16_t>(REVERSE_U16_BYTE(crc));
}

} // namespace devices
} //namespace alf64
