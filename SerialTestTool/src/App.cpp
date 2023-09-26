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

#include "App.hpp"
using namespace std;

App::App(void):
        initOk(false)
{
    if(Init() == EC_OK)
    {
        initOk = true;
    }
}

App::~App(void)
{
    delete m_tester;
    m_tester = NULL;
}

ec_t App::Init(void)
{
    ec_t ec = EC_OK;
    m_tester = new SerialDeviceTester();
    options.clear();
    dut = defaultDut;
    if(strcmp(defaultDut, "sensx") == 0)
    {
        ec = m_tester->SetDut(SerialDeviceTester::DUT_SENSX);
        RETURN_EC_ON_ERROR(ec);
    }
    else
    {
        ec = EC_FAIL;
    }
    stressTestsEnable = false;

    return ec;
}

ec_t App::ParseArgs(int argc, const char** argv)
{
    RETURN_VAL_ON_FAIL(argc > 0, EC_FAIL);
    RETURN_VAL_ON_FAIL(argv != NULL, EC_FAIL);

    argc--; //exclude app name arg
    const char** const opts_start = argv + 1;

    // save args (if they are recognised as valid)
    std::vector<app_option_t> args;
    args.clear();
    if((argc > 0) && argc % 2 == 0) // always expect a pair: option_type, option_arg
    {
        const uint8_t options_cnt = static_cast<uint8_t>(argc / 2);
        const char** current_opt = opts_start;
        for(uint8_t i = 0; i < options_cnt; i++)
        {
            app_option_t opt;
            if(strcmp(*current_opt, opts_abbr.at(APP_OPTION_PORT_SELECTION)) == 0)
            {
                opt.option_type = static_cast<app_option_type_t>(APP_OPTION_PORT_SELECTION);
                opt.option_arg = *(current_opt + 1);
                args.push_back(opt);
            }
            else if(strcmp(*current_opt, opts_abbr.at(APP_OPTION_ENABLE_STRESS)) == 0)
            {
                opt.option_type = static_cast<app_option_type_t>(APP_OPTION_ENABLE_STRESS);
                opt.option_arg = *(current_opt + 1);
                args.push_back(opt);
            }
            current_opt += 2;
        }
    }

    // fill options with args, in the option_type_t order
    if(args.size() > 0)
    {
        uint8_t highest_value = 0;
        for(uint8_t i = 0; i < static_cast<uint8_t>(args.size()); i++)
        {
            app_option_t option = args.at(i);
            uint8_t neu_highest_value = static_cast<uint8_t>(option.option_type);
            if(neu_highest_value > highest_value)
            {
                highest_value = neu_highest_value;
            }
        }
        options.resize(static_cast<size_t>(highest_value+1));

        // app_option_t
        for(uint8_t i = 0; i < static_cast<uint8_t>(args.size()); i++)
        {
            app_option_t option = args.at(i);
            options.at(option.option_type) = option;
        }
    }

#if 0
    printf("Number of options got: %d \n", options.size());
    for(size_t i = 0; i < options.size(); i++)
    {
        printf("option(%d).option_type: %d\n option(%d).option_arg: %s\n",
                i,
                options.at(i).option_type,
                i,
                options.at(i).option_arg);
    }
#endif

    return EC_OK;
}

void App::DispHelp(void)
{
    fprintf(stdout, "======= HELP =======\n");
    printf("Important: this program shall be invoked with administrator privileges, since it operates on hw serial ports.\n");
    printf("%s is a tool to test AGP slave devices via the serial port.\n\n"
            "Usage:\n", appName);
    printf("%s \n\tDisplays this help information.\n", appName);
    printf("%s -p <portname>\n\t"
            "Runs standard AGP tests through the COM port pointed by portname.\n"
            "\tDefault BaudRate is: %s.\n"
            "\tDefault DataMode is: %s.\n"
            "\tDefault DUT (Device Under Test) is: %s.\n",
            appName,
            m_tester->m_serial->GetBaudrate(),
            m_tester->m_serial->GetDataMode(),
            GetDut());
    printf("%s -p <portname> -s 1 \n\t"
            "Enables additional stress tests execution if all standard AGP tests pass.\n\t"
            "Stress test execution takes few minutes, that's why it's optional.\n\n", appName);
}

