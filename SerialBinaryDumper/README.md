# SerialBinaryDumper
SerialBinaryDumper is a tool to dump binary file to the serial port.

It simply takes binary file as an argument, opens a serial port of user's choice and sends byte by byte the contents of the file.
Additionaly, user can specify the following options:
a) A delay to be applied between each byte send.
b) A baudrate to be used with serial port (default is: 9600 bps, but many others are supported).
c) A datamode to be used with serial port (default is 8n1, but many others are supported).

Building under Linux.
make all

Building under Windows (requires gcc and make, i.e. from MinGW package).
make -f Makefile.win32 all

For details about usage, please call:
SerialBinaryDumper -h

Additional info.
This program uses an open source rs232 library files, provided by Teunis van Beelen.
Main source files are provided by alf64.
It is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License.
