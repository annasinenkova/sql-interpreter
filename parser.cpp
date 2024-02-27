#include "parser.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>


const std::vector<std::string> ParserException::message = {"INCORRECT_LEX"};


std::string ParserException::GetMessage() {
    return "Parser: " + message[this->err_code];
}


void Parser::analyze(std::vector<std::vector<std::string> > & t) {
    gl();
    SQL(t);
}


void Parser::gl() {
    curr_lex = scan.get_lex();
    curr_type = curr_lex.get_type();
}


std::string toString(long val) {
    std::ostringstream oss;
    oss << val;
    return oss.str();
}

#define CHECKLEX(type) \
    if (curr_type != type) { \
        throw ParserException(ParserException::ESE_INCORRECT_LEX); \
    } \
    gl();


void Parser::SQL(std::vector<std::vector<std::string> > & t) { // SQL -> "SELECT" SELECT | "INSERT" INSERT | "UPDATE" UPDATE | "DELETE" DELETE | "CREATE" CREATE | "DROP" DROP
    lex_type tp = curr_type;
    gl();
    std::vector<std::string> tt;
    table = NULL;
    try {
        switch (tp) {
            case LEX_SELECT:
                SELECT();
                CHECKLEX(LEX_END);
                for (size_t i = 0; i < fields.size(); ++i) {
                    field = table->GetField(fields[i]);
                    tt.push_back(field->GetName());
                }
                t.push_back(tt);
                table->ReadFirst();
                do {
                    if (poliz.calculate(table, long_in_list, text_in_list)) {
                        tt.resize(0);
                        for (size_t i = 0; i < fields.size(); ++i) {
                            field = table->GetField(fields[i]);
                            if (field->GetType() == TEXT) {
                                tt.push_back(field->Text());
                            }
                            else {
                                tt.push_back(toString(field->Long()));
                            }
                        }
                        t.push_back(tt);
                    }
                } while (table->ReadNext());
                delete table;
                break;
            case LEX_UPDATE:
                UPDATE();
                CHECKLEX(LEX_END);
                table->ReadFirst();
                do {
                    if (poliz.calculate(table, long_in_list, text_in_list)) {
                        update_poliz.calculate(table, long_in_list, text_in_list);
                        table->Update();
                        }
                } while (table->ReadNext());
                delete table;
                break;
            case LEX_DELETE:
                DELETE();
                CHECKLEX(LEX_END);
                table->ReadFirst();
                do {
                    if (poliz.calculate(table, long_in_list, text_in_list)) {
                        table->Delete();
                    }
                } while (table->ReadNext());
                delete table;
                break;
            case LEX_DROP:
                DROP();
                CHECKLEX(LEX_END);
                Table::Drop(t_name);
                break;
            case LEX_INSERT:
                INSERT();
                CHECKLEX(LEX_END);
                table->Add();
                delete table;
                break; 
            case LEX_CREATE:
                CREATE();
                CHECKLEX(LEX_END);
                table = Table::Create(Struct);
                delete table;
                break;
            default:
                throw ParserException(ParserException::ESE_INCORRECT_LEX);
        }
    }
    catch (Exception & e) {
        if (!table) {
            delete table;
        }
        throw;
    }
}   


void Parser::CREATE() { // CREATE -> "TABLE" "C_ID" <tbl_name = c_id> "(" FIELD_DESC_LIST ")" <create>
    CHECKLEX(LEX_TABLE);
    if (curr_type != LEX_ID) {
        throw ParserException(ParserException::ESE_INCORRECT_LEX);
    }
    t_name = curr_lex.get_text();
    Struct = Table::CreateTableStruct();
    Struct->SetName(t_name);
    gl();
    CHECKLEX(LEX_LPAREN);
    FIELD_DESC_LIST();
    CHECKLEX(LEX_RPAREN);
}


void Parser::FIELD_DESC_LIST() { // FIELD_DESC_LIST -> FIELD_DESC { "," FIELD_DESC }
    FIELD_DESC();
    while (curr_type == LEX_COMMA) {
        gl();
        FIELD_DESC();
    }
}


void Parser::FIELD_DESC() { // FIELD_DESC -> "C_ID" <fld_name = c_id> FIELD_TYPE
    if (curr_type != LEX_ID) {
        throw ParserException(ParserException::ESE_INCORRECT_LEX);
    }
    f_name = curr_lex.get_text();
    gl();
    FIELD_TYPE();
}


