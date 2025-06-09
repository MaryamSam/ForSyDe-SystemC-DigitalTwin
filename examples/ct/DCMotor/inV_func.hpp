#ifndef INV_FUNC_HPP
#define INV_FUNC_HPP

#include <systemc>
#include <vector>
#include <forsyde.hpp>

using namespace sc_core;
using namespace ForSyDe::CT;

void in_func(CTTYPE& out1, const sc_time& inp1)
{
#pragma ForSyDe begin in_func
    // if (inp1.to_seconds() > 5)
        out1 = 1.0 * inp1.to_seconds();
    // else 
    //     out1 = 0;    
#pragma ForSyDe end
}

#endif 