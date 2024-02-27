#ifndef __UNSOCKET_HPP__
#define __UNSOCKET_HPP__

#include "exception.hpp"
#include <iostream>
#include <string>
#include <vector>


class SocketException : public Exception {
    static const std::vector<std::string> message;
public:
    enum SocketExceptionCode {
        ESE_SUCCESS,
	    ESE_SOCKCREATE,
        ESE_SOCKCONN,
        ESE_SOCKILLEGAL,
        ESE_SOCKHOSTNAME,
        ESE_SOCKSEND,
        ESE_SOCKRECV,
        ESE_SOCKBIND,
        ESE_SOCKLISTEN,
        ESE_SOCKACCEPT,
        ESE_SOCKWRITE,
        ESE_SOCKREAD
    };
    SocketException(SocketExceptionCode code) : Exception(code) {}
    std::string GetMessage();
};


class SocketAddress {
protected:
    struct sockaddr * addr;
public:
    SocketAddress() : addr(NULL) {}
    virtual ~SocketAddress() {}
    virtual size_t GetLength() = 0;
    virtual SocketAddress * Clone() = 0;
    operator struct sockaddr *();
};


class UnSocketAddress : public SocketAddress {
public:
    UnSocketAddress(const char *);
    SocketAddress * Clone();
    size_t GetLength();
    ~UnSocketAddress();
};


class BaseSocket {
public:
    explicit BaseSocket(int desc = -1, SocketAddress * addr = NULL) : sock_desc(desc), sock_addr(addr) {}
    void Write(void *, size_t);
    void PutChar(char);
    void PutSize(size_t);
    void PutString(const char *);
    void PutString(const std::string &);
    int Read(void *, size_t);  
    char GetChar();
    size_t GetSize();
    std::string GetString();
    int GetSockDescriptor();
    ~BaseSocket();
protected:
    int sock_desc;
    SocketAddress * sock_addr;
};


class ClientSocket : public BaseSocket {
public:
    void Connect();
};
    

class UnClientSocket : public ClientSocket {
public:
    UnClientSocket(const char *);
};
    

class ServerSocket : public BaseSocket {
protected:
    void Bind();
    void Listen();
    virtual void OnAccept(BaseSocket *) {}
public:
    BaseSocket * Accept();
};
    
    
class UnServerSocket : public ServerSocket {
public:
    UnServerSocket(const char *);           
};
    

#endif
