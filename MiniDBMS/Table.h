#ifndef TABLE_H
#define TABLE_H

#include <string>
#include <vector>

class Table {
public:
    std::string name;
    std::vector<std::string> columns;
    std::vector<std::string> types;
    std::vector< std::vector<std::string> > rows;

    Table();
    Table(std::string tableName);

    void addColumn(std::string col, std::string type);
    bool insertRow(std::vector<std::string> values);

    int getColumnIndex(std::string col);

    void save(std::string file);
    void load(std::string file);
};

#endif
