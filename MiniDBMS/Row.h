#ifndef ROW_H
#define ROW_H

#include <vector>
#include <string>

class Row {
public:
    std::vector<std::string> values;

    Row() {}

    Row(std::vector<std::string> v) {
        values = v;
    }
};

#endif
