/**********************************************************************
    * mulacc.hpp -- a multiply-accumulate process                     *
    *                                                                 *
    * Author:  Hosein Attarzadeh (shan2@kth.se)                       *
    *                                                                 *
    * Purpose: Demonstration of a simple sequential processes.        *
    *                                                                 *
    * Usage:   MulAcc example                                         *
    *                                                                 *
    * License: BSD3                                                   *
    *******************************************************************/


#ifndef MULACC_HPP
#define MULACC_HPP

#include <forsyde.hpp>
#include "mul.hpp"
#include "add.hpp"

using namespace ForSyDe;

SC_MODULE(mulacc)
{
    SY::in_port<int>  a, b;
    SY::out_port<int> result;
    
    SY::signal<int> addi1, addi2, acci;
    
    SC_CTOR(mulacc)
    {
        
        SY::make_scomb2("mul1", mul_func, addi1, a, b);
        //SY::make_sdelay("accum", 0, addi2, acci);
        
        #ifndef FORSYDE_COSIMULATION_WRAPPERS
            auto add1 = SY::make_scomb2("add1", add_func, acci, addi1, addi2);
            SY::make_sdelay("accum", 0, addi2, acci);
            add1->oport1(result);
        #else
            // auto add1 = SY::make_pipewrap2("add1", 0, "simulink1", acci, addi1, addi2);
            // add1->oport1(result);

            auto addANDaccum1 = SY::make_pipewrap("pip_example", -1  , "pip_example", acci, addi1);
            addANDaccum1->oport1(result);
        #endif
        
        
        
    }
};

#endif
