#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>

struct CreateTableCmd {
    std::string tableName;
    std::vector<std::string> columns;
    std::vector<std::string> types;
    bool ok;
};

struct InsertCmd {
    std::string tableName;
    std::vector<std::string> values;
    bool ok;
};

struct SelectCmd {
    bool selectAll;
    std::string tableName;
    std::vector<std::string> columns;
    bool ok;
};

struct DeleteCmd {
    std::string tableName;
    bool hasWhere;
    std::string whereColumn;
    std::string whereValue;
    bool ok;
};

struct UpdateCmd {
    std::string tableName;
    std::string setColumn;
    std::string setValue;

    bool hasWhere;
    std::string whereColumn;
    std::string whereValue;

    bool ok;
};

class Parser {
public:
    std::string upper(std::string s);

    CreateTableCmd parseCreate(std::string sql);
    InsertCmd parseInsert(std::string sql);
    SelectCmd parseSelect(std::string sql);
    DeleteCmd parseDelete(std::string sql);
    UpdateCmd parseUpdate(std::string sql);
};

#endif
