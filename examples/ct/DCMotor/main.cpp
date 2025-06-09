//------------------------------------------------------------------------------
// sc_main:
//   1) Create a CT signal for the voltage input.
//   2) Instantiate CT_step_source and connect it to the CT input.
//   3) Instantiate top_dc_motor and connect its v_in to the step source.
//   4) Every 10 µs, read i(t) and ω(t) from the motor and print them.
//------------------------------------------------------------------------------

#include <systemc>
#include <forsyde.hpp>
#include "top.hpp"

using namespace sc_core;
using namespace ForSyDe;

int sc_main(int argc, char* argv[])
{
    top top("motor");

    sc_start();
        
    return 0;
}