void Parser::FIELD_TYPE() { // FIELD_TYPE -> "TEXT" "(" "NUM" ")" <add_text> | "LONG" <add_long>
    if (curr_type == LEX_TEXT) {
        gl();
        CHECKLEX(LEX_LPAREN);
        if (curr_type != LEX_NUM) {
            throw ParserException(ParserException::ESE_INCORRECT_LEX);
        }
        Struct->AddText(f_name, curr_lex.get_value());
        gl();
        CHECKLEX(LEX_RPAREN);
    }
    else {
        CHECKLEX(LEX_LONG);
        Struct->AddLong(f_name);
    }
}


void Parser::INSERT() { // INSERT -> "INTO" "C_ID" <tbl_name = c_id, open> "(" FIELD_VALUE { "," FIELD_VALUE } ")"
    CHECKLEX(LEX_INTO);
    if (curr_type != LEX_ID) {
        throw ParserException(ParserException::ESE_INCORRECT_LEX);
    }
    t_name = curr_lex.get_text();
    table = Table::Open(t_name);
    field = table->GetFirstField();
    gl();
    CHECKLEX(LEX_LPAREN);
    FIELD_VALUE();
    while (curr_type == LEX_COMMA) {
        gl();
        FIELD_VALUE();
    }
    CHECKLEX(LEX_RPAREN);
}


void Parser::FIELD_VALUE() { // FIELD_VALUE -> "NUM" | "STR"
    if (curr_type == LEX_NUM) {
        if (field->GetType() != LONG) {
            throw ParserException(ParserException::ESE_INCORRECT_LEX);
        }
        field->Long() = curr_lex.get_value();
        ++field;
        gl();
    }
    else if (curr_type == LEX_STR) {
        if (field->GetType() != TEXT) {
            throw ParserException(ParserException::ESE_INCORRECT_LEX);
        }
        field->Text() = curr_lex.get_text();
        ++field;
        gl();
    }
    else {
        throw ParserException(ParserException::ESE_INCORRECT_LEX);
    }
}


void Parser::WHERE() { // WHERE -> "WHERE" NEWWHERE
    CHECKLEX(LEX_WHERE);
    NEWWHERE();
}


void Parser::NEWWHERE() { // NEWWHERE -> "ALL" <poliz.push(all)> | EXPRESSION TYT
    if (curr_type == LEX_ALL) {
        poliz.put_lex(curr_lex);
        gl();
    }
    else {
        EXPRESSION();
        TYT();
    }
}


void Parser::TYT() { // <TYT> -> [ "NOT" ] "LIKE" "STR" <!text> <poliz.push(str)> <poliz.push(like)> | [ "NOT" ] "IN" "(" CONST_LIST ")" <!text, !long> <poliz.push(in)> | eps <!bool>
    bool lex_not = false;
    if (curr_type == LEX_NOT) {
        lex_not = true;
        gl();
    }
    if (curr_type == LEX_LIKE) {
        Lex l = curr_lex;
        gl();
        if (curr_type != LEX_STR) {
            throw ParserException(ParserException::ESE_INCORRECT_LEX);
        }
        poliz.put_lex(curr_lex);
        gl();
        poliz.put_lex(l);
    }
    else if (curr_type == LEX_IN) {
        Lex l = curr_lex;
        gl();
        CHECKLEX(LEX_LPAREN);
        CONST_LIST();
        CHECKLEX(LEX_RPAREN);
        poliz.put_lex(l);
    }
    if (lex_not) {
        poliz.put_lex(Lex(LEX_NOT, 0));
    }
}


void Parser::CONST_LIST() { // CONST_LIST -> "STR" <poliz.push(str)> { "," "STR" <poliz.push(str)> } | "NUM" <poliz.push(long)> { "," "NUM <poliz.push(long)> }
    if (curr_type == LEX_STR) {
        text_in_list.push_back(curr_lex.get_text());
        gl();
        while (curr_type == LEX_COMMA) {
            gl();
            if (curr_type == LEX_STR) {
                text_in_list.push_back(curr_lex.get_text());
            }
            else {
                throw ParserException(ParserException::ESE_INCORRECT_LEX);
            }
            gl();
        }
    }
    else if (curr_type == LEX_NUM) {
        long_in_list.push_back(curr_lex.get_value());
        gl();
        while (curr_type == LEX_COMMA) {
            gl();
            if (curr_type == LEX_NUM) {
                long_in_list.push_back(curr_lex.get_value());
            }
            else {
                throw ParserException(ParserException::ESE_INCORRECT_LEX);
            }
            gl();
        }
    }
    else {
        throw ParserException(ParserException::ESE_INCORRECT_LEX);
    }

}


