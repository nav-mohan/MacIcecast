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
    FILE *outfile;

    HANDLE_AACENCODER   handle;
    int16_t *m_pcmBufferInternal;
    AACENC_BufDesc m_pcmBuffDesc    = { 0 };
    AACENC_InArgs m_pcmArgs         = { 0 };
    int m_pcmBuffIDs                = IN_AUDIO_DATA;
    int m_pcmBuffElSizes            = sizeof(int16_t);
    int m_miniPcmBuffSize           = 8192;     // fdk-aac can only encode 2048 samples/channels at a time for some reason
    int16_t *m_pcmBuffer; // using int16_t m_pcmBuffer[8192] causes an "invalid parameter error" in fdk-aac for some reason

    uint8_t* m_encBuffer;
    AACENC_BufDesc m_aacBuffDesc    = { 0 };
    AACENC_OutArgs m_aacArgs        = { 0 };
    int m_encBufferIdentifiers      = OUT_BITSTREAM_DATA;
    int out_size                    = 2*CALCULATE_CHANNEL_BUFFER_SIZE;
    int aac_bufElSizes              = sizeof(uint8_t);

    int m_channels                  = 2;
    int m_samplerate                = 44100;
    int m_bitrate                   = 128000;
    int m_bytesEncoded              = 0;

    AACEncoder()
    {
        outfile = fopen("blabla.aac","wb");
	    aacEncOpen(&handle, 0, m_channels);
	    aacEncoder_SetParam(handle, AACENC_AOT, 2);
	    aacEncoder_SetParam(handle, AACENC_SAMPLERATE, m_samplerate);
	    aacEncoder_SetParam(handle, AACENC_CHANNELMODE, MODE_2);
	    aacEncoder_SetParam(handle, AACENC_CHANNELORDER, 1);
        aacEncoder_SetParam(handle, AACENC_BITRATE, m_bitrate);
	    aacEncEncode(handle, NULL, NULL, NULL, NULL);


        m_pcmBufferInternal = (int16_t*)malloc(2*CALCULATE_CHANNEL_BUFFER_SIZE);
        m_pcmBuffer = (int16_t*)malloc(m_miniPcmBuffSize);
        m_pcmBuffDesc.numBufs = 1;
        m_pcmBuffDesc.bufs = (void**)&m_pcmBuffer;
        m_pcmBuffDesc.bufferIdentifiers = &m_pcmBuffIDs;
        m_pcmBuffDesc.bufElSizes = &m_pcmBuffElSizes;
        m_pcmBuffDesc.bufSizes = &m_miniPcmBuffSize;

        m_encBuffer = (uint8_t*)malloc(2*CALCULATE_CHANNEL_BUFFER_SIZE);
        m_aacBuffDesc.numBufs = 1;
        m_aacBuffDesc.bufs = (void**)&m_encBuffer;
        m_aacBuffDesc.bufferIdentifiers = &m_encBufferIdentifiers;
        m_aacBuffDesc.bufElSizes = &aac_bufElSizes;
        m_aacBuffDesc.bufSizes = &out_size;

    }

    ~AACEncoder()
    {
        if(outfile)         fclose(outfile);
        aacEncClose(&handle);
    }	


    constexpr int       DoEncodeInterleaved(const void *pcmBuffer, const uint32_t pcmBufferSize)
    {
        memset(m_pcmBufferInternal, 0, 2 * CALCULATE_CHANNEL_BUFFER_SIZE);
        memcpy(m_pcmBufferInternal, pcmBuffer, pcmBufferSize);
        int bytes_read = (int)pcmBufferSize;
        int err = 0;
        int total_pcm_samples = bytes_read/2;
        int sr = 0;
        int sl = total_pcm_samples; // pcm samples left to read
        int be = 0; // bytes encoded per loop
        printf("sl = %d\n",sl);
        // i'm making some goofbaclll mistake here
        // set the proper pcm args for inbufsize and insamples based on miniPcm, not the whole pcm
        int bytes_to_be_processed = std::min(sl,8192);
        while(sl >= 0)
        {
            bytes_to_be_processed = std::min(sl,8192);
            m_pcmArgs.numInSamples = bytes_to_be_processed <= 0 ? -1 : bytes_to_be_processed/2;
		    m_pcmBuffDesc.bufSizes = &bytes_to_be_processed;
            printf("MEMSETTING AAC %d\n",bytes_to_be_processed);
            memset(m_pcmBuffer, 0, bytes_to_be_processed);
            printf("MEMCOPYING AAC %d\n",bytes_to_be_processed);
            memcpy(m_pcmBuffer,m_pcmBufferInternal+sr*2,bytes_to_be_processed);
            printf("ENCODING AAC\n");
            err = aacEncEncode(handle, &m_pcmBuffDesc, &m_aacBuffDesc, &m_pcmArgs, &m_aacArgs);
            fwrite(m_encBuffer, 1, m_aacArgs.numOutBytes, outfile);
            sl -= m_aacArgs.numInSamples;
            sr += m_aacArgs.numInSamples;
            be += m_aacArgs.numOutBytes;
        }
        printf("AAC %d --> %d\n",total_pcm_samples, sr);
        m_bytesEncoded = be;
		return m_bytesEncoded;
    }
// ----------------------------------------------------------------------

}; // struct AACEncoder


#endif // AAC_ENCODER