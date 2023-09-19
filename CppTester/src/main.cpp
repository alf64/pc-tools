//============================================================================
// Name        : CppTester.cpp
// Author      : BM
// Version     :
// Copyright   : Freeware
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "cfuncs.h"
#include "CppFuncs.hpp"

using namespace std;

int main() {
	cout << "Hello! This is CppTester" << endl;

	printf("\n2 + 2 = %d \n", CFUNCS_Add(2, 2));

	CppFuncs cppf;
	cppf.PrintHi();

	CppFuncsChild1 cppf1;
	cppf1.PrintHi();

	CFUNCS_FnCall(CFUNCS_Fn);

    uint32_t sumResult = CFUNCS_Sum(56, 33);
    printf("sumResult = %d\n", sumResult);

	return 0;
}
