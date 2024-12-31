#if !defined(MSLOGGER_H)
#define MSLOGGER_H

#include <streambuf>
#include <stdio.h>
#include <mutex>
#include <iostream>
#include <filesystem> // C++17
#include <fstream>
#include "utilities.h"

#define DEFAULT_BUFFER_SIZE 1024 // 1KB
#define DEFAULT_LOGROTATE_SIZE 5*1024*1024 // 5MB

template <LEVELS LOGLEVEL=INFO>
class MsLogger : public std::streambuf
{
private: // member variables
    static std::mutex mutCout_;
    static std::mutex mutFile_;
    static std::mutex mutBuffer_;
    static char *buffer_;
    static std::size_t bufferSize_;
    static std::ofstream file_;
    static uint16_t fileindex_;
    static std::string fileDate_;
    static std::string filepath_;
    void write_to_file();
    static void generate_filename();

protected:
    std::streamsize xsputn(const char_type* __s, std::streamsize __n) override;    
    int_type overflow(int_type __ch = traits_type::eof()) override;
    int sync() override;

public:
    static MsLogger<LOGLEVEL> &get_instance();
    static void presize_buffer(std::size_t bs); // set buffersize before instantiation. can increase/decrease buffersize
    void set_bufferSize(std::size_t bs); // set bufferSize after instantiation of logger. can only increase buffersize
    void log_to_stdout(std::string msg, LEVELS l=LOGLEVEL);
    void log_to_file(std::string msg, LEVELS l=LOGLEVEL);

private: // contructors
    MsLogger<LOGLEVEL>();
    ~MsLogger<LOGLEVEL>();
    MsLogger<LOGLEVEL>(const MsLogger<LOGLEVEL>&) = delete;
    MsLogger<LOGLEVEL>& operator= (const MsLogger<LOGLEVEL>&) = delete;



// public interfaces that would come in handy when running tests
// the alternative is to derive a fixture from MsLogger
#if defined(TESTING)
public:
    std::streamsize pubget_bufferSize() const {return bufferSize_;};
    enum LEVELS pubget_logLevel() const {return LOGLEVEL;};
    char *pubget_buffer() const {return buffer_;};
    void* pubget_pbase() const {return pbase();};
    std::size_t pubget_occupied() const {return (pptr() - pbase());}
    void* pubget_pptr() const {return pptr();};
    void* pubget_epptr() const {return epptr();};
#endif // TESTING

};


// instantating static member variables
template <LEVELS LOGLEVEL>
char *MsLogger<LOGLEVEL>::buffer_ = nullptr;

template <LEVELS LOGLEVEL>
uint16_t MsLogger<LOGLEVEL>::fileindex_ = 1;

template <LEVELS LOGLEVEL>
std::string MsLogger<LOGLEVEL>::fileDate_= generate_date();

template <LEVELS LOGLEVEL>
std::mutex MsLogger<LOGLEVEL>::mutCout_;

template <LEVELS LOGLEVEL>
std::mutex MsLogger<LOGLEVEL>::mutFile_;

template <LEVELS LOGLEVEL>
std::mutex MsLogger<LOGLEVEL>::mutBuffer_;

template <LEVELS LOGLEVEL>
std::ofstream MsLogger<LOGLEVEL>::file_;

template <LEVELS LOGLEVEL>
std::string MsLogger<LOGLEVEL>::filepath_;

template <LEVELS LOGLEVEL>
std::size_t MsLogger<LOGLEVEL>::bufferSize_ = DEFAULT_BUFFER_SIZE;



// Definitions of Templated Class

template <LEVELS LOGLEVEL>
MsLogger<LOGLEVEL>& MsLogger<LOGLEVEL>::get_instance()
{
    static MsLogger<LOGLEVEL> instance;
    return instance;
}

template<LEVELS LOGLEVEL>
void MsLogger<LOGLEVEL>::presize_buffer(std::size_t bs)
{
    std::lock_guard<std::mutex> blk(mutBuffer_);
    if(buffer_ == nullptr)
        bufferSize_ = bs;
    return; // quietly fails if buffer_ != nullptr
}

template <LEVELS LOGLEVEL>
void MsLogger<LOGLEVEL>::set_bufferSize(std::size_t bs)
{
    std::lock_guard<std::mutex> blk(mutBuffer_);
    if(bs < bufferSize_)
        return; // quietly fails if we try downsizing the buffer
    char *tmp = (char*)malloc(bufferSize_);
    memcpy(tmp,buffer_,bufferSize_);
    delete buffer_;
    buffer_ = (char*)malloc(bs);
    MsLogger<LOGLEVEL>::get_instance().setp(buffer_,buffer_+bs);
    memcpy(buffer_,tmp,bufferSize_);
    bufferSize_ = bs;
}

template<LEVELS LOGLEVEL>
void MsLogger<LOGLEVEL>::generate_filename()
{
    std::string today = generate_date();
    if(today != fileDate_) 
    {
        fileDate_ = today;
        fileindex_ = 1;
    }
    
    switch (LOGLEVEL)
    {
    case TRACE:
        filepath_ = std::move(today + (fileindex_>9?"_":"_0") + std::to_string(fileindex_) + ".trace.log");
        break;
    case DEBUG:
        filepath_ = std::move(today + (fileindex_>9?"_":"_0") + std::to_string(fileindex_) + ".debug.log");
        break;
    case INFO:
        filepath_ = std::move(today + (fileindex_>9?"_":"_0") + std::to_string(fileindex_) + ".info.log");
        break;
    case WARN:
        filepath_ = std::move(today + (fileindex_>9?"_":"_0") + std::to_string(fileindex_) + ".warn.log");
        break;
    case ERROR:
        filepath_ = std::move(today + (fileindex_>9?"_":"_0") + std::to_string(fileindex_) + ".error.log");
        break;
    default:
        filepath_ = std::move(today + (fileindex_>9?"_":"_0") + std::to_string(fileindex_) + ".log");
        break;
    }
    // std::cout << "GENERATED FILEPATH " << filepath_ << std::endl;
}

