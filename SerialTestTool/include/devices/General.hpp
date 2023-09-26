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

#ifndef DEVICES_GENERAL_HPP_
#define DEVICES_GENERAL_HPP_

#include <iostream>
#include <vector>
#include <list>

#include "ec.h"

//!< Size (in bytes) of Status field.
#define DEVICE_STATUS_SIZE 1
//!< Size (in bytes) of CRC field.
#define DEVICE_CRC_SIZE 2
//!< Reverses order of bytes in u16 variable.
#define REVERSE_U16_BYTE(u16) ((u16 >> 8) | (u16 << 8))
//!< Reverses order of bytes in u32 variable.
#define REVERSE_U32_BYTE(u32) \
    ((u32 >> 24) | (u32 << 24) | ((u32 >> 8) & 0xFF00ul) | ((u32 << 8) & 0xFF0000ul))

namespace alf64
{
namespace devices
{
typedef std::vector<uint8_t> ByteVector;

class GeneralDevice
{
public:
    enum Status
    {
        OK,
        HwFault,
        Busy,
        WrongCmd,
        WrongCmdLength,
        UnsupportedCmd,
        InvalidArg,
        CrcError,
        NotInit,
        DecodeError
    };

    enum DeviceStatus
    {
        App,
        Bootloader,
        NoFirmware,
        Unknown,
        Error,
    };

    enum CommonFunctions
    {
        SetForceFwUpgrade = 0xFC,
        SetReset = 0xFD,
        GetFwVersion = 0xFE,
        GetDeviceStatus = 0xFF,
    };

    typedef struct
    {
        struct
        {
            uint8_t major;
            uint8_t minor;
            uint8_t patch;
        } fwVersion;
        enum DeviceStatus status;
    } Info;

    typedef struct
    {
        uint8_t function;
        ByteVector data;
    } AgpMessage;

    GeneralDevice(void);
    virtual ~GeneralDevice(void); // must be virtual since we have virtual funcs

    Info GetInfo(void);

    Status ForceReset(const bool fwUpg);
    Status ReadFwVersion(void);
    Status ReadDeviceStatus(void);

    int GetNextRequest(ByteVector &data);
    Status ParseResponse(ByteVector &data);
    void ClearRequestQueue(void);

    static uint16_t Crc16(const ByteVector &data);

protected:
    bool initOk;
    typedef struct
    {
        int resLength;
        ByteVector data;
    } Request;
    Info info;

    std::list<Request> reqList;

    ec_t Init(void);
    Status AddQueueRequest(AgpMessage &msg);
    virtual int GetResLength(const uint8_t funct);

private:
    Status ValidateRequest(AgpMessage &msg);

    virtual Status ValidateSpecificRequest(const AgpMessage &msg) = 0;
    virtual Status ParseSpecificResponse(const uint8_t funct, const ByteVector &data) = 0;
    virtual int GetSpecificResLength(const uint8_t funct) = 0;
    virtual int EmptyReqListMsg(ByteVector &data);

};
} // namespace devices
} //namespace alf64


#endif /* DEVICES_GENERAL_HPP_ */
