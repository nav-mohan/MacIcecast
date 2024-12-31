#include "client.hpp"
#include "mslogger.hpp"

Client::Client(boost::asio::io_context& ioc) :
    ioContext_(ioc)
,   tcpSocket_(ioContext_)
,   resolver_(ioContext_)
{
    // DoResolve();
}

Client::~Client(){}

void Client::SetMountpoint(const std::string host, const std::string port, const std::string endpoint, const std::string password, const std::string mime)
{
    m_host = host;
    m_port = port;
    m_endpoint = endpoint;
    m_password = password;
    m_mime = mime;

    std::string unhashed_http_auth = "source:"+m_password;
    char *hashed_http_auth = util_base64_encode(unhashed_http_auth.c_str());
    m_header = "PUT " + m_endpoint + " HTTP/1.1\n";
    m_header += "User-Agent: Lavf/59.27.100\n";
    m_header += "Accept: */*\n";
    m_header += "Expect: 100-continue\n";
    m_header += "Connection: close\n";
    m_header += "Host: " + m_host + ":" + m_port + "\n";
    m_header += "Content-Type: " + m_mime + "\n";
    m_header += "Icy-Metadata: " + m_metadata + "\n";
    m_header += "Ice-Public: 0\n";
    m_header += "Authorization: Basic ";
    m_header.append(hashed_http_auth);
    m_header += "\n\n";
    basic_log("CONNECTING & AUTHENTICATING "+ m_host + ":"+ m_port + "/"+m_endpoint);
}

void Client::DoResolve()
{
    // start resolving
    endpoints_.clear();
    resolver_.async_resolve
    (
        m_host,m_port,
        [this](const boost::system::error_code& ec, boost::asio::ip::tcp::resolver::iterator endpoint_iter)
        {
            basic_log("RESOLVING... "+ m_host + ":"+ m_port + "/"+m_endpoint);
            if(ec)
            {
                // DoReconnect();
                DoWait(5);
                DoResolve();
                basic_log("FAILED TO RESOLVE " + ec.what(),WARN);
            }
            else
            {
                basic_log("RESOLVED "+ m_host + ":"+ m_port + "/"+m_endpoint,INFO);
                for (; endpoint_iter != boost::asio::ip::tcp::resolver::iterator(); ++endpoint_iter) {
                    endpoints_.push_back(*endpoint_iter);
                }
                curEndpoint_ = endpoints_.begin();
                return DoConnect();
            }
        }
    );
}

void Client::DoConnect()
{
    // start connecting
    auto ep = *curEndpoint_;
    tcpSocket_.async_connect
    (
        ep,
        [this](const boost::system::error_code& ec)
        {
            basic_log("CONNECTING "+ m_host + ":"+ m_port + "/"+m_endpoint,INFO);
            if(!ec)
            {
                basic_log("CONNECTED "+ m_host + ":"+ m_port + "/"+m_endpoint,INFO);
                return DoAuthenticate();
            }
            else
            {
                basic_log("FAILED TO CONNECTED "+ m_host + ":"+ m_port + "/"+m_endpoint + " | " + ec.what(),WARN);
                return DoReconnect();
            }
        }
    );
}


void Client::DoAuthenticate()
{
    const char *buffer = m_header.c_str();
    std::size_t writeLen = m_header.size();
    boost::asio::async_write(
        tcpSocket_, boost::asio::buffer(buffer,writeLen), 
        [this,buffer](boost::system::error_code ec, std::size_t l)
        {
            if(!ec)
            {
                basic_log("AUTHENTICATED! "+ m_host + ":"+ m_port + "/"+m_endpoint + " | " + buffer ,INFO);
                return DoReadAuthResponse();
            }
            else 
            {
                basic_log("FAILED TO AUTHENTICATE "+ m_host + ":"+ m_port + "/"+m_endpoint + " | " + ec.what(),WARN);
            }
        }
    );
}

void Client::DoReadAuthResponse()
{
    // start reading
    memset(tempBuffer_,0,READ_BUFFERSIZE);
    tcpSocket_.async_receive
    (
        boost::asio::buffer(tempBuffer_,READ_BUFFERSIZE),
        [&](boost::system::error_code readErr, size_t readLen)
        {
            if(readErr) // this code path is only executed when the socket is disconnected. so just reconnect
            {
                basic_log("AUTH ERROR : "+ m_host + ":"+ m_port + "/"+m_endpoint + " | " + readErr.what() + " | " + std::string(tempBuffer_),WARN);
                DoDisconnect();
                return DoConnect();
            }
            else
            {
                // if it returns a 401 then check response and dont try to reconnect.
                basic_log("AUTH RESPONSE : "+ m_host + ":"+ m_port + "/"+m_endpoint + " | " + std::string(tempBuffer_),DEBUG);
                std::string response(tempBuffer_);
                if(response == "HTTP/1.1 100 Continue\r\nContent-Length: 0\r\n\r\n")
                    return DoReadAuthResponse();
                else if(response == "HTTP/1.0 200 OK\r\n\r\n")
                {
                    return DoRead(); // This is REQUIRED for keeping the connection open. Also, IceCast might say something that would be relevant to our state. 
                }
                // else if(response == "")
                else 
                    return;
            }
        }
    );    
}

void Client::DoRead()
{
    // start reading
    memset(tempBuffer_,0,READ_BUFFERSIZE);
    tcpSocket_.async_receive
    (
        boost::asio::buffer(tempBuffer_,READ_BUFFERSIZE),
        [&](boost::system::error_code readErr, size_t readLen)
        {
            if(readErr) // this code path is only executed when the socket is disconnected. so just reconnect
            {
                basic_log("READ ERROR: "+ m_host + ":"+ m_port + "/"+m_endpoint + " | " + readErr.what() + " | " + std::string(tempBuffer_),ERROR);
                DoDisconnect();
                return DoConnect();
            }
            else
            {
                // if it returns a 401 then check response and dont try to reconnect.
                basic_log("READ RESPONSE : "+ m_host + ":"+ m_port + "/"+m_endpoint + " | " + std::string(tempBuffer_),DEBUG);
                DoRead();
            }
        }
    );
}

void Client::DoWrite(const void *buffer, const std::size_t writeLen)
{
    // printf("WRITING %lu\n",writeLen);
    boost::asio::async_write(
        tcpSocket_, boost::asio::buffer(buffer,writeLen), 
        [this,buffer,writeLen](boost::system::error_code ec, std::size_t l)
        {
            if(!ec)
            {
                // printf("WROTE %lu\n",writeLen);
            }
            else 
            {
                basic_log("WRITE ERROR : "+ m_host + ":"+ m_port + "/"+m_endpoint + " | " + ec.what(),ERROR);
                if(ec ==  boost::system::errc::broken_pipe) DoReconnect();
            }
        }
    );
}

void Client::DoReconnect()
{
    DoDisconnect();
    DoWait(5);
    return DoConnect();
}

void Client::DoDisconnect()
{
    tcpSocket_.~basic_stream_socket();
    tcpSocket_ = boost::asio::ip::tcp::socket(ioContext_);
}

void Client::DoWait(uint32_t sec)
{
    boost::asio::steady_timer timer(ioContext_, std::chrono::seconds(sec));
    timer.wait();
}
