#if !defined(ENCODER_ENUMS)
#define ENCODER_ENUMS

typedef enum class ENCODERCHANNEL {MONO=1 , STEREO=2} ENCODERCHANNEL ;
typedef enum class ENCODERBITRATE {LOW=64, MEDIUM=128, HIGH=256} ENCODERBITRATE ; 
typedef enum class ENCODERSAMPLERATE {LOW = 22050, MEDIUM = 44100, HIGH=48000} ENCODERSAMPLERATE;

// a macro for a tedious expression. The division by 2 is for 0.5 seconds buffer length
#define CALCULATE_CHANNEL_BUFFER_SIZE static_cast<uint32_t>(ec) * static_cast<uint32_t>(es)/2 

#endif // ENCODER_ENUMS
