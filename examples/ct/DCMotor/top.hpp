#ifndef TOP_HPP
#define TOP_HPP

#include <systemc>
#include <vector>
#include <forsyde.hpp>
#include "report.hpp"
//#include "inV_func.hpp"

using namespace sc_core;
using namespace ForSyDe::CT;

void in_func(CTTYPE& out1, const sc_time& inp1)
{
#pragma ForSyDe begin in_func
   // if (inp1.to_seconds() > 5)
        out1 = 5.0;// * inp1.to_seconds();
    // else 
    //     out1 = 0;    
#pragma ForSyDe end
}

SC_MODULE(top){

    CT2CT v_in , v_out;    
    CT2CT i_in , i_out1, i_out2;   
    CT2CT w_out;    

    SC_CTOR(top){

        const double L = 1.1e-4;    
        const double R = 1.7;     
        const double J = 3.88e-7;   
        const double b = 2.75e-4;  
        const double Kt = 0.0059; 
        const double Kb = 0.0274;    

        std::vector<CTTYPE> num_i = { J, b };
        std::vector<CTTYPE> num_w = { Kt } ;

        std::vector<CTTYPE> den = { (J * L),(J * R + b * L),(b * R + Kb * Kt) };
        
        //I(s)/V(s) = (J s + b) / [J L s^2 + (J R + b L) s + (b R + K^2)]
        //omega(s)/V(s) = (k/J) / [s + (b/J)] 

        make_fanout2("fan_i", i_in, i_out2, i_out1);

        make_source("stimuli_v", in_func, sc_time(10,SC_SEC), v_in);

        make_delay("dalay", sc_time(10, SC_US) ,v_out ,v_in);

        make_filterf( "filter_i", num_i, den, sc_time(10, SC_SEC), i_out1, v_out);
        make_filterf( "filter_w", num_w, den, sc_time(10, SC_US), w_out, i_in );

        make_sink("sink_i",  reportI, sc_time(50, SC_US), i_out2);
        make_sink("sink_w",  reportW, sc_time(50, SC_US), w_out);

    }
    
    #ifdef FORSYDE_INTROSPECTION
    void start_of_simulation()
    {
        ForSyDe::XMLExport dumper("gen/");
        dumper.traverse(this);
    }
    #endif

};

#endif 
