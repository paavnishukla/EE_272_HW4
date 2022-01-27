#ifndef SERIALIZER_H
#define SERIALIZER_H

template<typename DTYPE, typename DTYPE_SERIAL, int OC0, int accumbuffersize>
class Serializer{
public:
    Serializer(){}

    #pragma hls_design interface
    #pragma hls_pipeline_init_interval 1
    void CCS_BLOCK(run)(ac_channel<DTYPE> &inputChannel,
                        ac_channel<DTYPE_SERIAL> &serialOutChannel)
        {
            #ifndef __SYNTHESIS__
            while(inputChannel.available(1))
            #endif
            {
            DTYPE_SERIAL buffer[accumbuffersize][OC0];

                #pragma hls_pipeline_init_interval 1
                for(int i = 0; i < accumbuffersize; i++){
                    DTYPE input = inputChannel.read();
                    #pragma hls_unroll yes
                    for(int j = 0; j < OC0; j++){
                        buffer[i][j] = input.value[j];
                    }
                }


                #pragma hls_pipeline_init_interval 1
                for(int i = 0; i < accumbuffersize; i++){
                    for(int j = 0; j < OC0; j++){
                        serialOutChannel.write(buffer[i][j]);
                    }
                }
            }
        }
    };


#endif
