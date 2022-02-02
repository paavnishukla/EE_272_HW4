#ifndef INPUT_DOUBLE_BUFFER_H
#define INPUT_DOUBLE_BUFFER_H
#include <stdio.h>
#include <stdlib.h>

template <int size, int IC0, int OC0>
class InputDoubleBufferWriter{
public:
    InputDoubleBufferWriter(){}

    #pragma hls_design interface
    void CCS_BLOCK(run)(ac_channel<Params> &paramsIn,
                        ac_channel<PackedInt<INPUT_PRECISION, 4> > &din,
                        ac_channel<chanStruct<PackedInt<INPUT_PRECISION,IC0>,size> > &dout)
    {
        // -------------------------------
        // Your code starts here
        //first read the paramsIn
        Params params = paramsIn.read();
        printf("Am here\n");
        ac_int<32, false> tilesize = (params.IC1)*((params.OX0)*(params.STRIDE) + (params.FX))*((params.OY0)*(params.STRIDE) + (params.FY));
        chanStruct<PackedInt<INPUT_PRECISION,IC0>,size> tmp;

        for (int j = 0;j < (int)tilesize ;j++){
            //tmp.data[j] = 0;
            //tmp.data[j] = din.read() ;
            PackedInt<INPUT_PRECISION,IC0> entry ;
            for(int i = 0;i < IC0/4; i++){
                for(int k = 0;k < 4;k ++) {
                    entry.value[i*IC0/4 + k] = din.read().value[k];
                    printf("Hello");
                }
            }
            tmp.data[j] = entry;
 
        }
            dout.write(tmp);
            
        
        //read din, since its 4 bits at a time, I dont know why?, then dout it IC0 bits at the time 
        //Do this for all the input values.
        // Your code ends here
        // -------------------------------
    }
};

template <int size, int IC0, int OC0>
class InputDoubleBufferReader{
public:
    InputDoubleBufferReader(){}

    #pragma hls_design interface
    void CCS_BLOCK(run)(ac_channel<Params> &paramsIn,
                        ac_channel<chanStruct<PackedInt<INPUT_PRECISION, IC0>,size> > &din, 
                        ac_channel<PackedInt<INPUT_PRECISION, IC0> > &dout)
    {
        // -------------------------------
        // Your code starts here
        Params params = paramsIn.read();

        //ac_channel<chanStruct<PackedInt<INPUT_PRECISION, IC0>,size> > tmp;
        ac_int<32, false> tilesize = (params.IC1)*((params.OX0)*(params.STRIDE) + (params.FX))*((params.OY0)*(params.STRIDE) + (params.FY));
        chanStruct<PackedInt<INPUT_PRECISION, IC0>,size> tmp;
        tmp = din.read();
        for(int i = (int)tilesize-1;i >= 0;i--){
            dout.write(tmp.data[i]);
         }

        //tmp = din.read();
        //for(int i = 0;i <)
        // Your code ends here
        // -------------------------------
    }
};

template <int size, int IC0, int OC0>
class InputDoubleBuffer{
public:
  InputDoubleBuffer(){}

  #pragma hls_design interface
  void CCS_BLOCK(run)(ac_channel<PackedInt<INPUT_PRECISION, 4> > &inputs_in, 
                      ac_channel<PackedInt<INPUT_PRECISION, IC0> > &inputs_out,
                      ac_channel<Params> &paramsIn)
    {

        Params params = paramsIn.read();

        inputDoubleBufferReaderParams.write(params);
        inputDoubleBufferWriterParams.write(params);

        inputDoubleBufferWriter.run(inputDoubleBufferWriterParams, inputs_in, mem);

        inputDoubleBufferReader.run(inputDoubleBufferReaderParams, mem, inputs_out);
    }

private:
    ac_channel<chanStruct<PackedInt<INPUT_PRECISION, IC0>,size> > mem;
    
    InputDoubleBufferWriter<size, IC0, OC0> inputDoubleBufferWriter;
    ac_channel<Params> inputDoubleBufferWriterParams;
    
    InputDoubleBufferReader<size, IC0, OC0> inputDoubleBufferReader;
    ac_channel<Params> inputDoubleBufferReaderParams;
};

#endif
