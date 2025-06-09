#ifndef REPORT_HPP
#define REPORT_HPP

#include <forsyde.hpp>
#include <iostream>

using namespace ForSyDe;

void reportI(const CTTYPE& v)
{   
    #pragma ForSyDe begin report
        std::cout << sc_time_stamp().to_seconds() << " s: i = " << v << std::endl;
    #pragma ForSyDe end
}
void reportW(const CTTYPE& v)
{   
    #pragma ForSyDe begin report
        std::cout << sc_time_stamp().to_seconds() << " s: w = " << v << std::endl;
    #pragma ForSyDe end
}
#endif