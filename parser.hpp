#ifndef __PARSER_HPP__
#define __PARSER_HPP__

#include "unsocket.hpp"
#include "scanner.hpp"
#include "table.hpp"
#include "poliz.hpp"
#include <iostream>
#include <vector>
#include <string>


class ParserException : public Exception {
    static const std::vector<std::string> message;
public:
    enum ParserExceptionCode {
        ESE_INCORRECT_LEX
    };
    ParserException(ParserExceptionCode code) : Exception(code) {}
    std::string GetMessage();
};


class Parser {
    Scanner scan;
    Poliz poliz;
    Poliz update_poliz;
    Lex curr_lex;
    lex_type curr_type;
    TableStruct * Struct;
    Table * table;
    Field * field;
    std::string t_name;
    std::string f_name;
    std::vector<long> long_in_list;
    std::vector<std::string> fields;
    std::vector<std::string> text_in_list;
    void gl();
    void SQL(std::vector<std::vector<std::string> > &);
    void SELECT();
    void UPDATE();
    void DELETE();
    void CREATE();
    void INSERT();
    void DROP();
    void WHERE();
    void NEWWHERE();
    void FIELDS_LIST();
    void FIELD_VALUE();  
    void FIELD_DESC_LIST();
    void FIELD_DESC();
    void FIELD_TYPE();
    void EXPRESSION();
    void CONST_LIST();
    void LONG_EXPRESSION();
    void TYT();
    void NEW_LONG_EXPRESSION();
    void LONG_SUM();
    void LONG_MULT();
    void TEXT_RELATION();
    void LONG_RELATION();

public:
    Parser(BaseSocket * s) : scan(s) {}
    void analyze(std::vector<std::vector<std::string> > &);
};

#endif
