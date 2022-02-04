#ifndef SYSTOLIC_ARRAY_CORE_H
#define SYSTOLIC_ARRAY_CORE_H

#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/comparison/not_equal.hpp>
#include <boost/preprocessor/repetition/for.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/tuple/size.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/punctuation/comma.hpp>
#include <boost/preprocessor/arithmetic/dec.hpp>

#include "ProcessingElement.h"
#include "Fifo.h"


struct LoopIndices{
    uint_16 ic1_idx;
    uint_16 fx_idx;
    uint_16 fy_idx;
};



template <typename IDTYPE, typename WDTYPE, typename ODTYPE, int OC0, int IC0>
class SystolicArrayCore
{
public:
    SystolicArrayCore() {}

#pragma hls_design interface
    void CCS_BLOCK(run)(
        ac_channel<PackedInt<INPUT_PRECISION, IC0> > &input, 
        ac_channel<PackedInt<WEIGHT_PRECISION, OC0> > &weight, 
        ac_channel<PackedInt<OUTPUT_PRECISION, OC0> > &output,
        ac_channel<Params> &paramsIn,
        ac_channel<LoopIndices> &loopIndicesIn)
    {
        #ifndef __SYNTHESIS__
        //assert(params.OX0 * params.OY0 < ACCUMULATION_BUFFER_SIZE);
        // Debug example:
        // printf("paramsIn channel size: %d\n", paramsIn.size());
        // printf("weigh channel size: %d\n", weight.size());
        // printf("input channel size: %d\n\n", input.size());
        #endif

        #ifndef __SYNTHESIS__
        while(paramsIn.available(1))
        #endif
        {
            // -------------------------------
            // Read in the params and loop indices from the channel
            // Your code starts here
            Params params = paramsIn.read();
            LoopIndices loopindices = loopIndicesIn.read();

            // Your code ends here
            // -------------------------------


            // -------------------------------
            // Create a loop for a "run" of the systolic array.
            // The number of steps in a run of the systolic array is equal to:
            // the ramp-up time + number of pixels + flush time
            // Your code starts here
            //for (int i = 0; i < 2*ARRAY_DIMENSION - 1 ; i++) {
            int rampUpTime = ARRAY_DIMENSION;
            int flushTime = ARRAY_DIMENSION-1;
            int numPixels = (int)(params.OX0*params.OY0) ;
            WDTYPE weightsArray[ARRAY_DIMENSION][ARRAY_DIMENSION];            
            for (int h = 0; h < rampUpTime+numPixels+flushTime; h++) {
            // Your code ends here 
            // You should now be in the body of the loop
            // -------------------------------

                // -------------------------------
                // If you are in the ramp up time, read in weights from the channel
                // and store it in the weights array
                // Your code starts here
                 if (h < rampUpTime) {
                         if (!weight.available(1)) {
                            printf("Weight reader in systolic array is the error");
                        }
                    PackedInt<WEIGHT_PRECISION, OC0> temp = weight.read();
                    for (int j = 0; j < ARRAY_DIMENSION-1; j++)
                    {
                        for (int k = 0; k < ARRAY_DIMENSION; k++)
                        {
                            weightsArray[j+1][k] = weightsArray[j][k];
                            weightsArray[0][k] = temp.value[k];
                        }                 
                    }            
                }
                // -------------------------------
                
                
                PackedInt<INPUT_PRECISION, IC0> in_col;

                // -------------------------------
                // Read inputs from the channel and store in the variable in_col
                // Note: you don't read in any inputs during the flush time
                // Your code starts here
                if(h < (numPixels)){
                        if (!input.available(1)) {
                            printf("Input read in systolic array is the error,corresponding h is %d",h);
                            
                        }
                in_col = input.read();
                }
                // Your code ends here
                // -------------------------------

                // Debug example:        
                // printf("in_col: %s\n", in_col.to_string());
 

                /*
                 * FIFOs for inputs coming in to the systolic array
                 * assign values to in_col, and the skewed version will be in input_buf
                 */
                PackedInt<INPUT_PRECISION, IC0> input_buf;


                #define INPUT_FIFO_BODY(z,i,unused) \
                    IDTYPE BOOST_PP_CAT(input_fifo_output_, i); \
                    IDTYPE BOOST_PP_CAT(input_fifo_input_, i) = in_col.value[i]; \
                    BOOST_PP_CAT(input_fifo_, i).run( BOOST_PP_CAT(input_fifo_input_, i) , BOOST_PP_CAT(input_fifo_output_, i) ); \
                    input_buf.value[i] = BOOST_PP_CAT(input_fifo_output_, i);
                
                REPEAT(INPUT_FIFO_BODY)

                // -------------------------------
                // Assign values from input_buf into the registers for the first column of PEs
                // Your code starts here
                 for (int j = 0; j<ARRAY_DIMENSION;j++) {
                    inputReg1[j][0] = input_buf.value[j];
                }
 
                // Your code ends here
                // -------------------------------

                PackedInt<OUTPUT_PRECISION, OC0> psum_buf;
                
                // -------------------------------
                // Set partial outputs for the array to psum_buf.
                // Depending on the loop index, the partial output will be 0 or a value from the accumulation buffer
                // Your code starts here
                for (int j = 0; j<ARRAY_DIMENSION; j++){
                    if (loopindices.fx_idx == 0 && loopindices.fy_idx == 0 && loopindices.ic1_idx == 0) {
                        psum_buf.value[j] = 0;
                    }
                    else {
                        psum_buf.value[j] = accumBuf[h-(rampUpTime+flushTime)][j];
                    }
                }

                // Your code ends here
                // -------------------------------
                
                // Debug example:
                // printf("psum_buf: %s\n", psum_buf.to_string());
                /*
                 * FIFOs for partial outputs coming in to the systolic array
                 * assign values to psum_buf, and the skewed version will be in output_buf
                 */
                PackedInt<OUTPUT_PRECISION, OC0> output_buf;
                #define ACCUM_FIFO_BODY(z,h,unused) \
                    ODTYPE BOOST_PP_CAT(psum_fifo_output_, h); \
                    ODTYPE BOOST_PP_CAT(psum_fifo_input_, h) = psum_buf.value[h]; \
                    BOOST_PP_CAT(psum_fifo_, h).run( BOOST_PP_CAT(psum_fifo_input_, h) , BOOST_PP_CAT(psum_fifo_output_, h) ); \
                    output_buf.value[h] = BOOST_PP_CAT(psum_fifo_output_, h);
                
                REPEAT(ACCUM_FIFO_BODY)
        
                // -------------------------------
                // Assign values from output_buf into the partial sum registers for the first row of PEs
                // Your code starts here
                for (int j = 0; j<ARRAY_DIMENSION;j++) {
                    psum_reg[0][j] = output_buf.value[j];
                }
                //}
                // Your code ends here
                // -------------------------------
            

                // -------------------------------
                // Run the 16x16 PE array
                // Make sure that the correct registers are given to the PE
                // Your code starts here
                //

                for (int j = 0; j < ARRAY_DIMENSION; j++) {
                    for (int k = 0; k < ARRAY_DIMENSION; k++) {
                        PE_Array[j][k].run(inputReg1[j][k], psum_reg[j][k], weightsArray[j][k], inputReg2[j][k], psum_reg2[j][k]);
                       // psum_reg[j][k] = psumReg2[j][k];
                    }
                }
                // Your code ends here
                // -------------------------------
                

                /*
                 * FIFOs for partial outputs coming out of the systolic array
                 * The skewed version will be in the variable output_row
                 */
               //   //

                PackedInt<OUTPUT_PRECISION, OC0> output_row;
                 #define FIFO_WRITE_BODY_NEW(z,i,unused)\
                    ODTYPE BOOST_PP_CAT(accum_fifo_output_, i); \
                    BOOST_PP_CAT(accum_fifo_, i).run( psum_reg[IC0][i] , BOOST_PP_CAT(accum_fifo_output_, i) );\
                    output_row.value[i] = BOOST_PP_CAT(accum_fifo_output_,i); \
                
                REPEAT(FIFO_WRITE_BODY_NEW)

                // -------------------------------
                // After a certain number of cycles, you will have valid output from the systolic array
                // Depending on the loop indices, this valid output will either be written into the accumulation buffer or written out
                // Your code starts here
                //for (int j = 0;j < ARRAY_DIMENSION;j ++) {
                
                
                    if((loopindices.fx_idx == (params.FX-1)) && (loopindices.fy_idx == params.FY-1) && (loopindices.ic1_idx == params.IC1-1)) {
                        output.write(output_row)  ;
                    }
                    else if (h>(rampUpTime+flushTime)) {
                        for (int j = 0; j < ARRAY_DIMENSION; j++) {
                        accumBuf[h-(rampUpTime+flushTime)][j] = psum_reg2[ARRAY_DIMENSION-1][j];
                    }
                   
                    }
                //}
                // for (int j = 0; j < ARRAY_DIMENSION; j++) {
                //     if (i > rampUpTime && i < rampUpTime+numberPixels) {
                //         accumBuf[0][j] = psum_reg;
                //     }
                // }                // Your code ends here
                // -------------------------------
                
                // -------------------------------
                // Cycle the input/psum registers
                // That is, the outputs that a PE wrote to should now become the input for the next PE
                // Your code starts here
                 for (int j = 0; j < ARRAY_DIMENSION; j++) {
                    for (int k = 1; k < ARRAY_DIMENSION; k++) {
                        inputReg1[j][k] = inputReg2[j][k-1];
                        psum_reg[k][j] = psum_reg2[k-1][j];
                    }
                }
                // Your code ends here
                // -------------------------------
            }
        }
    
        // Debug example:
        // printf("outputs written: %d\n", output.size());
    }

private:
    
