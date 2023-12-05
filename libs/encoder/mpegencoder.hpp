#if !defined(MPEG_ENCODER)
#define MPEG_ENCODER

#include <string>
#include <lame/lame.h>
#include "encoderenums.hpp"

template<
    ENCODERBITRATE eb = ENCODERBITRATE::MEDIUM ,
    ENCODERSAMPLERATE es = ENCODERSAMPLERATE::MEDIUM ,
    ENCODERCHANNEL ec = ENCODERCHANNEL::STEREO 
    >
struct MPEGEncoder
{
// ----------------------------------------------------------------------
    const std::string   m_name              = "MPEG";
    lame_global_flags   *m_lgf;
// ----------------------------------------------------------------------
    short               m_pcmBuffer         [2 * CALCULATE_CHANNEL_BUFFER_SIZE];
    unsigned char       m_encBuffer         [2 * CALCULATE_CHANNEL_BUFFER_SIZE];
    const uint32_t      m_bitrate           = static_cast<uint32_t>(eb);
    const uint32_t      m_samplerate        = static_cast<uint32_t>(es);
    const uint32_t      m_channels          = static_cast<uint32_t>(ec);
    int                 m_bytesEncoded      = 0;
// ----------------------------------------------------------------------

    constexpr int       DoEncodeInterleaved(const void *pcmBuffer, const uint32_t pcmBufferSize)
    {
        // printf("RECEIVED %d BYTES\n", pcmBufferSize);
        memset(m_pcmBuffer, 0, 2 * CALCULATE_CHANNEL_BUFFER_SIZE);
        memcpy(m_pcmBuffer, pcmBuffer, pcmBufferSize);

        m_bytesEncoded = lame_encode_buffer_interleaved(
            m_lgf,
            m_pcmBuffer,
            pcmBufferSize/4,
            m_encBuffer,
            2 * CALCULATE_CHANNEL_BUFFER_SIZE
        );
        // printf("MPEG ENCODED %d BYTES\n",m_bytesEncoded);
        // if(m_bytesEncoded > 0)
            // fwrite(m_encBuffer,1,m_bytesEncoded,outfile);
        return m_bytesEncoded;
    }
// ----------------------------------------------------------------------

    // FILE *outfile = nullptr;
    ~MPEGEncoder()
    {
        printf("MPEGEncoder::~MPEGEncoder()\n");
        if(m_lgf)   lame_close(m_lgf); 
        // if(outfile) fclose(outfile);
    }

    MPEGEncoder()
    {
        m_lgf = lame_init();
        if(m_channels == 1) lame_set_mode(m_lgf,MONO); else lame_set_mode(m_lgf,STEREO);
        lame_set_in_samplerate(m_lgf,m_samplerate);
        lame_set_brate(m_lgf,m_bitrate);
        lame_set_num_channels(m_lgf,m_channels);
        lame_init_params(m_lgf);
        // outfile = fopen("hahaha.mp3","wb");
    }
// ----------------------------------------------------------------------
}; // struct MPEGEncoder

#endif // MPEG_ENCODER
