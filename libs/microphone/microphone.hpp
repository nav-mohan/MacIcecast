#if !defined(MICROPHONE_HPP)
#define MICROPHONE_HPP

#include <iostream>

#include <Foundation/Foundation.h>
#include <CoreFoundation/CoreFoundation.h>
#include <AudioToolbox/AudioToolbox.h>

#include <boost/asio.hpp>
#include "mslogger.hpp"

// Custom UserData
static const int kNumberBuffers = 3;
template <class Encoder, class Client>
struct UserData
{
    AudioStreamBasicDescription m_streamDesc;
    AudioQueueRef               m_queue;
    AudioQueueBufferRef         m_buffers[kNumberBuffers];
    Encoder                     m_encoder;
    Client                      m_client;
    uint32_t                    m_bufferByteSize;
    int64_t                     m_currentPacket;
    bool                        m_isRunning;
    UserData(boost::asio::io_context& ioc) : m_client(ioc){}
};

#define SAMPLERATE 44100.0
#define CHANNELS 2
#define BITDEPTH 16 

template <class Encoder, class Client>
struct Microphone
{
    UserData<Encoder,Client> m_userData;
    Microphone(boost::asio::io_context& ioc);
    ~Microphone();
    static void Callback( void*, AudioQueueRef, AudioQueueBufferRef, const AudioTimeStamp*, uint32_t inNumPackets, const AudioStreamPacketDescription*);
    void Start();
    void Stop();
    const uint32_t DeriveBufferSize(const AudioStreamBasicDescription& aStreamDesc, const float seconds); 
    void SetClient(const std::string host, const std::string port, const std::string endpoint, const std::string password, const std::string mime);
    void StartClient();
};

template <class Encoder,class Client>

Microphone<Encoder,Client>::Microphone(boost::asio::io_context& ioc) : m_userData(ioc)
{
    m_userData.m_streamDesc.mFormatID = kAudioFormatLinearPCM;
    m_userData.m_streamDesc.mSampleRate = m_userData.m_encoder.m_samplerate;
    m_userData.m_streamDesc.mChannelsPerFrame = m_userData.m_encoder.m_channels;
    m_userData.m_streamDesc.mBitsPerChannel = BITDEPTH;
    m_userData.m_streamDesc.mBytesPerPacket = m_userData.m_encoder.m_channels * BITDEPTH/8;
    m_userData.m_streamDesc.mBytesPerFrame = m_userData.m_encoder.m_channels * BITDEPTH/8;
    m_userData.m_streamDesc.mFramesPerPacket = 1;
    // m_userData.m_streamDesc.mFormatFlags = kLinearPCMFormatFlagIsBigEndian | kLinearPCMFormatFlagIsSignedInteger| kLinearPCMFormatFlagIsPacked;
    m_userData.m_streamDesc.mFormatFlags = kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;

    AudioQueueNewInput(
        &m_userData.m_streamDesc,
        Callback,
        &m_userData,
        nullptr,
        kCFRunLoopCommonModes,
        0,
        &m_userData.m_queue
    );
    
    uint32_t streamDescSize = sizeof(m_userData.m_streamDesc);
    AudioQueueGetProperty(
        m_userData.m_queue, 
        kAudioQueueProperty_StreamDescription,
        &m_userData.m_streamDesc,
        &streamDescSize
    );
    // m_userData.m_bufferByteSize = DeriveBufferSize(m_userData.m_streamDesc, 0.5);
    // printf("m_bufferByteSize = %u\n",m_userData.m_bufferByteSize);
    for(int i = 0; i < kNumberBuffers; i++)
    {
        AudioQueueAllocateBuffer(m_userData.m_queue, 4096, &m_userData.m_buffers[i]); // FDK-AAC can only support 2048 samples/channel according to API documentation
        AudioQueueEnqueueBuffer(m_userData.m_queue, m_userData.m_buffers[i],0,nullptr);
    }
    basic_log("CONSTRUCT MICROPHONE - "
    + m_userData.m_encoder.m_name + ", "
    + std::string(m_userData.m_encoder.m_channels == 1 ? "Mono, " : "Stereo, ")
    + std::to_string( m_userData.m_encoder.m_name == "MPEG"? m_userData.m_encoder.m_bitrate : m_userData.m_encoder.m_bitrate/1000) + " Kbps, "
    + std::to_string(m_userData.m_encoder.m_samplerate)+ " Hz"
    ,DEBUG); 
}
template <class Encoder, class Client>
Microphone<Encoder,Client>::~Microphone()
{
    basic_log("DESTROY MICROPHONE - "
    + m_userData.m_encoder.m_name + ", "
    + std::string(m_userData.m_encoder.m_channels == 1 ? " Mono, " : " Stereo, ")
    + std::to_string( m_userData.m_encoder.m_name == "MPEG"? m_userData.m_encoder.m_bitrate : m_userData.m_encoder.m_bitrate/1000) + " Kbps, "
    + std::to_string(m_userData.m_encoder.m_samplerate)+ " Hz"
    ,DEBUG); 
    Stop();
}

