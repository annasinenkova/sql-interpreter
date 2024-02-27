#include "unsocket.hpp"
#include <iostream>
#include <iomanip>
#include <cstdio>

const char * address = "mysocket";

int main() {
    try {
        UnClientSocket sock(address);
        sock.Connect();
        char c;
        size_t size;
        std::cin >> size;
        sock.PutSize(size);
        for (size_t k = 0; k < size; ++k) {
            do {
                c = getchar();
                std::cout << c;
                sock.PutChar(c);
            } while (c != ';');
            std::cout << std::endl;
            size_t im = sock.GetSize();
            size_t jm = sock.GetSize();
            for (size_t i = 0; i < im; ++i) {
                for (size_t j = 0; j < jm; ++j) {
                    std::cout << std::setw(20) << sock.GetString() << " ";
                }
                std::cout << std::endl;
            }
        }
    }
    catch (Exception & e) {
        e.Report();
    }
    return 0;
}

