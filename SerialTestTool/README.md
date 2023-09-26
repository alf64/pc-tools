# SerialTestTool

Command line application for testing devices over serial protocol.
Physical interface: UART.
Data protocol: AGP (ALF64 General Protocol).

If device under test implements AGP over UART, you can use this app to test:
 * if device implementation of AGP is correct
 * some of device functionality 

### Resources
Software:
 - minGW tools (installed and added to PATH so the Eclipse can recognize them)
 - Eclipse cpp-2022-12-R-win32-x86_64

### How to start developing
Open the project in the Eclipse IDE, edit & build/debug.

### How to start using this app
1. Connect the device under test to your PC via UART <-> USB converter.
2. Run the application in command line using administrative privileges. This app needs it because it opens COM port on your PC.
3. Run the application with no arguments to see help.

### Additional information
The code is organized in a way, that there is GeneralDevice class representing the general functions that each AGP device shall implement.
You can think of it as mandatory functionality.
Particular device (i.e.: SensX) is represented by a class that derives from GeneralDevice class.
Therefore such device contains mandatory functionalities and also adds its own specific ones.
