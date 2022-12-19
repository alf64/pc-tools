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

#ifndef SBDOP_H_
#define SBDOP_H_

#include "rs232.h"

#if defined(__linux__) || defined(__FreeBSD__)   /* Linux & FreeBSD */
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#endif

#if defined(__linux__) || defined(__FreeBSD__)   /* Linux & FreeBSD */
#define MAX_SYS_COMPORTS 38
#else
#define MAX_SYS_COMPORTS 32
#endif
extern const char* portnames_h[MAX_SYS_COMPORTS];

//!< Max file size to be dumped is 1GB
#define SBDOP_MAX_FILESIZE 1073741824

#if defined(__linux__) || defined(__FreeBSD__)   /* Linux & FreeBSD */
//!< Max delay (miliseconds) supported by this app (nanosleep limit)
#define SBDOP_MAX_DELAYMS 999
#else
//!< Max delay (miliseconds) supported by this app
#define SBDOP_MAX_DELAYMS 60000
#endif

#define SBDOP_DEFAULT_BAUDRATE "9600"
#define SBDOP_DEFAULT_DATAMODE "8n1"
#define SBDOP_DEFAULT_DELAY "0"
#define SBDOP_DEFAULT_BURST "1"

typedef enum
{
    OP_DISP_HELP = 0,
    OP_LIST_COMPORTS = 1,
    OP_DUMP_BINARY = 2,

    OP_INVALID = 0xFF
}op_t;

typedef struct
{
    const char* portname;
    const char* baudrate;
    const char* datamode; // 8N1, 9N2, ...
    const char* filename;
    const char* delay;
    const char* burst;
    uint8_t reserved[20];
}op_args_db_t;

typedef union
{
    uint8_t value_arr[32];
    op_args_db_t dumpbin;
}op_args_t;

typedef struct
{
    op_t op;
    op_args_t args;
} ops_t;

/*
 * @brief Displays help info.
 */
void SBDOP_DispHelpInfo(void);

/*
 * @brief Gets com port number (int) from portname (const char*).
 * @param portname A pointer to the name of the port.
 * @retval 0-(MAX_SYS_COMPORTS-1) Valid com port number.
 * @retval -1 Cannot get port number from given name.
 */
int SBDOP_GetComPortNumFromName(const char* portname);

/*
 * @brief Gets baudrate (int) from baudrate (const char*).
 * @param baudrate A pointer to the baudrate.
 * @retval >0 Valid baudrate.
 * @retval -1 Invalid baudrate.
 */
int SBDOP_GetBaudRateFromName(const char* baudrate);

/*
 * @brief Validates given datamode.
 * @param datamode A pointer to datamode.
 * @retval TRUE If given datamode is valid.
 * @retval FALSE If given datamode is invalid.
 */
uint8_t SBDOP_ValidDataMode(const char* datamode);

/*
 * @brief Validates file with given filename.
 * @details
 * Validating a file means to check if it exists, if it is able to be opened andd
 * if it is smaller than SBDOP_MAX_FILESIZE.
 * @param filename A name of the file to validate.
 * @param filesize A pointer where this function will save the size (in bytes) of the validated file.
 * Value under this parameter is valid only if function returns with success.
 * This parameter can be omitted by passing NULL.
 * @retval TRUE If such file is valid.
 * @retval FALSE If such file is not valid.
 */
uint8_t SBDOP_ValidFile(
        const char* filename,
        uint32_t* filesize);

/*
 * @brief Validates com port with given portname (checks if it is possible to open such port).
 * @param portname A name of the port to be validated.
 * @retval TRUE If such port is valid and usable.
 * @retval FALSE If such port is invalid (unusable).
 */
uint8_t SBDOP_ValidComPort(const char* portname);

/*
 * @brief Returns delay (as int) from the given delay (const char*).
 * @param delay A delay as const char*
 * @retval -1 Failed to get int from given const char*
 * @retval !-1 The delay as int.
 */
int SBDOP_GetDelayFromName(const char* delay);

/*
 * @brief Returns burst (as int) from the given burst (const char*).
 * @param burst A burst as const char*
 * @retval -1 Failed to get int from given const char*
 * @retval !-1 The burst as int.
 */
int SBDOP_GetBurstFromName(const char* burst);

/*
 * @brief Validates burst.
 *
 * @param burst A burst to be validated.
 * @param datasize A size of the data burst is associated with.
 *
 * @retval TRUE If such file is valid.
 * @retval FALSE If such file is not valid.
 */
uint8_t SBDOP_ValidBurst(
        uint32_t burst,
        uint32_t datasize);

/*
 * @brief Lists system's available com ports names.
 */
void SBDOP_ListComPorts(void);

/*
 * @brief Dumps binary file to com port.
 *
 * @param portnum Number of serial port to which the binary file shall be dumped.
 * @param baud Baudrate to use with serial port.
 * @param datamode Datamode to use with serial port.
 * @param filename A name of the binary file to be dumped.
 * @param delay_ms A delay (in miliseconds) to be used between each binary character send.
 * @param burst A burst (in bytes) to be applied.
 * Burst > 1 means that:
 * - data will be send grouped, each group of size: burst
 * - there is no delay between bytes send in group
 * - delay_ms parameter is interpreted as a delay
 * between each group transmission.
 *
 * @retval -1 If failed to dump binary file to port.
 * @retval 0 If succeeded to dump binary file to port.
 */
int SBDOP_DumpBinaryToPort(
        int portnum,
        int baud,
        int delay_ms,
        int burst,
        const char* datamode,
        const char* filename,
        uint32_t filesize);

/*
 * @brief Returns x*100 / y as uint16_t.
 * @attention The following must be true: x <= y
 * @retval 0 - 100 Successful x*100 / y result.
 * @retval 0xFFFF Error: probably x is not <= y.
 */
uint16_t SBDOP_PercentageCompletion(
        uint32_t x,
        uint32_t y);

/*
 * @brief Performs delay.
 * @param delay_ms Delay to be performed (in miliseconds).
 */
void SBDOP_Delay(uint32_t delay_ms);


#endif /* SBDOP_H_ */
