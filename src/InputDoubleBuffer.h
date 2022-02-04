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
        //printf("Am here\n");
        ac_int<32, false> tilesize = (params.IC1)*((params.OX0-1)*(params.STRIDE) + (params.FX))*((params.OY0-1)*(params.STRIDE) + (params.FY));
        chanStruct<PackedInt<INPUT_PRECISION,IC0>,size> tmp;

        for(int k = 0;k < params.OY1;k++) {
            for(int m = 0;m < params.OX1;m++){
                    for (int j = 0;j < (int)tilesize ;j++){
                        PackedInt<INPUT_PRECISION,IC0> entry ;
                        for(int i = 0;i < IC0/4; i++){
                            PackedInt<INPUT_PRECISION,4> store ;
                            if (!din.available(1)) {
                            printf("Input double buffer  store in writer  is the error");
                            }
                            store = din.read();

                            for(int l = 0;l < 4;l ++) {
                                if (!din.available(1)) {
                                    printf("Input double buffer writer is the error");
                                }
                                
                                entry.value[i*4 + l] = store.value[l];
                               // printf("params FX- %d,FY- %d, IC1- %d, OC1 - %d, OX0 - %d, OX1 - %d, OY0 = %d, OY1 - %d, STRIDE- %d",params.FX,params.FY,params.IC1,params.OC1,params.OX0,params.OX1,params.OY0,params.OY1,params.STRIDE);
                            }
                        } 
            
                    tmp.data[j] = entry;
                    }
                dout.write(tmp);
                printf("size is %d",dout.size());
        
            }
                
        }
            
        
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
        printf("size of din is %d\n",din.size());
        //ac_channel<chanStruct<PackedInt<INPUT_PRECISION, IC0>,size> > tmp;
      //  ac_int<32, false> tilesize = (params.IC1)*((params.OX0-1)*(params.STRIDE) + (params.FX))*((params.OY0-1)*(params.STRIDE) + (params.FY));
        chanStruct<PackedInt<INPUT_PRECISION, IC0>,size> tmp;
        int adr_ix0,adr_iy0, config_IX0,config_IY0,adr;
       
        for(int k = 0;k < params.OY1;k++) {
            for(int m = 0;m < params.OX1;m++){
                if (!din.available(1)) {
                    printf("Input double buffer reader is the error");
                    }
                tmp = din.read();
                for(int l = 0;l < params.OC1;l++){
  
                    for (int ic1 = 0; ic1 < params.IC1; ic1++) {
                        //tmp = din.read();
                        for (int fy = 0; fy < params.FY;fy++){
                            for (int fx = 0; fx < params.FX;fx++){
                                for (int oy0 = 0; oy0 < params.OY0; oy0++) {
                                    for (int ox0 = 0; ox0 < params.OX0; ox0++) {
                                        if((fx == 0 && fy == 0)||(l == 0)) {
                                   
                                        adr_ix0 = params.STRIDE*ox0+fx;
                                        adr_iy0 = params.STRIDE*oy0+fy;
                                        config_IX0 = (params.OX0-1)*(params.STRIDE)+(params.FX);
                                        config_IY0 = (params.OY0-1)*(params.STRIDE)+(params.FY);

                                        adr = ic1*config_IX0*config_IY0+adr_iy0*config_IX0+adr_ix0;
                                        dout.write(tmp.data[adr]);
                                        } else {
                                           // adr_ix0 = params.STRIDE*ox0;
                                            //adr_iy0 = params.STRIDE*oy0;
                                        dout.write(tmp.data[adr]);
                                        }
                                        //printf("Values are %d",adr);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    printf("Size of reader dout is %d",dout.size());
                    
        
        

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
