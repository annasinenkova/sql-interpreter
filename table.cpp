#include "table.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <fcntl.h>
#include <unistd.h>


const std::vector<std::string> TableException::message = {"TBL_OPEN", "TBL_READ", "TBL_WRITE"};


std::string TableException::GetMessage() {
    return "Table: " + message[this->err_code];
}


Type Field::GetType() {
    return this->type;
}


std::string Field::GetName() {
    return this->name;
}


long & Field::Long() {
    return this->value;
}


std::string & Field::Text() {
    return this->text;
}


TableStruct * TableStruct::AddText(const std::string & name, size_t len) {
    this->fields.push_back(Field(TEXT, len, name));
    return this;
}


TableStruct * TableStruct::AddLong(const std::string & name) {
    this->fields.push_back(Field(LONG, 0, name));
    return this;
}


TableStruct * TableStruct::SetName(const std::string & name) {
    this->tbl_name = name;
    return this;
}


size_t TableStruct::LineOffset() {
    size_t size = 0;
    for (size_t i = 0; i < this->fields.size(); ++i) {
        if (this->fields[i].type == LONG) {
            size += sizeof(long);
        }
        else {
            size += this->fields[i].len * sizeof(this->fields[i].text[0]);
        }
    }
    return size;
}


TableStruct * Table::CreateTableStruct() {
    return new TableStruct;
}


void Table::Drop(const std::string & name) {
    unlink(name.c_str());
}


void Table::Drop() {
    unlink(tbl_str->tbl_name.c_str());
}


#define WRITE(fd, buf, size) \
    if (write((fd), (buf), (size)) != (int)(size)) { \
        throw TableException(TableException::ESE_TBLWRITE); \
    } \

#define READ(fd, buf, size) \
    if (read((fd), (buf), (size)) != (int)(size)) { \
        throw TableException(TableException::ESE_TBLREAD); \
    } \


Table * Table::Create(TableStruct * str) {
    Drop(str->tbl_name);
    str->fd = open(str->tbl_name.c_str(), O_CREAT | O_RDWR, 00777);
    size_t size = str->fields.size(), name_size;
    WRITE(str->fd, &size, sizeof(size));
    for (size_t i = 0; i < size; ++i) {
        name_size = str->fields[i].name.size() * sizeof(str->fields[i].name[0]);
        WRITE(str->fd, &name_size, sizeof(name_size));
        WRITE(str->fd, str->fields[i].name.c_str(), name_size); 
        WRITE(str->fd, &(str->fields[i].type), sizeof(str->fields[i].type));
        WRITE(str->fd, &(str->fields[i].len), sizeof(str->fields[i].len));
    }
    str->first_offset = lseek(str->fd, 0, SEEK_CUR);
    str->line_offset = str->LineOffset();
    close(str->fd);
    return new Table(str);
}


Table * Table::Open(const std::string & name) {
    TableStruct * str = new TableStruct;
    str->tbl_name = name;
    str->fd = open(name.c_str(), O_RDWR);
    size_t size = 0;
    size_t len, name_size;
    char field_name[20];
    Type type;
    READ(str->fd, &size, sizeof(size));
    for (size_t i = 0; i < size; ++i) {
        READ(str->fd, &name_size, sizeof(name_size));
        READ(str->fd, field_name, name_size);
        READ(str->fd, &type, sizeof(type));
        READ(str->fd, &len, sizeof(len));
        field_name[name_size] = '\0';
        str->fields.push_back(Field(type, len, field_name));
    }
    str->first_offset = lseek(str->fd, 0, SEEK_CUR);
    str->line_offset = str->LineOffset();
    return new Table(str);
}


#define READRECORD \
	for (size_t i = 0; i < tbl_str->fields.size(); ++i) { \
        if (tbl_str->fields[i].type == LONG) { \
            int n = read(tbl_str->fd, &(tbl_str->fields[i].value), sizeof(tbl_str->fields[i].value)); \
            if (n == 0) { \
                return false; \
            } \
            else if (n != sizeof(tbl_str->fields[i].value)) { \
                throw TableException(TableException::ESE_TBLREAD); \
            } \
        } \
        else { \
            char nsd[50]; \
            int size = tbl_str->fields[i].len * sizeof(tbl_str->fields[i].text[0]); \
            int n = read(tbl_str->fd, nsd, size); \
            nsd[n] = '\0'; \
            tbl_str->fields[i].text = nsd; \
            tbl_str->fields[i].text.resize(tbl_str->fields[i].len, '\0'); \
            if (n == 0) { \
                return false; \
            } \
            else if (n != size) { \
                throw TableException(TableException::ESE_TBLREAD); \
            } \
        } \
    } \
    lseek(tbl_str->fd, -tbl_str->line_offset, SEEK_CUR);


#define WRITERECORD \
	for (size_t i = 0; i < tbl_str->fields.size(); ++i) { \
        if (tbl_str->fields[i].type == LONG) { \
            WRITE(tbl_str->fd, &(tbl_str->fields[i].value), sizeof(tbl_str->fields[i].value)); \
        } \
        else { \
            tbl_str->fields[i].text.resize(tbl_str->fields[i].len, '\0'); \
            WRITE(tbl_str->fd, tbl_str->fields[i].text.c_str(), tbl_str->fields[i].len * sizeof(tbl_str->fields[i].text[0])); \
        } \
    } \
    lseek(tbl_str->fd, -tbl_str->line_offset, SEEK_CUR);


void Table::Delete() {
    for (size_t i = 0; i < tbl_str->fields.size(); ++i) {
        if (tbl_str->fields[i].type == LONG) {
            tbl_str->fields[i].value = 0;
        }
        else {
            tbl_str->fields[i].text = std::string(tbl_str->fields[i].len, '\0');
        }
    }
    WRITERECORD;
}


void Table::Add() {
    lseek(tbl_str->fd, 0, SEEK_END);
    WRITERECORD;
}


bool Table::ReadFirst() {
    lseek(tbl_str->fd, tbl_str->first_offset, SEEK_SET);
    READRECORD;
    return true;
}


bool Table::ReadNext() {
    lseek(tbl_str->fd, tbl_str->line_offset, SEEK_CUR);
    READRECORD;
    return true;
}


void Table::Update() {
    WRITERECORD;
}


Field * Table::GetField(const std::string & name) {
    for (size_t i = 0; i < tbl_str->fields.size(); ++i) {
        if (tbl_str->fields[i].name == name) {
            return &tbl_str->fields[i];
        }
    }
    return NULL;
}


Field * Table::GetFirstField() {
    return &tbl_str->fields[0];
}


Field * Table::GetLastField() {
    return &tbl_str->fields[tbl_str->fields.size() - 1];
}


Table::~Table() {
    delete tbl_str;
}

