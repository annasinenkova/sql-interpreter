#include "poliz.hpp"
#include <iostream>
#include <vector>
#include <regex>
#include <string>
#include <stack>


const std::vector<std::string> PolizException::message = {"INCORRECT_TYPE"};


std::string PolizException::GetMessage() {
    return "Poliz: " + message[this->err_code];
}


void Poliz::put_lex(Lex l) {
    p.push_back(l);
}


void Poliz::clear() {
    p.clear();
}


bool Poliz::calculate(Table * table, const std::vector<long> & long_in_list, const std::vector<std::string> & text_in_list) {
    Field * field;
    long num_second, num_first;
    std::string tnum_second, tnum_first;
    long bnum_first, bnum_second, res, num;
    st_type type, type_second, type_first;
    std::regex reg;
    std::cmatch result;
    for (size_t i = 0; i < p.size(); ++i) {
        switch(p[i].get_type()) {
            case LEX_ALL:
                res = true;
                st.push(st_elem(res));
                tp_st.push(BOOL);
                break;
            case LEX_LIKE:
                type_second = tp_st.top();
                tp_st.pop();
                type_first = tp_st.top();
                tp_st.pop();
                if (type_first != type_second || type_first != WORD) {
                    throw PolizException(PolizException::ESE_INCORRECT_TYPE);
                }
                tnum_second = st.top().t;
                st.pop();
                tnum_first = st.top().t;
                st.pop();
                tnum_second = std::regex_replace(tnum_second, std::regex("_"), ".");
                tnum_second = std::regex_replace(tnum_second, std::regex("%"), ".*");
                reg = tnum_second;
                res = false;
                if (std::regex_match(tnum_first.c_str(), result, reg)) {
                    res = true;
                }                
                st.push(st_elem(res));
                tp_st.push(BOOL);
                break;     
            case LEX_IN:
                type_first = tp_st.top();
                tp_st.pop();
                if (type_first == BOOL) {
                    throw PolizException(PolizException::ESE_INCORRECT_TYPE);
                }
                res = false;
                if (type_first == NUM) {
                    num_first = st.top().l;
                    for (size_t j = 0; j < long_in_list.size(); ++j) {
                        if (num_first == long_in_list[j]) {
                            res = true;
                        }
                    }
                }
                else {
                    tnum_first = st.top().t;
                    for (size_t j = 0; j < text_in_list.size(); ++j) {
                        if (tnum_first == text_in_list[j]) {
                            res = true;
                        }
                    }
                }
                st.push(st_elem(res));
                tp_st.push(BOOL);
                break;
            case LEX_NUM:
                st.push(st_elem(p[i].get_value()));
                tp_st.push(NUM);
                break;
            case LEX_STR:
                st.push(st_elem(p[i].get_text()));
                tp_st.push(WORD);
                break;
            case LEX_ID:
                field = table->GetField(p[i].get_text());
                if (field->GetType() == TEXT) {
                    st.push(st_elem(field->Text()));
                    tp_st.push(WORD);
                }
                else {
                    st.push(st_elem(field->Long()));
                    tp_st.push(NUM);
                }
                break;
            case LEX_PLUS:
                type_second = tp_st.top();
                tp_st.pop();
                type_first = tp_st.top();
                tp_st.pop();
                if (type_first != type_second || type_first != NUM) {
                    throw PolizException(PolizException::ESE_INCORRECT_TYPE);
                }
                num_second = st.top().l;
                st.pop();
                num_first =  st.top().l;
                st.pop();
                st.push(st_elem(num_first + num_second));
                tp_st.push(NUM);
                break;           
            case LEX_MINUS:
                type_second = tp_st.top();
                tp_st.pop();
                type_first = tp_st.top();
                tp_st.pop();
                if (type_first != type_second || type_first != NUM) {
                    throw PolizException(PolizException::ESE_INCORRECT_TYPE);
                }
                num_second =  st.top().l;
                st.pop();
                num_first =  st.top().l;
                st.pop();
                st.push(st_elem(num_first - num_second));
                tp_st.push(NUM);
                break;     
            case LEX_SLASH:
                type_second = tp_st.top();
                tp_st.pop();
                type_first = tp_st.top();
                tp_st.pop();
                if (type_first != type_second || type_first != NUM) {
                    throw PolizException(PolizException::ESE_INCORRECT_TYPE);
                }
                num_second =  st.top().l;
                st.pop();
                num_first =  st.top().l;
                st.pop();
                st.push(st_elem(num_first / num_second));
                tp_st.push(NUM);
                break;     
            case LEX_TIMES:
                type_second = tp_st.top();
                tp_st.pop();
                type_first = tp_st.top();
                tp_st.pop();
                if (type_first != type_second || type_first != NUM) {
                    throw PolizException(PolizException::ESE_INCORRECT_TYPE);
                }
                num_second =  st.top().l;
                st.pop();
                num_first =  st.top().l;
                st.pop();
                st.push(st_elem(num_first * num_second));
                tp_st.push(NUM);
                break; 
            case LEX_CENT:
                type_second = tp_st.top();
                tp_st.pop();
                type_first = tp_st.top();
                tp_st.pop();
                if (type_first != type_second || type_first != NUM) {
                    throw PolizException(PolizException::ESE_INCORRECT_TYPE);
                }
                num_second =  st.top().l;
                st.pop();
                num_first =  st.top().l;
                st.pop();
                st.push(st_elem(num_first % num_second));
                tp_st.push(NUM);
                break;
            case LEX_OR:
                type_second = tp_st.top();
                tp_st.pop();
                type_first = tp_st.top();
                tp_st.pop();
                if (type_first != type_second || type_first != BOOL) {
                    throw PolizException(PolizException::ESE_INCORRECT_TYPE);
                }
                bnum_second = st.top().l;
                st.pop();
                bnum_first = st.top().l;
                st.pop();
                st.push(st_elem(bnum_first or bnum_second));
                tp_st.push(BOOL);
                break;
            case LEX_ADDR:
                st.push(st_elem(p[i].get_text()));
                tp_st.push(WORD);
                break;      
            case LEX_OPEQ:
                type_second = tp_st.top();
                tp_st.pop();
                type_first = tp_st.top();
                tp_st.pop();
                if (type_second == BOOL) {
                    throw PolizException(PolizException::ESE_INCORRECT_TYPE);
                }
                if (type_second == NUM) {
                    num_second = st.top().l;
                    st.pop();
                    field = table->GetField(st.top().t);
                    if (field->GetType() != LONG) {
                        throw PolizException(PolizException::ESE_INCORRECT_TYPE);
                    }
                    st.pop();
                    field->Long() = num_second;
                    st.push(st_elem(true));
                }
                else {
                    tnum_second = st.top().t;
                    st.pop();
                    field = table->GetField(st.top().t);
                    if (field->GetType() != TEXT) {
                        throw PolizException(PolizException::ESE_INCORRECT_TYPE);
                    }
                    st.pop();
                    field->Text() = tnum_second;
                    st.push(st_elem(true));
                }
                tp_st.push(BOOL);
                break;
            case LEX_AND:
                type_second = tp_st.top();
                tp_st.pop();
                type_first = tp_st.top();
                tp_st.pop();
                if (type_first != type_second || type_first != BOOL) {
                    throw PolizException(PolizException::ESE_INCORRECT_TYPE);
                }
                bnum_second = st.top().l;
                st.pop();
                bnum_first = st.top().l;
                st.pop();
                st.push(st_elem(bnum_first and bnum_second));
                tp_st.push(BOOL);
                break;
            case LEX_NEQ:
                type_second = tp_st.top();
                tp_st.pop();
                type_first = tp_st.top();
                tp_st.pop();
                if (type_first != type_second || type_first == BOOL) {
                    throw PolizException(PolizException::ESE_INCORRECT_TYPE);
                }
                if (type_first == NUM) {
                    num_second = st.top().l;
                    st.pop();
                    num_first = st.top().l;
                    st.pop();
                    res = num_first != num_second;
                    st.push(st_elem(res));
                }
                else {
                    tnum_second = st.top().t;
                    st.pop();
                    tnum_first = st.top().t;
                    st.pop();
                    res = tnum_first != tnum_second;
                    st.push(st_elem(res));
                }
                tp_st.push(BOOL);
                break;
            case LEX_GEQ:
                type_second = tp_st.top();
                tp_st.pop();
                type_first = tp_st.top();
                tp_st.pop();
                if (type_first != type_second || type_first == BOOL) {
                    throw PolizException(PolizException::ESE_INCORRECT_TYPE);
                }
                if (type_first == NUM) {
                    num_second = st.top().l;
                    st.pop();
                    num_first = st.top().l;
                    st.pop();
                    res = num_first >= num_second;
                    st.push(st_elem(res));
                }
                else {
                    tnum_second = st.top().t;
                    st.pop();
                    tnum_first = st.top().t;
                    st.pop();
                    res = tnum_first >= tnum_second;
                    st.push(st_elem(res));
                }
                tp_st.push(BOOL);
                break;
            case LEX_LEQ:
                type_second = tp_st.top();
                tp_st.pop();
                type_first = tp_st.top();
                tp_st.pop();
                if (type_first != type_second || type_first == BOOL) {
                    throw PolizException(PolizException::ESE_INCORRECT_TYPE);
                }
                if (type_first == NUM) {
                    num_second = st.top().l;
                    st.pop();
                    num_first = st.top().l;
                    st.pop();
                    res = num_first <= num_second;
                    st.push(st_elem(res));
                }
                else {
                    tnum_second = st.top().t;
                    st.pop();
                    tnum_first = st.top().t;
                    st.pop();
                    res = tnum_first <= tnum_second;
                    st.push(st_elem(res));
                }
                tp_st.push(BOOL);
                break;
            case LEX_GTR:
                type_second = tp_st.top();
                tp_st.pop();
                type_first = tp_st.top();
                tp_st.pop();
                if (type_first != type_second || type_first == BOOL) {
                    throw PolizException(PolizException::ESE_INCORRECT_TYPE);
                }
                if (type_first == NUM) {
                    num_second = st.top().l;
                    st.pop();
                    num_first = st.top().l;
                    st.pop();
                    res = num_first > num_second;
                    st.push(st_elem(res));
                    tp_st.push(BOOL);
                }
                else {
                    tnum_second = st.top().t;
                    st.pop();
                    tnum_first = st.top().t;
                    st.pop();
                    res = tnum_first > tnum_second;
                    st.push(st_elem(res));
                    tp_st.push(BOOL);
                }
                break;
            case LEX_LSS:
                type_second = tp_st.top();
                tp_st.pop();
                type_first = tp_st.top();
                tp_st.pop();
                if (type_first != type_second || type_first == BOOL) {
                    throw PolizException(PolizException::ESE_INCORRECT_TYPE);
                }
                if (type_first == NUM) {
                    num_second = st.top().l;
                    st.pop();
                    num_first = st.top().l;
                    st.pop();
                    res = (num_first < num_second);
                    st.push(st_elem(res));
                }
                else {
                    tnum_second = st.top().t;
                    st.pop();
                    tnum_first = st.top().t;
                    st.pop();
                    res = tnum_first < tnum_second;
                    st.push(st_elem(res));
                }
                tp_st.push(BOOL);
                break;
            case LEX_EQ:
                type_second = tp_st.top();
                tp_st.pop();
                type_first = tp_st.top();
                tp_st.pop();
                if (type_first != type_second || type_first == BOOL) {
                    throw PolizException(PolizException::ESE_INCORRECT_TYPE);
                }
                if (type_first == NUM) {
                    num_second = st.top().l;
                    st.pop();
                    num_first = st.top().l;
                    st.pop();
                    res = (num_first == num_second);
                    st.push(st_elem(res));
                }
                else {
                    tnum_second = st.top().t;
                    st.pop();
                    tnum_first = st.top().t;
                    st.pop();
                    res = !strcmp(tnum_first.c_str(), tnum_second.c_str());
                    st.push(st_elem(res));
                }
                tp_st.push(BOOL);
                break;
            case LEX_NOT:
                type = tp_st.top();
                tp_st.pop();
                if (type != BOOL) {
                    throw PolizException(PolizException::ESE_INCORRECT_TYPE);
                }
                num = st.top().l;
                st.pop();
                st.push(st_elem(not num));
                tp_st.push(BOOL);
                break;
            default:
                throw PolizException(PolizException::ESE_INCORRECT_TYPE);
                break;
        }
    }
    type = tp_st.top();
    tp_st.pop();
    if (type != BOOL) {
        throw PolizException(PolizException::ESE_INCORRECT_TYPE);
    }
    num = st.top().l;
    st.pop();
    return num;
}

