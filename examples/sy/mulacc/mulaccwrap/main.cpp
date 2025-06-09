/**********************************************************************
    * main.cpp -- the main file and testbench for the mulacc example  *
    *                                                                 *
    * Author:  Hosein Attarzadeh (shan2@kth.se)                       *
    *                                                                 *
    * Purpose: Demonstration of a simple sequential processes.        *
    *                                                                 *
    * Usage:   MulAcc example                                         *
    *                                                                 *
    * License: BSD3                                                   *
    *******************************************************************/

// Add ForSyDe-SystemC's cosimulation wrappers, if not already done
#ifndef FORSYDE_COSIMULATION_WRAPPERS
#define FORSYDE_COSIMULATION_WRAPPERS
#endif

#include "top.hpp"

int sc_main(int argc, char **argv)
{
    top top1("top1");

    sc_start();
        
    return 0;
}


