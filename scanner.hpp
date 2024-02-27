#ifndef __SCANNER_HPP__
#define __SCANNER_HPP__

#include "exception.hpp"
#include "unsocket.hpp"
#include <iostream>
#include <vector>
#include <string>


class ScannerException : public Exception {
    static const std::vector<std::string> message;
public:
    enum ScannerExceptionCode {
        ESE_INCORRECT_CHAR
    };
    ScannerException(ScannerExceptionCode code) : Exception(code) {}
    std::string GetMessage();
};


enum lex_type {
    LEX_NULL,
    LEX_LIKE,
    LEX_IN,
    LEX_NUM,
    LEX_ID,
    LEX_STR,
    LEX_SELECT,
    LEX_INSERT,
    LEX_UPDATE,
    LEX_DELETE,
    LEX_CREATE,
    LEX_WHERE,
    LEX_TABLE,
    LEX_DROP,
    LEX_FROM,
    LEX_INTO,
    LEX_TEXT,
    LEX_LONG,
    LEX_SET,
    LEX_AND,
    LEX_NOT,
    LEX_ALL,
    LEX_OR,
    LEX_APOSTR,
    LEX_LPAREN,
    LEX_RPAREN,
    LEX_TIMES,
    LEX_COMMA,
    LEX_MINUS,
    LEX_SLASH,
    LEX_PLUS,
    LEX_CENT,
    LEX_NEQ,
    LEX_GEQ,
    LEX_LEQ,
    LEX_GTR,
    LEX_LSS,
    LEX_EQ,
    LEX_ADDR,
    LEX_OPEQ,
    LEX_END
};


class Lex {
    lex_type t_lex;
    std::string w_lex;
    long v_lex;
public:
    Lex(lex_type t = LEX_NULL, size_t v = 0) : t_lex(t), v_lex(v) {}
    Lex(lex_type t, std::string w) : t_lex(t), w_lex(w) {}
    long get_value();
    lex_type get_type();
    std::string get_text();
};


class Scanner {
    enum state { H, STR, IDENT, NUMB, ALE, NEQ, DELIM };
    static std::vector<std::pair<std::string, lex_type> > TD;
    static std::vector<std::pair<std::string, lex_type> > TW;
    state CS;
    char c;
    void gc();
    size_t look(const std::string &, std::vector<std::pair<std::string, lex_type> >);
    BaseSocket * sock;
public:
    Scanner(BaseSocket *);
    Lex get_lex();
};

#endif