void Parser::EXPRESSION() { // <EXPRESSION> -> "STR" <poliz.push(str)> | "C_ID" <!text, poliz.push(text)> | LONG_EXPRESSION
    if (curr_type == LEX_STR) {
        poliz.put_lex(curr_lex);
        gl();
    }
    else if (curr_type == LEX_ID) {
        Field * fiel = table->GetField(curr_lex.get_text());
        if (fiel->GetType() == TEXT) {
            poliz.put_lex(curr_lex);
            gl();
        }
        else {
            LONG_EXPRESSION();
        }
    }
    else {
        LONG_EXPRESSION();
    }
}


void Parser::LONG_EXPRESSION() { // LONG_EXPRESSION -> LONG_SUM { ["+"|"-"|"OR"] LONG_SUM <poliz.push(op)> }
    LONG_SUM();
    while (curr_type == LEX_PLUS || curr_type == LEX_MINUS || curr_type == LEX_OR) {
        Lex l = curr_lex;
        gl();
        LONG_SUM();
        poliz.put_lex(l);
    }
}


void Parser::LONG_SUM() { // LONG_SUM -> LONG_MULT { ["*"|"/"|"%"|"AND"] LONG_MULT <poliz.push(op)> }
    LONG_MULT();
    while (curr_type == LEX_TIMES || curr_type == LEX_SLASH || curr_type == LEX_CENT || curr_type == LEX_AND) {
        Lex l = curr_lex;
        gl();
        LONG_MULT();
        poliz.put_lex(l);
    }
}


void Parser::LONG_MULT() { // LONG_MULT -> "(" NEW_LONG_EXPRESSION ")" |  "NUM" <poliz.push(long)> | "C_ID" <!long, poliz.push(long)> | "NOT" LONG_MULT <poliz.push(not)>
    if (curr_type == LEX_LPAREN) {
        gl();
        NEW_LONG_EXPRESSION();
        CHECKLEX(LEX_RPAREN);
    }
    else if (curr_type == LEX_NUM) {
        poliz.put_lex(curr_lex);
        gl();
    }
    else if (curr_type == LEX_ID) {
        Field * fiel = table->GetField(curr_lex.get_text());
        if (fiel->GetType() == LONG) {
            poliz.put_lex(curr_lex);
            gl();
        }
        else {
            throw ParserException(ParserException::ESE_INCORRECT_LEX);
        }
    }
    else if (curr_type == LEX_NOT) {
        Lex l = curr_lex;
        gl();
        LONG_MULT();
        poliz.put_lex(l);
    }
    else {
        throw ParserException(ParserException::ESE_INCORRECT_LEX);
    }
}


void Parser::NEW_LONG_EXPRESSION() { // NEW_LONG_EXPRESSION -> LONG_EXPRESSION LONG_RELATION | "STR" <poliz.push(str)> TEXT_RELATION  | "C_ID" <!text, poliz.push(text)> TEXT_RELATION
    if (curr_type == LEX_STR) {
        poliz.put_lex(curr_lex);
        gl();
        TEXT_RELATION();
    }
    else if (curr_type == LEX_ID) {
        Field * fiel = table->GetField(curr_lex.get_text());
        if (fiel->GetType() == TEXT) {
            poliz.put_lex(curr_lex);
            gl();
            TEXT_RELATION();
        }
        else {
            LONG_EXPRESSION();
            LONG_RELATION();
        }
    }
    else {
        LONG_EXPRESSION();
        LONG_RELATION();
    }
}


void Parser::TEXT_RELATION() { // TEXT_RELATION -> [ "==" | ">" | "<" | ">=" | "<=" | "!=" ] TEXT_EXPRESSION <poliz.push(op)>
    if (curr_type != LEX_NEQ && curr_type != LEX_GEQ && curr_type != LEX_LEQ && curr_type != LEX_GTR && curr_type != LEX_LSS && curr_type != LEX_EQ) {
        throw ParserException(ParserException::ESE_INCORRECT_LEX);
    }
    Lex l = curr_lex;
    gl();
    if (curr_type == LEX_STR) {
        poliz.put_lex(curr_lex);
        gl();
    }
    else if (curr_type == LEX_ID) {
        Field * fiel = table->GetField(curr_lex.get_text());
        if (fiel->GetType() == TEXT) {
            poliz.put_lex(curr_lex);
            gl();
        }
        else {
            throw ParserException(ParserException::ESE_INCORRECT_LEX);
        }
    }
    else {
        throw ParserException(ParserException::ESE_INCORRECT_LEX);
    }
    poliz.put_lex(l);
}


