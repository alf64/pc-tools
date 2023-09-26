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

#ifndef APP_HPP_
#define APP_HPP_

#include <devices/SensX.hpp>
#include <iostream>
#include <vector>

#include "ec.h"
#include "serial/Serial.hpp"
#include "SerialDeviceTester.hpp"

class App
{
public:
    typedef enum
    {
        APP_OPTION_PORT_SELECTION = 0x00,
        APP_OPTION_ENABLE_STRESS = 0x01,

        APP_OPTION_RESERVED = 0xFF
    }app_option_type_t;

    typedef const char* app_option_arg_t;

    typedef struct
    {
        app_option_type_t option_type;
        app_option_arg_t option_arg;
    }app_option_t;

    App(void);
    ~App(void);
    static const char* GetAppName(void);
    ec_t Process(int argc, const char** argv);

private:
    static constexpr const char* appName = "SerialTestTool";
    static constexpr const char* defaultDut = "sensx";
    const std::vector<const char*> opts_abbr = {
            static_cast<const char*>("-p"), // APP_OPTION_PORT_SELECTION,
            static_cast<const char*>("-s") // APP_OPTION_ENABLE_STRESS
    };

    bool initOk;

    SerialDeviceTester* m_tester;
    std::vector<app_option_t> options;
    const char* dut;
    bool stressTestsEnable;

    ec_t Init(void);
    ec_t ParseArgs(int argc, const char** argv);
    void DispHelp(void);
    const char* GetDut(void);
    bool RunTests(void);
};



#endif /* APP_HPP_ */
