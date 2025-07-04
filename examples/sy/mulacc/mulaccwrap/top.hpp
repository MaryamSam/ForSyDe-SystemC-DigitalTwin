/**********************************************************************
    * Top.hpp -- the top module and testbench for the mulacc example  *
    *                                                                 *
    * Author:  Hosein Attarzadeh (shan2@kth.se)                       *
    *                                                                 *
    * Purpose: Demonstration of a simple sequential processes.        *
    *                                                                 *
    * Usage:   MulAcc example                                         *
    *                                                                 *
    * License: BSD3                                                   *
    *******************************************************************/

#include "report.hpp"
#include "siggen.hpp"
#include <iostream>

using namespace ForSyDe;

SC_MODULE(top)
{
    SY::signal<int> srca, srcb, result;
    
    SC_CTOR(top)
    {
        SY::make_sconstant("constant1", 3, 10, srca);
        
        SY::make_ssource("siggen1", siggen_func, 1, 10, srcb);
        
        // The mulacc is a socket-based cosimulation wrapper
        SY::make_socketwrap2("mulacc1", 0, "localhost", 1212, result, srca, srcb);
        
        SY::make_ssink("report1", report_func, result);
    }
#ifdef FORSYDE_INTROSPECTION
    void start_of_simulation()
    {
        ForSyDe::XMLExport dumper("gen/");
        dumper.traverse(this);
    }
#endif
};