void Parser::LONG_RELATION() { // LONG_RELATION ->  [ "==" | ">" | "<" | ">=" | "<=" | "!=" ] LONG_EXPRESSION <poliz.push(op)> | eps
    if (curr_type == LEX_NEQ || curr_type == LEX_GEQ || curr_type == LEX_LEQ || curr_type == LEX_GTR || curr_type == LEX_LSS || curr_type == LEX_EQ) {
        Lex l = curr_lex;
        gl();
        LONG_EXPRESSION();
        poliz.put_lex(l);
    }
}


void Parser::SELECT() { // SELECT -> FIELDS_LIST "FROM" "C_ID" <tbl_name = c_id, open> WHERE
    FIELDS_LIST();
    if (curr_type == LEX_TIMES) {
        gl();
        CHECKLEX(LEX_FROM);
        if (curr_type == LEX_ID) {
            t_name = curr_lex.get_text();
            table = Table::Open(t_name);
            gl();
        }
        else {
            throw ParserException(ParserException::ESE_INCORRECT_LEX);
        }
        field = table->GetFirstField();
        while (field != table->GetLastField()) {
            fields.push_back(field->GetName());
            ++field;
        }
        fields.push_back(field->GetName());
    }
    else {
        CHECKLEX(LEX_FROM);
        if (curr_type == LEX_ID) {
            t_name = curr_lex.get_text();
            table = Table::Open(t_name);
            gl();
        }
        else {
            throw ParserException(ParserException::ESE_INCORRECT_LEX);
        }  
    }
    WHERE();
}


void Parser::FIELDS_LIST() { // FIELDS_LIST ->  "*" <list.push(all)> | "C_ID" <list.push(field)> { "," "C_ID" <list.push(field)> }
    if (curr_type == LEX_ID) {
        fields.push_back(curr_lex.get_text());
        gl();
        while (curr_type == LEX_COMMA) {
            gl();
            fields.push_back(curr_lex.get_text());
            gl();
        }
    }
    else if (curr_type != LEX_TIMES) {
        throw ParserException(ParserException::ESE_INCORRECT_LEX);
    }
}


void Parser::DELETE() { // DELETE -> "FROM" "C_ID" <tbl_name = c_id, open> WHERE
    CHECKLEX(LEX_FROM);
    if (curr_type == LEX_ID) {
        t_name = curr_lex.get_text();
        table = Table::Open(t_name);
        gl();
    }
    else {
        throw ParserException(ParserException::ESE_INCORRECT_LEX);
    }
    WHERE();
}


void Parser::DROP() { // DROP -> "TABLE" "C_ID" <tbl_name = c_id>
    CHECKLEX(LEX_TABLE);
    if (curr_type != LEX_ID) {
        throw ParserException(ParserException::ESE_INCORRECT_LEX);
    }
    t_name = curr_lex.get_text();
    gl();
}


void Parser::UPDATE() { // UPDATE -> "C_ID" <tbl_name = c_id, open> "SET" "C_ID" <fld_name = c_id, poliz.push(&)> "=" EXPRESSION <poliz.push(=)> WHERE
    if (curr_type != LEX_ID) {
        throw ParserException(ParserException::ESE_INCORRECT_LEX);
    }
    t_name = curr_lex.get_text();
    table = Table::Open(t_name);
    gl();
    CHECKLEX(LEX_SET);
    if (curr_type != LEX_ID) {
        throw ParserException(ParserException::ESE_INCORRECT_LEX);
    }
    poliz.put_lex(Lex(LEX_ADDR, curr_lex.get_text()));
    gl();
    if (curr_type != LEX_EQ) {
        throw ParserException(ParserException::ESE_INCORRECT_LEX);
    }
    Lex l = curr_lex;
    gl();
    EXPRESSION();
    poliz.put_lex(Lex(LEX_OPEQ, l.get_value()));
    update_poliz = poliz;
    poliz.clear();
    WHERE();
}

