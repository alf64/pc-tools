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

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "App.hpp"

using namespace std;

/*
 * Currently only build for Windows OS is possible.
 */
#ifndef _WIN32
#error "This app is meant to be build for Windows."
#endif

int main(int argc, const char** argv) {
    int retval = EXIT_FAILURE;

    printf("%s started.\n", App::GetAppName());

	App app;
	ec_t ec = app.Process(argc, argv);
	if(ec == EC_OK)
	{
	    cout << "Program terminated successfully." << endl;
	    retval = EXIT_SUCCESS;
	}
	else
	{
        cout << "Program terminated with failure!!!" << endl;
	}

	return retval;
}
