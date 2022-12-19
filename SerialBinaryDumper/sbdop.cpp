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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>

#if defined(__linux__) || defined(__FreeBSD__)   /* Linux & FreeBSD */
#include <time.h>
#else
#include <chrono>
using high_res_clock_t = std::chrono::high_resolution_clock;
using steady_clock_t = std::chrono::steady_clock;
using milliseconds_t = std::chrono::milliseconds;
using microseconds_t = std::chrono::microseconds;
#endif

#include "sbdop.h"


#if defined(__linux__) || defined(__FreeBSD__)   /* Linux & FreeBSD */
const char* portnames_h[MAX_SYS_COMPORTS] =
{
"ttyS0","ttyS1","ttyS2","ttyS3","ttyS4","ttyS5",
"ttyS6","ttyS7","ttyS8","ttyS9","ttyS10","ttyS11",
"ttyS12","ttyS13","ttyS14","ttyS15","ttyUSB0",
"ttyUSB1","ttyUSB2","ttyUSB3","ttyUSB4","ttyUSB5",
"ttyAMA0","ttyAMA1","ttyACM0","ttyACM1",
"rfcomm0","rfcomm1","ircomm0","ircomm1",
"cuau0","cuau1","cuau2","cuau3",
"cuaU0","cuaU1","cuaU2","cuaU3"
};
#else /* windows */
const char* portnames_h[MAX_SYS_COMPORTS] =
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
#endif


