#include "unsocket.hpp"
#include <iostream>
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <vector>


const std::vector<std::string> SocketException::message = {"SUCCESS", "SOCKCREATE", "SOCKCONN",
                             "SOCKILLEGAL", "SOCKHOSTNAME", "SOCKSEND", "SOCKRECV", "SOCKBIND",
                             "SOCKLISTEN", "SOCKACCEPT", "SOCKWRITE", "SOCKREAD"};


std::string SocketException::GetMessage() {
    return "Socket: " + message[this->err_code];
}


SocketAddress::operator struct sockaddr * () {
    return this->addr;
}


UnSocketAddress::UnSocketAddress(const char * sock_name) {
    this->addr = new struct sockaddr;
    this->addr->sa_family = AF_UNIX;
    strcpy(this->addr->sa_data, sock_name);
}
      
       
size_t UnSocketAddress::GetLength() {
    return sizeof(*(this->addr));
}


SocketAddress * UnSocketAddress::Clone() {
    SocketAddress * newaddr = new UnSocketAddress(this->addr->sa_data);
    return newaddr;
}


UnSocketAddress::~UnSocketAddress() {
    delete this->addr;
}


#define WRITE(desc, buf, len) \
    if (write(desc, buf, len) != (int)(len)) { \
        throw SocketException(SocketException::ESE_SOCKWRITE); \
    } \


void BaseSocket::Write(void * buf, size_t len) {
    WRITE(this->sock_desc, &len, sizeof(len));
    WRITE(this->sock_desc, buf, len);
}

void BaseSocket::PutChar(char c) {
    WRITE(this->sock_desc, &c, sizeof(c));
}


void BaseSocket::PutSize(size_t s) {
    WRITE(this->sock_desc, &s, sizeof(s));
}


void BaseSocket::PutString(const char * str) {
    size_t len = strlen(str) + 1;
    WRITE(this->sock_desc, &len, sizeof(len));
    WRITE(this->sock_desc, str, len);
}


void BaseSocket::PutString(const std::string & str) {
    size_t len = strlen(str.c_str()) + 1;
    WRITE(this->sock_desc, &len, sizeof(len));
    WRITE(this->sock_desc, str.c_str(), len);
}


#define READ(desc, buf, len) \
    if (read(desc, buf, len) != (int)(len)) { \
        throw SocketException(SocketException::ESE_SOCKREAD); \
    } \


int BaseSocket::Read(void * buf, size_t len) {
    int n = read(this->sock_desc, buf, len);
    if (n != (int)len) {
        throw SocketException(SocketException::ESE_SOCKREAD);
    }
    return n;
}        


char BaseSocket::GetChar() {
    char c = 0;
    READ(this->sock_desc, &c, sizeof(c));
    return c;
}


size_t BaseSocket::GetSize() {
    size_t s = 0;
    READ(this->sock_desc, &s, sizeof(s));
    return s;
}


std::string BaseSocket::GetString() {
    size_t len;
    READ(this->sock_desc, &len, sizeof(len));
    std::string str;
    str.resize(len);
    READ(this->sock_desc, &str[0], len);
    return str;  
}


int BaseSocket::GetSockDescriptor() {
    return this->sock_desc;
}


BaseSocket::~BaseSocket() {
    close(this->sock_desc);
    delete this->sock_addr;
}


void ClientSocket::Connect() {
    if (connect(this->sock_desc, *(this->sock_addr), this->sock_addr->GetLength()) < 0) { 
        throw SocketException(SocketException::ESE_SOCKCONN);
    }
}


UnClientSocket::UnClientSocket(const char * address) {
    this->sock_addr = new UnSocketAddress(address);
    if ((this->sock_desc = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        throw SocketException(SocketException::ESE_SOCKCREATE);
    }
}


void ServerSocket::Bind() {
    if (bind(this->sock_desc, *(this->sock_addr), this->sock_addr->GetLength()) < 0) {
        throw SocketException(SocketException::ESE_SOCKBIND);
    }    
}
        
        
void ServerSocket::Listen() {
    if (listen(this->sock_desc, 1) < 0) {
        throw SocketException(SocketException::ESE_SOCKLISTEN);
    }
}


BaseSocket * ServerSocket::Accept() {
    Listen();
    int newdesc;
    if ((newdesc = accept(this->sock_desc, NULL, NULL)) < 0) {
        throw SocketException(SocketException::ESE_SOCKACCEPT);
    }
    BaseSocket * ptr = new BaseSocket(newdesc, this->sock_addr->Clone());
    OnAccept(ptr);
    return ptr;
}


UnServerSocket::UnServerSocket(const char * address) {
    unlink(address);
    this->sock_addr = new UnSocketAddress(address);
    if ((this->sock_desc = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        throw SocketException(SocketException::ESE_SOCKCREATE);
    }
    Bind();
}

