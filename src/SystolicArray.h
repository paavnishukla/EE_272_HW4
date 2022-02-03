#ifndef SYSTOLIC_ARRAY_H
#define SYSTOLIC_ARRAY_H

#include "ProcessingElement.h"
#include "conv.h"
#include "Fifo.h"
#include "SystolicArrayCore.h"

// Include mc_scverify.h for CCS_* macros
#include <mc_scverify.h>

class SystolicArrayLooper
{
public:
    SystolicArrayLooper() {}

#pragma hls_design interface
void run(ac_channel<Params> &paramsIn,
         ac_channel<Params> &paramsOut,
         ac_channel<LoopIndices> &loopIndicesOut)
    {
        // -------------------------------
        // Generate the loop indices here for the systolic array.
        // Write the loop indices as well as the params out to channels.
        // Your code starts here
        Params params;
        params = paramsIn.read();
        LoopIndices loopindices;
        // loopindices.fx_idx = 0;
        // loopindices.fy_idx = 0;
        // loopindices.ic1_idx = 0;
        for(uint_16 o = 0;o < params.OY1;o++) {
            for(uint_16 n = 0;n < params.OX1;n++){
                for(uint_16 m = 0;m < params.OC1;m++){
                    for(uint_16 i = 0;i < params.IC1;i++){
                        for(uint_16 j = 0;j < params.FY;j++){
                            for(uint_16 k = 0;k < params.FX;k++){
                                loopindices.ic1_idx = i;
                                loopindices.fy_idx = j;
                                loopindices.fx_idx = k;
                                paramsOut.write(params);
                                loopIndicesOut.write(loopindices);
                                
                            }
                        }
                    }
                }
            }
        }
        // Your code ends here
        // -------------------------------
    }
};

template <typename IDTYPE, typename WDTYPE, typename ODTYPE, int OC0, int IC0>
class SystolicArrayWrapper
{
public:
    SystolicArrayWrapper(){}
    
#pragma hls_design interface
    void run(ac_channel<PackedInt<INPUT_PRECISION, IC0> > &input, 
             ac_channel<PackedInt<WEIGHT_PRECISION, OC0> > &weight, 
             ac_channel<PackedInt<OUTPUT_PRECISION, OC0> > &output,
             ac_channel<Params> &paramsIn)
    {
        systolicArrayLooper.run(paramsIn, paramsChannel, loopIndicesChannel);
        systolicArrayCore.run(input, weight, output, paramsChannel, loopIndicesChannel);
    }
private:
    SystolicArrayCore<IDTYPE, WDTYPE, ODTYPE, OC0, IC0> systolicArrayCore;
    SystolicArrayLooper systolicArrayLooper;
    ac_channel<Params> paramsChannel;
    ac_channel<LoopIndices> loopIndicesChannel;
};

#endif