void SBDOP_DispHelpInfo(void)
{
    fprintf(stdout, "======= HELP =======\n");
#if defined(__linux__) || defined(__FreeBSD__)   /* Linux & FreeBSD */
    printf("Important: this program shall be called with super user (sudo) privileges to properly discover serial ports.\n");
#else
    printf("Important: this program shall be invoked with administrator privileges, since it operates on hw serial ports.\n");
#endif
    printf("SerialBinaryDumper is a tool to dump binary file to the serial port.\n\n"
            "Usage:\n");
    printf("SerialBinaryDumper -h\n\t Displays this help information.\n");
    printf("SerialBinaryDumper -l\n\t Lists serial portnames available on the machine.\n");
    printf("SerialBinaryDumper -p <portname> -f <filename> [<options>]\n\t"
            "Dumps binary file pointed by filename to the port pointed by portname.\n"
            "\tMaximum supported filesize is: %d bytes.\n\n",
            SBDOP_MAX_FILESIZE);
    printf("Possible options are:\n"
            "-b <baudrate>\t A baudrate (in bps) to open serial port with.\n"
            "Default baudrate is: %s.\n"
            "Supported baudrate values are: 110, 300, 600, 1200, 2400, 4800, 9600, 19200,\n"
            "38400, 57600, 115200, 128000, 256000, 500000, 921600, 1000000, 1500000, 2000000, 3000000.\n",
            SBDOP_DEFAULT_BAUDRATE);
    printf("-dm <datamode>\t A datamode to open serial port with.\n"
            "Default datamode is: %s.\n"
            "Supported datamode values are: 5n1, 5n2, 5e1, 5e2, 5o1, 5o2,\n"
            "6n1, 6n2, 6e1, 6e2, 6o1, 6o2, 7n1, 7n2, 7e1, 7e2, 7o1, 7o2, 8n1, 8n2, 8e1, 8e2, 8o1, 8o2.\n",
            SBDOP_DEFAULT_DATAMODE);
    printf("-dl <delay>\t A delay (in miliseconds) to apply between each byte send.\n"
            "Default delay is: %s.\n"
            "Supported delay value range is: <0, %d>.\n",
            SBDOP_DEFAULT_DELAY,
            SBDOP_MAX_DELAYMS);
    printf("-bst <burst>\t A burst (in terms of number of bytes) used to group the data being dumped to port.\n"
            "Default burst is: %s.\n"
            "Supported burst value range is: <1, %d>.\n"
            "burst cannot be greater than size of the file being dumped, and also such file size needs "
            "to be dividable by burst.\n\n",
            SBDOP_DEFAULT_BURST,
            SBDOP_MAX_FILESIZE);
#if defined(__linux__) || defined(__FreeBSD__)   /* Linux & FreeBSD */
    printf("Sample invocations:\n"
            "sudo SerialBinaryDumper -l \n"
            "\tThis lists serial portnames available on the machine.\n"
            "sudo SerialBinaryDumper -p ttyS0 -f data32B.bin -b 57600 -dm 8n1 -dl 3\n"
            "\tThis dumps data32B.bin file to serial port ttyS0, with baudrate: 57600 (bps), datamode: 8n1, delay: 3 (miliseconds), default burst.\n"
            "sudo SerialBinaryDumper -p ttyS1 -f data64B.bin\n"
            "\tThis dumps data64B.bin file to serial port ttyS1, with default baudrate: %s (bps), default datamode: %s, default delay: %s (miliseconds).\n"
            "SerialBinaryDumper -p ttyS1 -f data32B.bin -b 57600 -dm 8n1 -dl 3 -bst 4\n"
            "\tThis dumps data32B.bin file to serial port ttyS1, with baudrate: 57600 (bps), datamode: 8n1, delay: 3 (miliseconds), burst: 4.\n"
            "Burst of 4 means that data is being send as 4-byte chunks. There is no delay between bytes in chunks, "
            "instead given delay is applied between chunks.\n",
            SBDOP_DEFAULT_BAUDRATE, SBDOP_DEFAULT_DATAMODE, SBDOP_DEFAULT_DELAY);
#else
    printf("Sample invocations:\n"
            "SerialBinaryDumper.exe -l \n"
            "\tThis lists serial portnames available on the machine.\n"
            "SerialBinaryDumper.exe -p COM3 -f data32B.bin -b 57600 -dm 8n1 -dl 3\n"
            "\tThis dumps data32B.bin file to serial port COM3, with baudrate: 57600 (bps), datamode: 8n1, delay: 3 (miliseconds), default burst.\n"
            "SerialBinaryDumper.exe -p COM4 -f data64B.bin\n"
            "\tThis dumps data64B.bin file to serial port COM4, with default baudrate: %s (bps), default datamode: %s, default delay: %s (miliseconds), default burst: %s (bytes).\n"
            "SerialBinaryDumper.exe -p COM3 -f data32B.bin -b 57600 -dm 8n1 -dl 3 -bst 4\n"
            "\tThis dumps data32B.bin file to serial port COM3, with baudrate: 57600 (bps), datamode: 8n1, delay: 3 (miliseconds), burst: 4.\n"
            "Burst of 4 means that data is being send as 4-byte chunks. There is no delay between bytes in chunks, "
            "instead given delay is applied between chunks.\n",
            SBDOP_DEFAULT_BAUDRATE, SBDOP_DEFAULT_DATAMODE, SBDOP_DEFAULT_DELAY, SBDOP_DEFAULT_BURST);
#endif
}

int SBDOP_GetComPortNumFromName(const char* portname)
{
    if(portname == NULL)
    {
        return -1;
    }
    else
    {
        return RS232_GetPortnr(portname);
    }
}

int SBDOP_GetBaudRateFromName(const char* baudrate)
{
    if(baudrate == NULL)
    {
        return -1;
    }

    int baud = -1;

    if(strcmp(baudrate, "110") == 0)
    {
        baud = 110;
    }
    else if(strcmp(baudrate, "300") == 0)
    {
        baud = 300;
    }
    else if(strcmp(baudrate, "600") == 0)
    {
        baud = 600;
    }
    else if(strcmp(baudrate, "1200") == 0)
    {
        baud = 1200;
    }
    else if(strcmp(baudrate, "2400") == 0)
    {
        baud = 2400;
    }
    else if(strcmp(baudrate, "4800") == 0)
    {
        baud = 4800;
    }
    else if(strcmp(baudrate, "9600") == 0)
    {
        baud = 9600;
    }
    else if(strcmp(baudrate, "19200") == 0)
    {
        baud = 19200;
    }
    else if(strcmp(baudrate, "38400") == 0)
    {
        baud = 38400;
    }
    else if(strcmp(baudrate, "57600") == 0)
    {
        baud = 57600;
    }
    else if(strcmp(baudrate, "115200") == 0)
    {
        baud = 115200;
    }
    else if(strcmp(baudrate, "128000") == 0)
    {
        baud = 128000;
    }
    else if(strcmp(baudrate, "256000") == 0)
    {
        baud = 256000;
    }
    else if(strcmp(baudrate, "500000") == 0)
    {
        baud = 500000;
    }
    else if(strcmp(baudrate, "921600") == 0)
    {
        baud = 921600;
    }
    else if(strcmp(baudrate, "1000000") == 0)
    {
        baud = 1000000;
    }
    else if(strcmp(baudrate, "1500000") == 0)
    {
        baud = 1500000;
    }
    else if(strcmp(baudrate, "2000000") == 0)
    {
        baud = 2000000;
    }
    else if(strcmp(baudrate, "3000000") == 0)
    {
        baud = 3000000;
    }
    else
    {
        // do nothing, invalid baudrate
    }

    return baud;
}

