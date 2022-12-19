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

#include "sbdop.h"

int main(int argc, const char** args)
{
    ops_t ops;
    ops.op = OP_INVALID;
    ops.args.dumpbin.baudrate = SBDOP_DEFAULT_BAUDRATE;
    ops.args.dumpbin.datamode = SBDOP_DEFAULT_DATAMODE;
    ops.args.dumpbin.delay = SBDOP_DEFAULT_DELAY;
    ops.args.dumpbin.burst = SBDOP_DEFAULT_BURST;
    ops.args.dumpbin.portname = NULL;
    ops.args.dumpbin.filename = NULL;

	for(int i = 0; i < argc; i++)
	{
	    if(argc == 1)
	    {
	        ops.op = OP_DISP_HELP;
	        break;
	    }
        if( ((argc == 2) && (strcmp(args[i], "--help") == 0)) ||
                ((argc == 2) && (strcmp(args[i], "-h") == 0)) )
        {
            ops.op = OP_DISP_HELP;
            break;
        }

        if(strcmp(args[i], "-l") == 0)
        {
            ops.op = OP_LIST_COMPORTS;
            break;
        }

        if(strcmp(args[i], "-b") == 0)
        {
            if((i+1) < argc)
            {
                ops.args.dumpbin.baudrate = args[i+1];
            }
            else
            {
                ops.op = OP_INVALID;
                break;
            }
        }

        if(strcmp(args[i], "-p") == 0)
        {
            if((i+1) < argc)
            {
                ops.args.dumpbin.portname = args[i+1];
            }
            else
            {
                ops.op = OP_INVALID;
                break;
            }
        }

        if(strcmp(args[i], "-dm") == 0)
        {
            if((i+1) < argc)
            {
                ops.args.dumpbin.datamode = args[i+1];
            }
            else
            {
                ops.op = OP_INVALID;
                break;
            }
        }

        if(strcmp(args[i], "-f") == 0)
        {
            if((i+1) < argc)
            {
                ops.args.dumpbin.filename = args[i+1];
            }
            else
            {
                ops.op = OP_INVALID;
                break;
            }
        }

        if(strcmp(args[i], "-dl") == 0)
        {
            if((i+1) < argc)
            {
                ops.args.dumpbin.delay = args[i+1];
            }
            else
            {
                ops.op = OP_INVALID;
                break;
            }
        }

        if(strcmp(args[i], "-bst") == 0)
        {
            if((i+1) < argc)
            {
                ops.args.dumpbin.burst = args[i+1];
            }
            else
            {
                ops.op = OP_INVALID;
                break;
            }
        }

        if(i == (argc-1))
        {
            // last loop iteration, no error and no other options: assume OP_DUMP_BINARY
            ops.op = OP_DUMP_BINARY;
        }

	}

	switch(ops.op)
	{
	    case OP_DISP_HELP:
	    {
	        SBDOP_DispHelpInfo();
	        break;
	    }
	    case OP_LIST_COMPORTS:
	    {
	        SBDOP_ListComPorts();
	        break;
	    }
	    case OP_DUMP_BINARY:
	    {
	        if(ops.args.dumpbin.portname == NULL)
	        {
	            printf("Error! Mandatory portname argument not given.\n");
	            break;
	        }
            if(ops.args.dumpbin.filename == NULL)
            {
                printf("Error! Mandatory filename argument not given.\n");
                break;
            }
            int portnum = -1;
	        if(!SBDOP_ValidComPort(ops.args.dumpbin.portname))
	        {
	            printf("Error! Com port with name: %s does not exist on this system.\n", ops.args.dumpbin.portname);
	            break;
	        }
	        else
	        {
	            portnum = SBDOP_GetComPortNumFromName(ops.args.dumpbin.portname);
	        }
	        int baud = SBDOP_GetBaudRateFromName(ops.args.dumpbin.baudrate);
	        if(baud == -1)
	        {
	            printf("Error! Baudrate: %s is not supported.\n", ops.args.dumpbin.baudrate);
	            break;
	        }
	        if(!SBDOP_ValidDataMode(ops.args.dumpbin.datamode))
	        {
	            printf("Error! Given data mode: %s is not supported.\n", ops.args.dumpbin.datamode);
	            break;
	        }
	        uint32_t filesize = 0;
	        if(!SBDOP_ValidFile(ops.args.dumpbin.filename, &filesize))
	        {
	            printf("Error! Cannot open file: %s.\nFile may not exist, is unable to be opened or exceeds size limit (%d bytes). \n",
	                    ops.args.dumpbin.filename,
	                    SBDOP_MAX_FILESIZE);
	            break;
	        }
	        int delay = SBDOP_GetDelayFromName(ops.args.dumpbin.delay);
	        if(delay == -1)
	        {
	            printf("Error! Given delay: %s is invalid.\n", ops.args.dumpbin.delay);
	            break;
	        }
	        if(delay > SBDOP_MAX_DELAYMS)
	        {
	            printf("Error! Given delay: %d too high. Max is: %d.\n", delay, SBDOP_MAX_DELAYMS);
	            break;
	        }
	        int burst = SBDOP_GetBurstFromName(ops.args.dumpbin.burst);
	        if(burst == -1 || burst == 0)
	        {
                printf("Error! Given burst: %s is invalid.\n", ops.args.dumpbin.burst);
                break;
	        }
	        if(!SBDOP_ValidBurst(burst, filesize))
	        {
                printf("Error! Given burst: %s is invalid.\n"
                        "It is either greater than filesize (%d bytes) or filesize is not"
                        "dividable by burst.\n",
                        ops.args.dumpbin.burst,
                        filesize);
                break;
	        }


	        printf("portname: %s.\n", ops.args.dumpbin.portname);
	        printf("baud: %d.\n", baud);
	        printf("delay: %d ms.\n", delay);
	        printf("datamode: %s.\n", ops.args.dumpbin.datamode);
	        printf("filename: %s.\n", ops.args.dumpbin.filename);
	        printf("filesize: %d bytes.\n", filesize);
	        printf("burst: %d bytes.\n", burst);

	        int ec = SBDOP_DumpBinaryToPort(
	                portnum,
	                baud,
	                delay,
	                burst,
	                ops.args.dumpbin.datamode,
	                ops.args.dumpbin.filename,
	                filesize);
	        if(ec == -1)
	        {
	            printf("Error! Dumping binary file to port failed.\n");
	        }
	        else
	        {
	            printf("Binary file successfully dumped to port.\n");
	        }

	        break;
	    }
	    default:
	    {
	        printf("Error! Bad or insufficient arguments given. \n");
	        break;
	    }
	}

    //system("pause");
    return EXIT_SUCCESS;
}

