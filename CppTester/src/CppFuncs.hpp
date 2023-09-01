/*
 * CppFuncs.hpp
 *
 *  Created on: 31 sie 2023
 *      Author: Bartosz
 */

#ifndef CPPFUNCS_HPP_
#define CPPFUNCS_HPP_

#include <stdio.h>

class CppFuncs
{
public:
    CppFuncs(void);
    ~CppFuncs(void);

    void PrintHi(void);

private:
    bool isInitialized;
};


class CppFuncsChild1: public CppFuncs
{
public:
    CppFuncsChild1(void);
    ~CppFuncsChild1(void);

private:

};


#endif /* CPPFUNCS_HPP_ */
