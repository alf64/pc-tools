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

#ifndef SERIAL_SERIAL_HPP_
#define SERIAL_SERIAL_HPP_

#include <iostream>
#include <string.h>

#include "ec.h"

class Serial
{
public:
    Serial(void);
    ~Serial(void);

    //!< Returns current baudrate.
    const char* GetBaudrate(void);

    //!< Returns current data mode.
    const char* GetDataMode(void);

    //!< Returns current com port.
    const char* GetComPort(void);

    /*
     * Tests COM port by opening and closing it.
     * Returns EC_OK if was able to open COM port.
     * Returns EC_FAIL if wasn't able to open COM port.
     */
    ec_t TestComPort(void);

    //!< Prints detected com ports.
    static void ListComPorts(void);

    //!< Sets com port.
    ec_t SetComPort(const char* portname);

    //!< Opens com port.
    ec_t OpenComPort(void);

    //!< Closes com port.
    void CloseComPort(void);

    //!< Returns true if com port is opened, otherwise returns false.
    bool isComPortOpened(void);

    /*
     * Dumps buffer to serial port.
     * Returns number of bytes written.
     * Returns -1 if error occurred.
     */
    int DumpBuffToPort(unsigned char* buff, unsigned int buff_size);

    /*
     * Dumps text to serial port.
     * Returns number of bytes written.
     * Returns -1 if error occurred.
     */
    int DumpStrToPort(const unsigned char* txt);

    /*
     * Invalidates RX hardware buffer.
     * If you expect response upon sending something over serial
     * port, you should consider calling this function before sending
     * just to make sure the RX buffer is clean
     * (does not contain any unread data)
     */
    ec_t FlushRX(void);

    /*
     * Read data from port.
     * Data is written under buff.
     * Actual number of data bytes obtained is returned.
     * Returns -1 if error occurred.
     */
    int ReadDataFromPort(unsigned char* buff);

    /*
     * @brief Read data from port.
     * @param buff - Pointer where the read data will be written.
     * @param max - Maximum bytes to read.
     * @returns Number of data bytes obtained, or error.
     * @retval >= 0 number of data bytes obtained.
     * @retval < 0 error
     */
    int ReadDataFromPort(unsigned char* buff, unsigned int max);

    //< Example1 of how to use Serial functionality provided by this class.
    ec_t TestSerial1(void);

    //< Example2 of how to use Serial functionality provided by this class.
    ec_t TestSerial2(void);

private:
    bool initOk;

    static const int rcvBuffSize = 4096;
    static constexpr const char* defaultComPortName = "COM1";
    static constexpr const char* defaultBaudRate = "115200";
    static constexpr const char* defaultDataMode = "8n1";
    static const int maxSysComPorts = 32;
    static constexpr const char* portnames_h[maxSysComPorts] =
    {
        "COM1",  "COM2",  "COM3",  "COM4",
        "COM5",  "COM6",  "COM7",  "COM8",
        "COM9",  "COM10", "COM11", "COM12",
        "COM13", "COM14", "COM15", "COM16",
        "COM17", "COM18", "COM19", "COM20",
        "COM21", "COM22", "COM23", "COM24",
        "COM25", "COM26", "COM27", "COM28",
        "COM29", "COM30", "COM31", "COM32"
    };

    bool _isComPortOpened;
    const char* portComName;
    int portComNum;
    const char* baudRate_str;
    int baudRate_val;
    const char* dataMode;
    unsigned char rcvBuff[rcvBuffSize] = {0};

    ec_t Init(void);

    /*
     * Returns baudarate in an integer form.
     * Returns -1 if error occurred.
     */
    static int GetBaudRateFromName(const char* baudrate_str);

    /*
     * Returns com port representation in an integer form.
     * Returns -1 if error occurred.
     */
    static int GetComPortNumFromName(const char* portname);
};


#endif /* SERIAL_SERIAL_HPP_ */
