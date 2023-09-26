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

#include <string.h>

#include "rs232/rs232.h"

#include "serial/Serial.hpp"

using namespace std;

constexpr const char* Serial::portnames_h[maxSysComPorts];

Serial::Serial(void):
        initOk(false)
{
    if(Init() == EC_OK)
    {
        initOk = true;
    }
}

Serial::~Serial(void)
{
    CloseComPort();
}

ec_t Serial::Init(void)
{
    _isComPortOpened = false;
    portComName = defaultComPortName;
    portComNum = GetComPortNumFromName(portComName);
    RETURN_VAL_ON_FAIL(portComNum != -1, EC_FAIL);
    baudRate_str = defaultBaudRate;
    baudRate_val = GetBaudRateFromName(baudRate_str);
    RETURN_VAL_ON_FAIL(baudRate_val != -1, EC_FAIL);
    dataMode = defaultDataMode;

    return EC_OK;
}

int Serial::GetBaudRateFromName(const char* baudrate_str)
{
    if(baudrate_str == NULL)
    {
        return -1;
    }

    int baud = -1;

    if(strcmp(baudrate_str, "115200") == 0)
    {
        baud = 115200;
    }
    else
    {
        baud = -1;
    }

    return baud;
}

const char* Serial::GetBaudrate(void)
{
    RETURN_VAL_ON_FAIL(initOk, NULL);
    return baudRate_str;
}

const char* Serial::GetDataMode(void)
{
    RETURN_VAL_ON_FAIL(initOk, NULL);
    return dataMode;
}

const char* Serial::GetComPort(void)
{
    RETURN_VAL_ON_FAIL(initOk, NULL);
    return portComName;
}

int Serial::GetComPortNumFromName(const char* portname)
{
    RETURN_VAL_ON_FAIL(portname != NULL, -1);
    return RS232_GetPortnr(portname);
}

ec_t Serial::TestComPort(void)
{
    RETURN_VAL_ON_FAIL(initOk, EC_FAIL);
    ec_t ec = EC_FAIL;
    if(RS232_OpenComport(portComNum, 115200, "8n1", 0) == 0)
    {
        RS232_CloseComport(portComNum);
        ec = EC_OK;
    }

    return ec;
}

void Serial::ListComPorts(void)
{
    uint8_t cp_found = 0;

    printf("Listing serial ports detected on this machine:\n");

    for(uint8_t i = 0; i < maxSysComPorts; i++)
    {
        int port_num = GetComPortNumFromName(portnames_h[i]);
        if(port_num == -1)
        {
            printf("Error! Failed to list serial portnames. \n");
            break;
        }
        if(RS232_OpenComport(port_num, 115200, "8n1", 0) == 0)
        {
            printf("* %s\n",portnames_h[i]);
            RS232_CloseComport(port_num);
            cp_found++;
        }
    }
    printf("Total serial ports found: %d. \n\n", cp_found);

    return;
}

ec_t Serial::SetComPort(const char* portname)
{
    RETURN_VAL_ON_FAIL(initOk, EC_FAIL);
    RETURN_VAL_ON_FAIL(portComName != NULL, EC_FAIL);

    ec_t ec = EC_OK;

    int portnum = GetComPortNumFromName(portname);
    if(portnum != -1)
    {
        portComName = portname;
        portComNum = portnum;
    }
    else
    {
        ec = EC_FAIL;
    }

    return ec;
}

ec_t Serial::OpenComPort(void)
{
    RETURN_VAL_ON_FAIL(initOk, EC_FAIL);
    RETURN_VAL_ON_FAIL(!_isComPortOpened, EC_OK);

    if(RS232_OpenComport(portComNum, baudRate_val, dataMode, 0) != 0)
    {
        return EC_FAIL;
    }
    else
    {
        _isComPortOpened = true;
        return EC_OK;
    }
}

void Serial::CloseComPort(void)
{
    RETURN_VOID_ON_FAIL(initOk);
    RS232_CloseComport(portComNum);
    _isComPortOpened = false;
}

bool Serial::isComPortOpened(void)
{
    return _isComPortOpened;
}

