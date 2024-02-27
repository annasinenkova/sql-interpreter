#include "scanner.hpp"
#include <iostream>
#include <vector>
#include <string>


const std::vector<std::string> ScannerException::message = {"INCORRECT_CHAR"};


std::string ScannerException::GetMessage() {
    return "Scanner: " + message[this->err_code];
}


std::vector<std::pair<std::string, lex_type> > Scanner::TW = {
    {"", LEX_NULL},
    {"SELECT", LEX_SELECT},
    {"INSERT", LEX_INSERT},
    {"UPDATE", LEX_UPDATE},
    {"DELETE", LEX_DELETE},
    {"CREATE", LEX_CREATE},
    {"DROP", LEX_DROP},
    {"FROM", LEX_FROM},
    {"INTO", LEX_INTO},
    {"SET", LEX_SET},
    {"TABLE", LEX_TABLE},
    {"TEXT", LEX_TEXT},
    {"LONG", LEX_LONG},
    {"WHERE", LEX_WHERE},
    {"OR", LEX_OR},
    {"AND", LEX_AND},
    {"NOT", LEX_NOT},
    {"ALL", LEX_ALL},
    {"LIKE", LEX_LIKE},
    {"IN", LEX_IN}
};


std::vector<std::pair<std::string, lex_type> > Scanner::TD = {
    {"", LEX_NULL},
    {"(", LEX_LPAREN},
    {")", LEX_RPAREN},
    {"*", LEX_TIMES},
    {",", LEX_COMMA},
    {"-", LEX_MINUS},
    {"+", LEX_PLUS},
    {"/", LEX_SLASH},
    {"!=", LEX_NEQ},
    {">=", LEX_GEQ},
    {"<=", LEX_LEQ},
    {">", LEX_GTR},
    {"<", LEX_LSS},
    {"=", LEX_EQ},
    {"%", LEX_CENT},
    {"'", LEX_APOSTR}
};


lex_type Lex::get_type() { 
    return t_lex; 
}


long Lex::get_value() {
    return v_lex;
}


std::string Lex::get_text() {
    return w_lex;
}


Scanner::Scanner(BaseSocket * s) {
    sock = s;
    CS = H;
    gc();
}


void Scanner::gc() {
    c = sock->GetChar();
}


size_t Scanner::look(const std::string & s, std::vector<std::pair<std::string, lex_type> > s_buf) {
    size_t i = 0;
    for (i = 0; i < s_buf.size(); ++i) {
        if (s == s_buf[i].first) {
            return i;
        }
    }
    return 0;
}


Lex Scanner::get_lex() {
    std::string buf;
    size_t pos_in_t = 0;
    size_t num = 0;
    CS = H;
    do {
        switch (CS) {
            case H:
                if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
                    gc();
                }
                else if (c == '\'') {
                    gc();
                    CS = STR;
                }
                else if (c == ';') {
                    return Lex(LEX_END, 0);
                }
                else if (isalpha(c)) {
                    buf.push_back(c);
                    gc();
                    CS = IDENT;
                }
                else if (c == '0') {
                    gc();
                    return Lex(LEX_NUM, 0);
                }
                else if (isdigit(c)) {
                    num = c - '0';
                    gc();
                    CS = NUMB;
                }
                else if (c == '<' || c == '>') {
                    buf.push_back(c);
                    gc();
                    CS = ALE;
                }
                else if (c == '!') {
                    buf.push_back(c);
                    gc();
                    CS = NEQ;
                }
                else {
                    CS = DELIM;
                }
                break;
            case STR:
                if (c == '\'') {
                    gc();
                    return Lex(LEX_STR, buf);
                }
                buf.push_back(c);
                gc();
                break;
            case IDENT:
                if (isalpha(c) || isdigit(c) || c == '_') {
                    buf.push_back(c);
                    gc();
                }
                else {
                    pos_in_t = look(buf, TW);
                    if (pos_in_t) {
                        return Lex(TW[pos_in_t].second, pos_in_t);
                    }
                    return Lex(LEX_ID, buf);
                }
                break;
            case NUMB:
                if (isdigit(c)) {
                    num = num * 10 + (c - '0');
                    gc();
                }
                else {
                    return Lex(LEX_NUM, num);
                }
                break;
            case ALE:
                if (c == '=') {
                    buf.push_back(c);
                    gc();
                }
                pos_in_t = look(buf, TD);
                return Lex(TD[pos_in_t].second, pos_in_t);
                break;
            case NEQ:
                if (c == '=') {
                    buf.push_back(c);
                    gc();
                    pos_in_t = look(buf, TD);
                    return Lex(TD[pos_in_t].second, pos_in_t);
                }
                else {
                    throw ScannerException(ScannerException::ESE_INCORRECT_CHAR);
                }
                break;
            case DELIM:
                buf.push_back(c);
                pos_in_t = look(buf, TD);
                if (pos_in_t) {
                    gc();
                    return Lex(TD[pos_in_t].second, pos_in_t);
                }
                else {
                    throw ScannerException(ScannerException::ESE_INCORRECT_CHAR);
                }
                break;
            default:
                throw ScannerException(ScannerException::ESE_INCORRECT_CHAR);
        }
    } while(true);
}

