#ifndef COLUMN_H
#define COLUMN_H

#include <string>

class Column {
public:
    std::string name;
    std::string type;

    Column() {
        name = "";
        type = "";
    }

    Column(std::string n, std::string t) {
        name = n;
        type = t;
    }
};

#endif