template <LEVELS LOGLEVEL>
MsLogger<LOGLEVEL>::MsLogger() 
{
    buffer_ = (char*)malloc(bufferSize_);
    // printf("Constructing %p (%ld,%d)\n",this,bufferSize_,LOGLEVEL);
    generate_filename();
    setp(buffer_,buffer_+bufferSize_);
}

template <LEVELS LOGLEVEL>
MsLogger<LOGLEVEL>::~MsLogger()
{
    // printf("Deleting %p (%ld,%d)\n",this,bufferSize_,LOGLEVEL);
    std::lock_guard<std::mutex> blk(mutBuffer_);
    if(buffer_ != nullptr)
    {
        if(pptr()-pbase() > 0)
            sync();
        delete buffer_;
    }
}

template <LEVELS LOGLEVEL>
void MsLogger<LOGLEVEL>::log_to_stdout(std::string msg, LEVELS l)
{
    std::lock_guard<std::mutex>lk(mutCout_);
    std::string tmp(colored.at(l) + generate_timestamp() + std::move(msg) + "\n");
    std::clog.write(tmp.c_str(),tmp.size());
}

template <LEVELS LOGLEVEL>
void MsLogger<LOGLEVEL>::log_to_file(std::string msg, LEVELS l)
{
    std::lock_guard<std::mutex> blk(mutBuffer_);
    std::string tmp(uncolored.at(l) + generate_timestamp() + std::move(msg) + "\n");
    sputn(tmp.c_str(),tmp.size());
}


template <LEVELS LOGLEVEL>
std::streamsize MsLogger<LOGLEVEL>::xsputn(const std::streambuf::char_type* __s, std::streamsize __n)
{
    if(epptr() - pptr() >= __n)
    {
        memcpy(pptr(),__s,__n);
        pbump(__n);
        return __n;
    }
    for(std::streamsize i=0 ; i < __n; i++)
    {
        if(traits_type::eq_int_type(sputc(__s[i]),traits_type::eof()))
            return i;
    }
    return __n;
}

template <LEVELS LOGLEVEL>
std::streambuf::int_type MsLogger<LOGLEVEL>::overflow(std::streambuf::int_type __ch)
{
    sync(); // flush the buffer
    if(traits_type::eq_int_type(__ch,traits_type::eof()))
        return traits_type::not_eof(__ch);
    buffer_[0] = __ch;
    pbump(1);
    return static_cast<int_type>(__ch);
}

template <LEVELS LOGLEVEL>
int MsLogger<LOGLEVEL>::sync()
{
    write_to_file();
    memset(pbase(),0,epptr()-pbase()); // reset the buffer_
    setp(buffer_,buffer_+bufferSize_);
    return 0;
}

template <LEVELS LOGLEVEL>
void MsLogger<LOGLEVEL>::write_to_file()
{
    std::lock_guard<std::mutex> flk(mutFile_);

    while(std::filesystem::exists(filepath_))
    {
        if(std::__fs::filesystem::file_size(filepath_) > DEFAULT_LOGROTATE_SIZE)
        {
            fileindex_++;
            generate_filename();
        }
        else break;
    }
    // log_to_stdout("WRITING TO FILE " + filepath_,ERROR);

    if(!std::__fs::filesystem::exists(filepath_) || !file_.is_open())
    {
        // log_to_stdout("CREATING AND OPENING " + filepath_,ERROR);
        file_.close();
        file_.open(filepath_,std::ios::app);
    }
    // log_to_stdout("FILE " + filepath_ + " IS READY",ERROR);
    // log_to_stdout("CURRENT FILEPATH = " + filepath_ ,ERROR);
    // log_to_stdout("FILESIZE = " + std::to_string(std::__fs::filesystem::file_size(filepath_)),ERROR);
    // log_to_stdout("BUFSIZE = " + std::to_string(pptr()-pbase()),ERROR);
    if(std::__fs::filesystem::file_size(filepath_) > DEFAULT_LOGROTATE_SIZE)
    {
        // log_to_stdout(filepath_ + "CREATING NEW FILE = " + std::to_string(fileindex_),ERROR);
        file_.close();
        fileindex_++;
        generate_filename();
        file_.open(filepath_,std::ios::app);
    }

    file_.write(buffer_,pptr()-pbase());
    file_.flush();
    // check goodbits, errorbits, failbits, 
    if(file_.good()) 
    {
        // log_to_stdout("GOODBIT " + std::to_string(file_.goodbit) + " WROTE " + std::to_string(pptr()-pbase()) ,INFO);
        return;
    }
    if(file_.bad()) return;
        // log_to_stdout("BADBIT " + std::to_string(file_.badbit),ERROR);
    if(file_.fail()) return;
        // log_to_stdout("FAILBIT " + std::to_string(file_.failbit),WARN);
    if(file_.eof()) return;
        // log_to_stdout("EOFBIT " + std::to_string(file_.eofbit),INFO);
}

static void basic_log(std::string msg, LEVELS l=INFO)
{
    MsLogger<INFO>::get_instance().log_to_file(msg,l);
    MsLogger<INFO>::get_instance().log_to_stdout(msg,l);
}

#endif // MsLogger_H