uint8_t SBDOP_ValidDataMode(const char* datamode)
{
    if(datamode == NULL)
    {
        return FALSE;
    }

    uint8_t valid = FALSE;
    uint8_t valid0 = FALSE;
    uint8_t valid1 = FALSE;
    uint8_t valid2 = FALSE;

    if(strlen(datamode) != 3)
    {
        return FALSE;
    }

    switch(datamode[0])
    {
        case '8':
        case '7':
        case '6':
        case '5':
        {
            valid0 = TRUE;
            break;
        }
        default :
        {
            break;
        }
    }

    switch(datamode[1])
    {
        case 'N':
        case 'n':
        case 'E':
        case 'e':
        case 'O':
        case 'o':
        {
            valid1 = TRUE;
            break;
        }
        default:
        {
            break;
        }
    }

    switch(datamode[2])
    {
        case '1':
        case '2':
        {
            valid2 = TRUE;
            break;
        }
        default:
        {
            break;
        }
    }

    valid = valid0 & valid1 & valid2;

    return valid;
}

uint8_t SBDOP_ValidFile(
        const char* filename,
        uint32_t* filesize)
{
    if(filename == NULL)
    {
        return FALSE;
    }

    FILE* binfile = fopen(filename, "rb");
    if(binfile == NULL)
    {
        return FALSE;
    }

    uint8_t temp = 0;
    uint32_t size = 0;
    while((feof(binfile) == 0) && (size < SBDOP_MAX_FILESIZE))
    {
        int ec = fread(&temp, 1, 1, binfile);
        if(ec == 0) // fread tells us nothing new read, end-of-file reached or error occured
        {
            break;
        }
        size++;
    }

    if(feof(binfile) == 0) // end-of-file not reached due to max size exceed
    {
        return FALSE;
    }

    if(filesize != NULL)
    {
        *filesize = size;
    }

    fclose(binfile);

    return TRUE;
}

uint8_t SBDOP_ValidComPort(const char* portname)
{
    int portnum = SBDOP_GetComPortNumFromName(portname);
    if(portnum == -1)
    {
        return FALSE;
    }

    if(RS232_OpenComport(portnum, 9600, "8n1", 0) != 0)
    {
        return FALSE;
    }
    else
    {
        RS232_CloseComport(portnum);
    }

    return TRUE;
}

int SBDOP_GetDelayFromName(const char* delay)
{
    if(delay == NULL)
    {
        return -1; // error
    }

    // atoi returns 0 on both: "0" and invalid conversion so we need to differentiate it
    if(strcmp(delay, "0") == 0)
    {
        return 0; // no delay
    }

    int dl = atoi(delay);
    if(dl == 0)
    {
        return -1; // error
    }

    return dl;
}

int SBDOP_GetBurstFromName(const char* burst)
{
    return SBDOP_GetDelayFromName(burst);
}

uint8_t SBDOP_ValidBurst(
        uint32_t burst,
        uint32_t datasize)
{
    if(burst == 0)
    {
        return FALSE;
    }

    if(burst > datasize)
    {
        return FALSE;
    }

    if(datasize % burst != 0)
    {
        return FALSE;
    }

    return TRUE;
}