template <class Encoder,class Client>
void Microphone<Encoder,Client>::Callback
( 
    void* userdata,
    AudioQueueRef inAQ,
    AudioQueueBufferRef inBuffer,
    const AudioTimeStamp* inStartTime,//the time at which the buffer's first sample was recorded
    uint32_t inNumPackets,//0 for CBR. ~23 for 0.5 secs of aac
    const AudioStreamPacketDescription*  inPacketDesc//meta-info about aac/mp3
)
{
    UserData<Encoder,Client>* ud =  static_cast<UserData<Encoder,Client>*>(userdata);
    
    inNumPackets = inBuffer->mAudioDataByteSize / ud->m_streamDesc.mBytesPerPacket;
    ud->m_currentPacket += inNumPackets;
    ud->m_encoder.DoEncodeInterleaved(inBuffer->mAudioData,inBuffer->mAudioDataByteSize);
    ud->m_client.DoWrite(ud->m_encoder.m_encBuffer, ud->m_encoder.m_bytesEncoded);
    if (ud->m_isRunning == 0) return;
    AudioQueueEnqueueBuffer(ud->m_queue, inBuffer, 0, NULL);
}

template <class Encoder, class Client>
void Microphone<Encoder,Client>::Start()
{
    basic_log("STARTING MICROHPHEN",INFO);
    m_userData.m_currentPacket = 0;
    m_userData.m_isRunning = true;
    AudioQueueStart(m_userData.m_queue, nullptr);
}

template <class Encoder, class Client>
void Microphone<Encoder,Client>::Stop()
{
    m_userData.m_isRunning = false;
    OSStatus err = noErr;
    err = AudioQueueStop(m_userData.m_queue, true);
    err = AudioQueueDispose(m_userData.m_queue, true);
}

template <class Encoder, class Client>
const uint32_t Microphone<Encoder,Client>::DeriveBufferSize(const AudioStreamBasicDescription& aStreamDesc, const float seconds)
{
    const uint32_t maxBufferSize = 0x50000; // 5 * 16^4
    const uint32_t maxPacketSize = aStreamDesc.mBitsPerChannel/8;
    const float numBytesForTime = aStreamDesc.mSampleRate * maxPacketSize * seconds;
    basic_log("numBytesForSie = " + std::to_string(numBytesForTime), INFO);
    if(numBytesForTime < maxBufferSize)
        return static_cast<uint32_t>(numBytesForTime);
    return static_cast<uint32_t>(maxBufferSize);
}

template <class Encoder, class Client>
void Microphone<Encoder,Client>::SetClient(const std::string host, const std::string port, const std::string endpoint, const std::string password, const std::string mime)
{
    m_userData.m_client.SetMountpoint(host,port,endpoint,password,mime);
    m_userData.m_client.start();
}

template <class Encoder, class Client>
void Microphone<Encoder,Client>::StartClient()
{
    m_userData.m_client.start();
}



#endif // MICROPHONE_HPP
