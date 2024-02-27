#ifndef __POLIZ_HPP__
#define __POLIZ_HPP__

#include "exception.hpp"
#include "scanner.hpp"
#include "table.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <stack>


class PolizException : public Exception {
    static const std::vector<std::string> message;
public:
    enum PolizExceptionCode {
        ESE_INCORRECT_TYPE
    };
    PolizException(PolizExceptionCode code) : Exception(code) {}
    std::string GetMessage();
};


enum st_type { BOOL, WORD, NUM };

struct st_elem {
    st_elem(long lp) : l(lp) {}
    st_elem(std::string tp) : t(tp) {}
    long l;
    std::string t; 
};


class Poliz {
    std::vector<Lex> p;
public:
    std::stack<st_elem> st;
    std::stack<st_type> tp_st;
    void put_lex(Lex);
    void clear();
    bool calculate(Table *, const std::vector<long> &, const std::vector<std::string> &);
};

#endif