void SBDOP_ListComPorts(void)
{
    uint8_t cp_found = 0;

    for(uint8_t i = 0; i < MAX_SYS_COMPORTS; i++)
    {
        int port_num = SBDOP_GetComPortNumFromName(portnames_h[i]);
        if(port_num == -1)
        {
            printf("Error! Failed to list serial portnames. \n");
            break;
        }
        if(RS232_OpenComport(port_num, 9600, "8n1", 0) == 0)
        {
            printf("%s\n",portnames_h[i]);
            RS232_CloseComport(port_num);
            cp_found++;
        }
    }
    printf("Total serial ports found: %d. \n", cp_found);

    return;
}

int SBDOP_DumpBinaryToPort(
        int portnum,
        int baud,
        int delay_ms,
        int burst,
        const char* datamode,
        const char* filename,
        uint32_t filesize)
{
    int ret = 0;
    if(datamode == NULL || filename == NULL)
    {
        return -1;
    }

    int ec = RS232_OpenComport(portnum, baud, datamode, 0);
    if(ec != 0)
    {
        printf("Error! Unable to open serial port.\n");
        return -1;
    }

    FILE* binfile = fopen(filename, "rb");
    if(binfile == NULL)
    {
        return -1;
    }

    uint8_t data;
    int burst_cnt = 0;
    for(uint32_t i = 0; i < filesize; i++)
    {
        uint16_t perc = SBDOP_PercentageCompletion((i+1), filesize);
        if(perc != 0xffff)
        {
            if(i == 0 && filesize == 1)
            {
                printf("Progress: %d%%\n", perc);
            }
            else if(i == 0)
            {
                printf("Progress: %d%% ", perc);
            }
            else if(i > 0 && i < (filesize-1))
            {
                putchar(0x0D);
                printf("Progress: %d%% ", perc);
            }
            else // i > 0 && i == filesize-1
            {
                putchar(0x0D);
                printf("Progress: %d%%\n", perc);
            }
        }

        ec = fread(&data, 1, 1, binfile);
        if(ec == 0) // nothing read
        {
            printf("Error! Unexpected end-of-file reached.\n");
            ret = -1;
            break;
        }

        ec = RS232_SendByte(portnum, data);
        if(ec == 1)
        {
            printf("Error! Failed to send data.\n");
            ret = -1;
            break;
        }
        burst_cnt++;
        if(burst_cnt == burst)
        {
            burst_cnt = 0;
            SBDOP_Delay(delay_ms);
        }
    }

    RS232_CloseComport(portnum);
    fclose(binfile);

    return ret;
}

uint16_t SBDOP_PercentageCompletion(
        uint32_t x,
        uint32_t y)
{
    if(x > y)
    {
        return 0xffff;
    }

    return (uint16_t)((x*100) / y);
}

void SBDOP_Delay(uint32_t delay_ms)
{
#if defined(__linux__) || defined(__FreeBSD__)   /* Linux & FreeBSD */
        //usleep((delay_ms*1000)); //POSIX, <unistd.h> - deprecated
        uint32_t delay_ns = delay_ms * 1000000;
        nanosleep(&((struct timespec){.tv_nsec = delay_ns, .tv_sec= 0}), NULL);
#else
        /*
         * TIP:
         * Sleep causes thread switch. This means that there is only guarantee that you will AT LEAST sleep for a given amount of time,
         * but in fact it may be a lot longer since it depends on the OS when exactly your thread will return to operate.
         * Example: Sleep(2) may sleep for 2 ms or for 25 ms, you never know.
         */
        // Sleep(delay_ms); // <Windows.h>

        /*
         * using chrono lib instead to improve performance of delay
         */
        steady_clock_t clock;
        auto start_time = clock.now();
        microseconds_t elapsed_us;
        const long long force_wait_us = (delay_ms * 1000);
        do
        {
            auto end_time = clock.now();
            elapsed_us = std::chrono::duration_cast<microseconds_t>(end_time - start_time);
        }
        //while(elapsed_us.count() < force_wait_us);
        // advice: do not use .count() in your alogrithms since it breaks optmial chrono behavior.
        while(elapsed_us < microseconds_t{force_wait_us});
        //printf("duration is: %I64u us\n", elapsed_us.count());
#endif
}