ec_t App::Process(int argc, const char** argv)
{
    RETURN_VAL_ON_FAIL(initOk, EC_FAIL);

    ec_t ec = EC_OK;
    ec = ParseArgs(argc, argv);
    RETURN_EC_ON_ERROR(ec);

    switch(options.size())
    {
        case 1:
        case 2:
        {
            const char* portname = options.at(APP_OPTION_PORT_SELECTION).option_arg;
            ec = m_tester->m_serial->SetComPort(portname);
            if(ec == EC_FAIL)
            {
                printf("Unable to set serial portname to: %s\n"
                        "Invalid portname.\n"
                        "(make sure you typed portname with upper case)\n",
                        portname);
                break;
            }
            else
            {
                printf("portname set to: %s\n", portname);
            }

            ec = m_tester->m_serial->TestComPort();
            if(ec == EC_FAIL)
            {
                printf("Unable to open serial port: %s.\n"
                        "Make sure you run %s with administrative privileges or if such serial "
                        "port exists on your machine.\n",
                        m_tester->m_serial->GetComPort(),
                        appName);
                break;
            }

            if(options.size() == 2)
            {
                const char* stressArg = options.at(APP_OPTION_ENABLE_STRESS).option_arg;
                if(strcmp(stressArg, "1") == 0)
                {
                    stressTestsEnable = true;
                    printf("stress tests enabled\n");
                }
                else
                {
                    printf("stress tests opt given but arg invalid, ignoring\n");
                }
            }

            break;
        }

        // no options or too much options:
        case 0:
        default:
        {
            DispHelp();
            m_tester->m_serial->ListComPorts();
            ec = EC_BUSY;
            break;
        }
    }

    if(ec == EC_BUSY){return EC_OK;} // help was displayed, quit
    RETURN_VAL_ON_FAIL(ec == EC_OK, EC_FAIL); // error condition occurred

    RETURN_VAL_ON_FAIL(RunTests(), EC_FAIL);

    return ec;
}

const char* App::GetAppName(void)
{
    return appName;
}

const char* App::GetDut(void)
{
    return dut;
}

bool App::RunTests(void)
{
    bool funcResult = true;
    bool negResult = true;
    bool stressResult = true;

    // ---------- Functionality tests ----------
    printf("TestSelfPlis()...\t\t\t\t");
    bool testResult = m_tester->TestSelfPlis();
    if(testResult)
    {
        printf("SUCCESS\n");
    }
    else
    {
        printf("FAILED!\n");
        funcResult = false;
    }

    printf("TestFwVersionRead()...\t\t\t\t");
    testResult = m_tester->TestFwVersionRead();
    if(testResult)
    {
        printf("SUCCESS\n");
    }
    else
    {
        printf("FAILED!\n");
        funcResult = false;
    }

    printf("TestDeviceStatusRead()...\t\t\t");
    testResult = m_tester->TestDeviceStatusRead();
    if(testResult)
    {
        printf("SUCCESS\n");
    }
    else
    {
        printf("FAILED!\n");
        funcResult = false;
    }

    printf("TestMeasurementRead()...\t\t\t");
    testResult = m_tester->TestMeasurementRead();
    if(testResult)
    {
        printf("SUCCESS\n");
    }
    else
    {
        printf("FAILED!\n");
        funcResult = false;
    }

    printf("TestRtcSet()...\t\t\t\t\t");
    testResult = m_tester->TestRtcSet();
    if(testResult)
    {
        printf("SUCCESS\n");
    }
    else
    {
        printf("FAILED!\n");
        funcResult = false;
    }

    printf("TestHostWakeUpTimeSet()...\t\t\t");
    testResult = m_tester->TestHostWakeUpTimeSet();
    if(testResult)
    {
        printf("SUCCESS\n");
    }
    else
    {
        printf("FAILED!\n");
        funcResult = false;
    }

    RETURN_VAL_ON_FAIL(funcResult, false);
    m_tester->DispDutFields();
    // ------- END OF: Functionality tests -------

    // ---------- Negative tests ----------
    printf("TestFuncUnknownFunc()...\t\t\t");
    testResult = m_tester->TestNegUnknownFunc();
    if(testResult)
    {
        printf("SUCCESS\n");
    }
    else
    {
        printf("FAILED!\n");
        negResult = false;
    }

    printf("TestNegWrongCrc()...\t\t\t\t");
    testResult = m_tester->TestNegWrongCrc();
    if(testResult)
    {
        printf("SUCCESS\n");
    }
    else
    {
        printf("FAILED!\n");
        negResult = false;
    }

    printf("TestNegWrongPlis()...\t\t\t\t");
    testResult = m_tester->TestNegWrongPlis();
    if(testResult)
    {
        printf("SUCCESS\n");
    }
    else
    {
        printf("FAILED!\n");
        negResult = false;
    }

    printf("TestNegWrongDataSize()...\t\t\t");
    testResult = m_tester->TestNegWrongDataSize();
    if(testResult)
    {
        printf("SUCCESS\n");
    }
    else
    {
        printf("FAILED!\n");
        negResult = false;
    }

    printf("TestNegIntFrame()...\t\t\t\t");
    testResult = m_tester->TestNegIntFrame();
    if(testResult)
    {
        printf("SUCCESS\n");
    }
    else
    {
        printf("FAILED!\n");
        negResult = false;
    }
    RETURN_VAL_ON_FAIL(negResult, false);
    // ------- END OF: Negative tests -------

    printf("TestMeasStability()...\t\t\t\tSTART\n");
    m_tester->TestMeasStability();
    printf("TestMeasStability()...\t\t\t\tEND\n");

    // ---------- Stress tests ----------
    if(stressTestsEnable)
    {
        printf("TestFuncStress()...\t\t\t\t");
        testResult = m_tester->TestFuncStress();
        if(testResult)
        {
            printf("SUCCESS\n");
        }
        else
        {
            printf("FAILED!\n");
            stressResult = false;
        }
    }
    RETURN_VAL_ON_FAIL(stressResult, false);
    // ------- END OF: Stress tests -------

    return true;
}
