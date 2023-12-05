#if !defined(CLIENT_HPP)
#define CLIENT_HPP

#define READ_BUFFERSIZE 1600

#include <boost/asio.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <functional>

#include "decodeutils.hpp"

struct Client
{
private:
    std::string m_host;         // = "localhost";
    std::string m_port;         // = "8000";
    std::string m_endpoint;     // = "/stream";
    std::string m_password;     // = "hackme";
    std::string m_mime;         // = "audio/mpeg"; // audio/aac
    std::string m_metadata;     // = "Radio Western";
    std::string m_header;       
    
    boost::asio::io_context& ioContext_;
    boost::asio::ip::tcp::resolver resolver_;
    std::vector<boost::asio::ip::tcp::endpoint> endpoints_;
    std::vector<boost::asio::ip::tcp::endpoint>::iterator curEndpoint_;
    boost::asio::ip::tcp::socket tcpSocket_;
    char tempBuffer_[READ_BUFFERSIZE];

    void DoResolve();
    void DoConnect();
    void DoAuthenticate();
    void DoReadAuthResponse();
    void DoRead();
    void DoReconnect();
    void DoDisconnect();
    void DoWait(uint32_t sec = 5);


public:
    Client(boost::asio::io_context& ioc);
    // Client();
    ~Client();
    void SetMountpoint(const std::string host, const std::string port, const std::string endpoint, const std::string password, const std::string mime);
    void inline start(){DoResolve();}
    void DoWrite(const void *buffer, size_t len);
};

#endif // CLIENT_HPP
