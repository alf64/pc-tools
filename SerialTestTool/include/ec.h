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

#ifndef EC_H_
#define EC_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    EC_OK = 0,
    EC_FAIL = 1,
    EC_BUSY = 2
}ec_t;


#if defined(LOG_ENABLE) && LOG_ENABLE == 1
    #define LOG_MSG printf
#else
    #define LOG_MSG(...) /* does nothing, just eats parameters */
#endif

//!< Marks resource as unused (gets rid of compiler warnings)
#define UNUSED(x) (void)(x)

//!< Returns array length (in terms of elements count)
#define ARRAY_LENGTH(a) (sizeof(a) / sizeof((a)[0]))

#define HALT_ON_ERROR(ec)                   \
    if ((ec) != EC_OK)            \
    {                                       \
        while(1){};                         \
    }

#define RETURN_EC_ON_ERROR(ec)              \
    if ((ec) != EC_OK)            \
    {                                       \
        return (ec);                        \
    }

#define RETURN_VOID_ON_ERROR(ec)            \
    if ((ec) != EC_OK)            \
    {                                       \
        return;                             \
    }

#define BREAK_ON_ERROR(ec)                  \
    if ((ec) != EC_OK)            \
    {                                       \
        break;                              \
    }

#define BREAK_ON_FAIL(condition)                \
        if (!(condition))                       \
        {                                       \
            break;                              \
        }

#define RETURN_VOID_ON_FAIL(condition)      \
    if (!(condition))                       \
    {                                       \
        return;                             \
    }

#define RETURN_VAL_ON_FAIL(condition, val)  \
    if (!(condition))                       \
    {                                       \
        return val;                         \
    }

#define RETURN_NULL_ON_FAIL(condition)     \
    if (!(condition))                           \
    {                                           \
        return NULL;                             \
    }

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* EC_H_ */
