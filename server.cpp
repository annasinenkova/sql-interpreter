#include "unsocket.hpp"
#include "parser.hpp"
#include <iostream>
#include <sstream>
#include <unistd.h>

const char * address = "mysocket";

class MyServerSocket : public UnServerSocket {
public:
    MyServerSocket () : UnServerSocket(address) {}
protected:
    void OnAccept(BaseSocket *);
};


void MyServerSocket::OnAccept(BaseSocket * pConn) {
    size_t size = pConn->GetSize();
    for (size_t k = 0; k < size; ++k) { 
        try {
            Parser parser(pConn);
            std::vector<std::vector<std::string> > t;
            parser.analyze(t);
            pConn->PutSize(t.size());
            if (t.size()) {
                pConn->PutSize(t[0].size());
            }
            else {
                pConn->PutSize(0);
            }
            for (size_t i = 0; i < t.size(); ++i) {
                for (size_t j = 0; j < t[i].size(); ++j) {
                    pConn->PutString(t[i][j]);
                }
            }
        }
        catch (Exception & e) {
            while(pConn->GetChar() != ';');
            pConn->PutSize(1);
            pConn->PutSize(1);
            pConn->PutString(e.GetMessage());
        }
    }
    delete pConn;
}


int main() {
    try {
        MyServerSocket sock;
        for (;;) {
            sock.Accept();
        }
    }
    catch (Exception & e) {
        e.Report();
    }
    return 0;
}

