#ifndef WEIGHT_DOUBLE_BUFFER_H
#define WEIGHT_DOUBLE_BUFFER_H


template <int size, int IC0, int OC0>
class WeightDoubleBufferWriter{
public:
    WeightDoubleBufferWriter(){}

    #pragma hls_design interface
    void CCS_BLOCK(run)(ac_channel<Params> &paramsIn,
                        ac_channel<PackedInt<WEIGHT_PRECISION, 4> > &din,
                        ac_channel<chanStruct<PackedInt<WEIGHT_PRECISION, OC0>, size> > &dout)
    {
        // -------------------------------
        // Your code starts here
        Params params = paramsIn.read();
        //printf("Am here\n");

        ac_int<32, false> tilesize = (params.IC1)*(IC0)*(params.FX)*(params.FY);
        chanStruct<PackedInt<WEIGHT_PRECISION,OC0>,size> tmp;
        for(int k = 0;k < params.OY1;k++){
            for(int l = 0;l < params.OX1;l++)
                for(int m = 0;m < params.OC1;m++) {

                    for (int j = 0;j < tilesize ;j++){
                        //tmp.data[j] = 0;
                        //tmp.data[j] = din.read() ;
                        PackedInt<WEIGHT_PRECISION,OC0> entry;
                        for(int i = 0;i < OC0/4; i++){
                            for(int n = 0; n < 4;n++){
                            //printf("Hello");
                                if (!din.available(1)) {
                                        printf("Weight double buffer writer is the error");
                                    }
                            entry.value[i*IC0/4 + n] =  din.read().value[n] ;
                        }
                    }

                    tmp.data[j] = entry;
                }

                dout.write(tmp);
            }
        }
      

        // Your code ends here
        // -------------------------------
    }
};

template <int size, int IC0, int OC0>
class WeightDoubleBufferReader{
public:
    WeightDoubleBufferReader(){}

    #pragma hls_design interface
    void CCS_BLOCK(run)(ac_channel<Params> &paramsIn,
                        ac_channel<chanStruct<PackedInt<WEIGHT_PRECISION, OC0>,size> > &din, 
                        ac_channel<PackedInt<WEIGHT_PRECISION, OC0> > &dout)
    {
        // -------------------------------
        // Your code starts here
        Params params = paramsIn.read();
        ac_int<32, false> tilesize = (params.IC1)*(IC0)*(params.FX)*(params.FY);
        chanStruct<PackedInt<INPUT_PRECISION, OC0>,size> tmp;
            if (!din.available(1)) {
                            printf("Weight double buffer reader is the error");
                        }
        tmp = din.read();
        for(int k = 0;k < params.OY1;k++){
            for(int l = 0;l < params.OX1;l++)
                for(int m = 0;m < params.OC1;m++) {

                    for(int i = 0;i < tilesize;i++){
                        dout.write(tmp.data[i]);
                    
                }
            }
        }
        // Your code ends here
        // -------------------------------
    }
};

template <int size, int IC0, int OC0>
class WeightDoubleBuffer{
public:
  WeightDoubleBuffer(){}

  #pragma hls_design interface
  void CCS_BLOCK(run)(ac_channel<PackedInt<WEIGHT_PRECISION, 4> > &weights_in, 
                      ac_channel<PackedInt<WEIGHT_PRECISION, OC0> > &weights_out,
                      ac_channel<Params> &paramsIn)
    {
        Params params = paramsIn.read();

        // #ifndef __SYNTHESIS__
        // ac_int<ac::log2_ceil<size>::val, false> block_size = IC0*params.IC1*params.FX*params.FY;
        // assert(block_size <= size);
        // #endif

        weightDoubleBufferReaderParams.write(params);
        weightDoubleBufferWriterParams.write(params);

        weightDoubleBufferWriter.run(weightDoubleBufferWriterParams, weights_in, mem);
        weightDoubleBufferReader.run(weightDoubleBufferReaderParams, mem, weights_out);
    }

private:
    ac_channel<chanStruct<PackedInt<WEIGHT_PRECISION, OC0>,size> > mem;
    
    WeightDoubleBufferWriter<size, IC0, OC0> weightDoubleBufferWriter;
    ac_channel<Params> weightDoubleBufferWriterParams;
    
    WeightDoubleBufferReader<size, IC0, OC0> weightDoubleBufferReader;
    ac_channel<Params> weightDoubleBufferReaderParams;
};


#endif
