#ifndef __TABLE_HPP__
#define __TABLE_HPP__

#include "exception.hpp"
#include <iostream>
#include <vector>
#include <string>


enum Type { TEXT, LONG };


class TableException : public Exception {
    static const std::vector<std::string> message;
public:
    enum TableExceptionCode {
        ESE_TBLOPEN,
        ESE_TBLREAD,
        ESE_TBLWRITE
    };
    TableException(TableExceptionCode code) : Exception(code) {}
    std::string GetMessage();
};
    
    
class Table;
class TableStruct;


class Field {
    friend class TableStruct;
    friend class Table;
public:
    Field(Type t, size_t l, const std::string & n) : type(t), len(l), name(n) {} 
    Type GetType();
    long & Long();
    std::string & Text();
    std::string GetName();
private:
    Type type;
    size_t len;
    long value;
    std::string name;
    std::string text;
};


class TableStruct {
    friend class Table;
public:
    TableStruct * AddText(const std::string &, size_t);
    TableStruct * AddLong(const std::string &);
    TableStruct * SetName(const std::string &);
private:
    int fd;
    size_t line_offset;
    size_t first_offset;
    std::string tbl_name;
    std::vector<Field> fields;
    size_t LineOffset();
};


class Table {  
public:
    static Table * Create(TableStruct *);
    static Table * Open(const std::string &); 
    static TableStruct * CreateTableStruct();
    static void Drop(const std::string &);
    Field * GetField(const std::string &);
    Field * GetFirstField();
    Field * GetLastField();
    void Add();
    void Delete();
    void Drop();
    bool ReadFirst();
    bool ReadNext();
    void Update();
    Table(TableStruct * str) : tbl_str(str) {}
    ~Table();
private:
    TableStruct * tbl_str;
};

   
#endif
