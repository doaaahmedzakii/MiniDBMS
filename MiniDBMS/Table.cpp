#include "Table.h"
#include "Utils.h"
#include <fstream>
#include <iostream>

Table::Table() {
    name = "";
}

Table::Table(std::string tableName) {
    name = tableName;
}

void Table::addColumn(std::string col, std::string type) {
    columns.push_back(col);
    types.push_back(type);
}

int Table::getColumnIndex(std::string col) {
    for (int i = 0; i < columns.size(); i++) {
        if (columns[i] == col)
            return i;
    }
    return -1;
}

bool Table::insertRow(std::vector<std::string> values) {
    if (values.size() != columns.size())
        return false;
    rows.push_back(values);
    return true;
}

// ------------------------------------
// JSON SAVE (manual, simple)
// ------------------------------------
void Table::save(std::string file) {
    std::ofstream out(file);

    out << "{\n";
    out << "  \"name\": \"" << name << "\",\n";

    // columns
    out << "  \"columns\": [";
    for (int i = 0; i < columns.size(); i++) {
        out << "\"" << columns[i] << "\"";
        if (i != columns.size() - 1) out << ", ";
    }
    out << "],\n";

    // types
    out << "  \"types\": [";
    for (int i = 0; i < types.size(); i++) {
        out << "\"" << types[i] << "\"";
        if (i != types.size() - 1) out << ", ";
    }
    out << "],\n";

    // rows
    out << "  \"rows\": [\n";

    for (int i = 0; i < rows.size(); i++) {
        out << "    [";

        for (int j = 0; j < rows[i].size(); j++) {
            out << "\"" << rows[i][j] << "\"";
            if (j != rows[i].size() - 1) out << ", ";
        }

        out << "]";
        if (i != rows.size() - 1) out << ",";
        out << "\n";
    }

    out << "  ]\n";
    out << "}\n";

    out.close();
}

// ------------------------------------
// JSON LOAD (manual parsing)
// ------------------------------------
void Table::load(std::string file) {
    std::ifstream in(file);
    if (!in.is_open()) return;

    columns.clear();
    types.clear();
    rows.clear();

    std::string line;

    bool inColumns = false;
    bool inTypes = false;
    bool inRows = false;

    while (std::getline(in, line)) {

        line = trim(line);

        // name
        if (line.find("\"name\"") != std::string::npos) {
            int p1 = line.find(":") + 1;
            int p2 = line.rfind("\"");
            std::string nm = line.substr(p1, p2 - p1);
            name = removeQuotes(trim(nm));
        }

        // start columns
        if (line.find("\"columns\"") != std::string::npos) {
            inColumns = true;
            continue;
        }

        if (inColumns) {
            if (line.find("]") != std::string::npos) {
                inColumns = false;
                continue;
            }

            std::vector<std::string> c = split(line, ',');
            for (auto x : c) {
                x = removeQuotes(x);
                if (x != "") columns.push_back(x);
            }
            continue;
        }

        // start types
        if (line.find("\"types\"") != std::string::npos) {
            inTypes = true;
            continue;
        }

        if (inTypes) {
            if (line.find("]") != std::string::npos) {
                inTypes = false;
                continue;
            }

            std::vector<std::string> t = split(line, ',');
            for (auto x : t) {
                x = removeQuotes(x);
                if (x != "") types.push_back(x);
            }
            continue;
        }

        // start rows
        if (line.find("\"rows\"") != std::string::npos) {
            inRows = true;
            continue;
        }

        if (inRows) {
            if (line == "]" || line == "],") {
                inRows = false;
                continue;
            }

            if (line.find("[") == 0) {
                std::string inside = line.substr(1, line.find("]") - 1);
                std::vector<std::string> vals = split(inside, ',');
                for (int i = 0; i < vals.size(); i++)
                    vals[i] = removeQuotes(vals[i]);
                rows.push_back(vals);
            }
        }
    }

    in.close();
}
