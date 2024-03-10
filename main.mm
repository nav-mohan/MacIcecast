#include <variant>
#include <iostream>
#include <string>
#include <memory>

#include "microphone.hpp"
#include "encoder.hpp"
#include "client.hpp"

// we need this for the default constructor of a std::variant
struct DummyMicrophone
{
    DummyMicrophone(){printf("DUMMY::CONSTRUCT\n");}
    void Start(){printf("DUMMY::Start\n");}
    void Stop(){printf("DUMMY::Stop\n");}
    ~DummyMicrophone(){printf("DUMMY::DESTORY\n");}
    void SetClient(const std::string host, const std::string port, const std::string endpoint, const std::string password, const std::string mime){}
    void StartClient(){}
};

// we are using unique_ptrs (or heap-allocated pointers) to prevent std::variant from destroying 
using AllPossibleMicPtrs = std::variant<
std::unique_ptr<DummyMicrophone >, // this prevents the default constructor of a std::variant from constructing an actual microphone
std::unique_ptr< Microphone<MPEGEncoder<ENCODERBITRATE::MEDIUM, ENCODERSAMPLERATE::MEDIUM, ENCODERCHANNEL::STEREO>, Client > >,
std::unique_ptr< Microphone<AACEncoder<ENCODERBITRATE::MEDIUM, ENCODERSAMPLERATE::MEDIUM, ENCODERCHANNEL::STEREO> , Client> >
>;

AllPossibleMicPtrs MicFactory(const std::string encoderType, boost::asio::io_context& ioc)
{
    AllPossibleMicPtrs mic;
    if(encoderType == "MPEG")
    {
        std::unique_ptr<Microphone<MPEGEncoder<ENCODERBITRATE::MEDIUM,ENCODERSAMPLERATE::MEDIUM,ENCODERCHANNEL::STEREO> , Client > > micPtr 
         = std::make_unique<Microphone<MPEGEncoder<ENCODERBITRATE::MEDIUM,ENCODERSAMPLERATE::MEDIUM,ENCODERCHANNEL::STEREO> , Client > >(ioc);
        mic = std::move(micPtr);
    }
    else 
    {
        std::unique_ptr<Microphone<AACEncoder<ENCODERBITRATE::MEDIUM> , Client > > micPtr 
        = std::make_unique<Microphone<AACEncoder<ENCODERBITRATE::MEDIUM> , Client > >(ioc);
        mic = std::move(micPtr);
    }
    return mic;
}

int main()
{
    printf("Starting...\n");
    boost::asio::io_context ioc;

    auto mpegMic = std::move(MicFactory("MPEG",ioc));
    std::visit([](auto&& mic)
    {
        mic->SetClient("localhost", "8000", "/stream", "hackme","audio/mpeg");
        mic->Start();
        mic->StartClient();
    },mpegMic);


    auto aacMic = std::move(MicFactory("AAC",ioc));
    std::visit([](auto&& mic)
    {
        mic->SetClient("localhost", "8000", "/RadioWestern", "hackme","audio/aac");
        mic->Start();
        mic->StartClient();
    },aacMic);
    
    ioc.run();

    // std::visit([](auto&& m){m->Stop();},aacMic);
}
