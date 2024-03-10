#if !defined(AAC_ENCODER)
#define AAC_ENCODER

#include <string>
#include <fdk-aac/aacenc_lib.h>
#include "encoderenums.hpp"

template<
    ENCODERBITRATE eb = ENCODERBITRATE::MEDIUM ,
    ENCODERSAMPLERATE es = ENCODERSAMPLERATE::MEDIUM ,
    ENCODERCHANNEL ec = ENCODERCHANNEL::STEREO 
    >
struct AACEncoder
{
    const std::string   m_name          = "AAC";
    // FILE *outfile;

    HANDLE_AACENCODER   handle;
    int16_t *m_pcmBufferInternal;
    AACENC_BufDesc m_pcmBuffDesc    = { 0 };
    AACENC_InArgs m_pcmArgs         = { 0 };
    int m_pcmBuffIDs                = IN_AUDIO_DATA;
    int m_pcmBuffElSizes            = sizeof(int16_t);
    int m_pcmBuffSize               = 2*CALCULATE_CHANNEL_BUFFER_SIZE;     // this is probably overkill but keep in mind that fdk-aac can only encode 2048 samples/channels at a time
    int16_t *m_pcmBuffer            = nullptr; // using int16_t m_pcmBuffer[8192] causes an "invalid parameter error" in fdk-aac for some reason

    uint8_t* m_encBuffer;
    AACENC_BufDesc m_aacBuffDesc    = { 0 };
    AACENC_OutArgs m_aacArgs        = { 0 };
    int m_encBufferIdentifiers      = OUT_BITSTREAM_DATA;
    int out_size                    = 2*CALCULATE_CHANNEL_BUFFER_SIZE;
    int aac_bufElSizes              = sizeof(uint8_t);

    const int      m_channels          = static_cast<int>(ec);
    const int      m_samplerate        = static_cast<int>(es);
    const int      m_bitrate           = static_cast<int>(eb) * 1000; // FDK-AAC bitrate is specified in bits/sec
    int m_bytesEncoded              = 0;

    AACEncoder()
    {
        // outfile = fopen("blabla.aac","wb");
	    aacEncOpen(&handle, 0, m_channels);
	    aacEncoder_SetParam(handle, AACENC_AOT, 2);
	    aacEncoder_SetParam(handle, AACENC_SAMPLERATE, m_samplerate);
	    aacEncoder_SetParam(handle, AACENC_CHANNELMODE, MODE_2);
	    aacEncoder_SetParam(handle, AACENC_CHANNELORDER, 1);
        aacEncoder_SetParam(handle, AACENC_BITRATE, m_bitrate);
	    aacEncEncode(handle, NULL, NULL, NULL, NULL);


        m_pcmBufferInternal = (int16_t*)malloc(2*CALCULATE_CHANNEL_BUFFER_SIZE);
        m_pcmBuffer = (int16_t*)malloc(m_pcmBuffSize);
        m_pcmBuffDesc.numBufs = 1;
        m_pcmBuffDesc.bufs = (void**)&m_pcmBuffer;
        m_pcmBuffDesc.bufferIdentifiers = &m_pcmBuffIDs;
        m_pcmBuffDesc.bufElSizes = &m_pcmBuffElSizes;
        m_pcmBuffDesc.bufSizes = &m_pcmBuffSize;

        m_encBuffer = (uint8_t*)malloc(2*CALCULATE_CHANNEL_BUFFER_SIZE);
        m_aacBuffDesc.numBufs = 1;
        m_aacBuffDesc.bufs = (void**)&m_encBuffer;
        m_aacBuffDesc.bufferIdentifiers = &m_encBufferIdentifiers;
        m_aacBuffDesc.bufElSizes = &aac_bufElSizes;
        m_aacBuffDesc.bufSizes = &out_size;

    }

    ~AACEncoder()
    {
        // if(outfile)         fclose(outfile);
        aacEncClose(&handle);
    }	


    constexpr int       DoEncodeInterleaved(const void *pcmBuffer, const uint32_t pcmBufferSize)
    {
        memset(m_pcmBufferInternal, 0, 2 * CALCULATE_CHANNEL_BUFFER_SIZE);
        memcpy(m_pcmBufferInternal, pcmBuffer, pcmBufferSize);
        int err = 0;

        int bytes_to_be_processed = 8192; // FDK-AAC can only support 2048 samples per channel according to API documentation
        m_pcmArgs.numInSamples = bytes_to_be_processed <= 0 ? -1 : bytes_to_be_processed/2;
        m_pcmBuffDesc.bufSizes = &bytes_to_be_processed;
        // printf("MEMSETTING AAC %d\n",bytes_to_be_processed);
        memset(m_pcmBuffer, 0, bytes_to_be_processed);
        // printf("MEMCOPYING AAC %d\n",bytes_to_be_processed);
        memcpy(m_pcmBuffer,m_pcmBufferInternal,bytes_to_be_processed);
        err = aacEncEncode(handle, &m_pcmBuffDesc, &m_aacBuffDesc, &m_pcmArgs, &m_aacArgs);
        // fwrite(m_encBuffer, 1, m_aacArgs.numOutBytes, outfile);
        m_bytesEncoded = m_aacArgs.numOutBytes;
        return m_bytesEncoded;

    }
// ----------------------------------------------------------------------

}; // struct AACEncoder


#endif // AAC_ENCODER