    // -------------------------------
    // Create the following:
    //  - PE array
    ProcessingElement<IDTYPE,ODTYPE> PE_Array[ARRAY_DIMENSION][ARRAY_DIMENSION]; //PE[][].run()
    IDTYPE inputReg1[ARRAY_DIMENSION][ARRAY_DIMENSION];
    IDTYPE inputReg2[ARRAY_DIMENSION][ARRAY_DIMENSION];
    ODTYPE psum_reg2[ARRAY_DIMENSION][ARRAY_DIMENSION];
    ODTYPE psum_reg[ARRAY_DIMENSION][ARRAY_DIMENSION];
  //  ODTYPE psum_reg[ARRAY_DIMENSION][rampUpTime+numPixels+flushTime]; //??
    WDTYPE weightReg[ARRAY_DIMENSION][ARRAY_DIMENSION];
    ODTYPE accumBuf[ACCUMULATION_BUFFER_SIZE][ARRAY_DIMENSION];
    
    //  - accumulation buffer
    //  - weight registers
    //  - input registers (two sets, one at the input of the PE and one at the output) 
    //  - psum registers (two sets, one at the input of the PE and one at the output) 
    // Your code starts here

    // Your code ends here
    // -------------------------------
    

#define INPUT_FIFOS_INIT(z, i, unused) \
    Fifo<IDTYPE, i + 1> BOOST_PP_CAT(input_fifo_, i);

    REPEAT(INPUT_FIFOS_INIT)

#define ACCUM_FIFOS_INIT(z, i, unused) \
    Fifo<ODTYPE, i + 1> BOOST_PP_CAT(psum_fifo_, i);

    REPEAT(ACCUM_FIFOS_INIT)
    

#define OUTPUT_FIFOS_INIT(z, i, unused) \
    Fifo<ODTYPE, OC0 - i> BOOST_PP_CAT(accum_fifo_, i);
    
    REPEAT(OUTPUT_FIFOS_INIT)
};

#endif
