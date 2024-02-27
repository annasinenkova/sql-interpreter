#ifndef __EXCEPTION_HPP__
#define __EXCEPTION_HPP__

#include <string>
#include <iostream>

class Exception {
protected:
    size_t err_code;
public:
    Exception(size_t code) : err_code(code) {}
    void Report();
    virtual std::string GetMessage() = 0;
};
    
#endif
