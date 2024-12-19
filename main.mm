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
std::unique_ptr< Microphone<AACEncoder<ENCODERBITRATE::MEDIUM, ENCODERSAMPLERATE::MEDIUM, ENCODERCHANNEL::STEREO> , Client> >,
std::unique_ptr< Microphone<AACEncoder<ENCODERBITRATE::HIGH, ENCODERSAMPLERATE::MEDIUM, ENCODERCHANNEL::STEREO> , Client> >
>;

AllPossibleMicPtrs MicFactory(const std::string encoderType, boost::asio::io_context& ioc)
{
    AllPossibleMicPtrs mic;
    if(encoderType == "MPEG MED")
    {
        std::unique_ptr<Microphone<MPEGEncoder<ENCODERBITRATE::MEDIUM,ENCODERSAMPLERATE::MEDIUM,ENCODERCHANNEL::STEREO> , Client > > micPtr 
         = std::make_unique<Microphone<MPEGEncoder<ENCODERBITRATE::MEDIUM,ENCODERSAMPLERATE::MEDIUM,ENCODERCHANNEL::STEREO> , Client > >(ioc);
        mic = std::move(micPtr);
    }
    else if(encoderType == "AAC MED")
    {
        std::unique_ptr<Microphone<AACEncoder<ENCODERBITRATE::MEDIUM> , Client > > micPtr 
        = std::make_unique<Microphone<AACEncoder<ENCODERBITRATE::MEDIUM> , Client > >(ioc);
        mic = std::move(micPtr);
    }
    else if (encoderType == "AAC HIGH")
    {
        std::unique_ptr<Microphone<AACEncoder<ENCODERBITRATE::HIGH> , Client > > micPtr 
        = std::make_unique<Microphone<AACEncoder<ENCODERBITRATE::HIGH> , Client > >(ioc);
        mic = std::move(micPtr);
    }

    return mic;
}

int main()
{
    printf("Starting...\n");
    boost::asio::io_context ioc1;
    boost::asio::io_context ioc2;
    boost::asio::io_context ioc3;

    auto mpegMic = std::move(MicFactory("MPEG MED",ioc1));
    std::visit([](auto&& mic)
    {
        mic->SetClient("localhost", "8000", "/stream", "hackme","audio/mpeg");
        mic->Start();
        mic->StartClient();
    },mpegMic);


    auto aacMicMed = std::move(MicFactory("AAC MED",ioc2));
    std::visit([](auto&& mic)
    {
        mic->SetClient("localhost", "8000", "/RadioWestern", "hackme","audio/aac");
        mic->Start();
        mic->StartClient();
    },aacMicMed);


    auto aacMicHigh = std::move(MicFactory("AAC HIGH",ioc3));
    std::visit([](auto&& mic)
    {
        mic->SetClient("localhost", "8000", "/RadioWestern2", "hackme","audio/aac");
        mic->Start();
        mic->StartClient();
    },aacMicHigh);


    std::thread t1([&](){ioc1.run();});
    std::thread t2([&](){ioc2.run();});
    std::thread t3([&](){ioc3.run();});

    t1.join();
    t2.join();
    t3.join();

    // std::visit([](auto&& m){m->Stop();},aacMic);
}