int Serial::DumpBuffToPort(unsigned char* buff, unsigned int buff_size)
{
    RETURN_VAL_ON_FAIL(initOk, -1);
    RETURN_VAL_ON_FAIL(buff != NULL, -1);
    RETURN_VAL_ON_FAIL(buff_size > 0, -1);
    RETURN_VAL_ON_FAIL(_isComPortOpened, -1);

    int n = RS232_SendBuf(portComNum, buff, buff_size);
    RS232_flushTX(portComNum);

    return n;
}

int Serial::DumpStrToPort(const unsigned char* txt)
{
    RETURN_VAL_ON_FAIL(initOk, -1);
    RETURN_VAL_ON_FAIL(txt != NULL, -1);
    RETURN_VAL_ON_FAIL(_isComPortOpened, -1);

    int bt_cnt = 0;

    while(*txt != '\0')
    {
        int n = RS232_SendByte(
                portComNum,
                static_cast<unsigned char>(*(txt++)));
        if((n < 0) && (bt_cnt == 0))
        {
            bt_cnt = -1;
            break;
        }
        else if((n == -1) && (bt_cnt > 0))
        {
            break;
        }
        else
        {
            bt_cnt += n;
        }
    }

    RS232_flushTX(portComNum);

    return bt_cnt;
}

ec_t Serial::FlushRX(void)
{
    RETURN_VAL_ON_FAIL(initOk, EC_FAIL);
    RETURN_VAL_ON_FAIL(_isComPortOpened, EC_FAIL);

    RS232_flushRX(portComNum);

    return EC_OK;
}

int Serial::ReadDataFromPort(unsigned char* buff)
{
    RETURN_VAL_ON_FAIL(initOk, -1);
    RETURN_VAL_ON_FAIL((buff != NULL), -1);
    RETURN_VAL_ON_FAIL(_isComPortOpened, -1);

    int n = RS232_PollComport(portComNum, rcvBuff, rcvBuffSize);
    RETURN_VAL_ON_FAIL(n > 0, n);

    memcpy(buff, rcvBuff, static_cast<size_t>(n));

    return n;
}

int Serial::ReadDataFromPort(unsigned char* buff, unsigned int max)
{
    RETURN_VAL_ON_FAIL(initOk, -1);
    RETURN_VAL_ON_FAIL((buff != NULL), -1);
    RETURN_VAL_ON_FAIL(_isComPortOpened, -1);
    RETURN_VAL_ON_FAIL(max <= rcvBuffSize, -1);

    int n = RS232_PollComport(portComNum, rcvBuff, max);
    RETURN_VAL_ON_FAIL(n > 0, n);

    memcpy(buff, rcvBuff, static_cast<size_t>(n));

    return n;
}

ec_t Serial::TestSerial1(void)
{
    RETURN_VAL_ON_FAIL(initOk, EC_FAIL);

    ec_t ec = OpenComPort();
    RETURN_EC_ON_ERROR(ec);
    unsigned char test_msg[] = "This is the SerialTestTool here! :)\n";
    int data_sent = DumpStrToPort(test_msg);

    printf("Polling for RX...\n");
    int data_rcv = 0;
    unsigned char rcv_buff[4096] = {0};
    do
    {
        data_rcv = ReadDataFromPort(rcv_buff);
    }while(data_rcv == 0);

    printf("Data sent: %d\n", data_sent);
    printf("Data rcv: %d\n", data_rcv);
    printf("rcv_buff contains: %s\n", rcv_buff);
    CloseComPort();

    return ec;
}

ec_t Serial::TestSerial2(void)
{
    RETURN_VAL_ON_FAIL(initOk, EC_FAIL);

    ec_t ec = OpenComPort();
    RETURN_EC_ON_ERROR(ec);

    unsigned char test_msg[] = "This is the SerialTestTool here! :)\n";
    int data_sent = DumpBuffToPort(test_msg, (sizeof(test_msg) - 1));
    printf("Data sent: %d\n", data_sent);

    CloseComPort();

    return ec;
}
