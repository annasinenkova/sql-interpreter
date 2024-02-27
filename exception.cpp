#include "exception.hpp"
#include <string>
#include <iostream>

void Exception::Report() {
    std::cout << GetMessage() << std::endl;
}


