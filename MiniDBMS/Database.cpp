#include "Database.h"
#include "Utils.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

// helper: remove trailing semicolon if exists
static std::string removeSemicolon(const std::string& s) {
    if (!s.empty() && s.back() == ';') return s.substr(0, s.size() - 1);
    return s;
}

Database::Database() {
    currentDB = "";
}

// --------------------
// DATABASES
// --------------------
void Database::createDatabase(std::string dbName) {
    dbName = trim(dbName);
    dbName = removeSemicolon(dbName);

    std::string path = "data/" + dbName;
    if (!fs::exists(path)) {
        fs::create_directories(path);
        std::cout << "Database '" << dbName << "' created.\n";
    }
    else {
        std::cout << "Database already exists.\n";
    }
}

void Database::useDatabase(std::string dbName) {
    dbName = trim(dbName);
    dbName = removeSemicolon(dbName);

    std::string path = "data/" + dbName;
    if (!fs::exists(path)) {
        std::cout << "Database does not exist.\n";
        return;
    }

    currentDB = dbName;
    tables.clear();
    loadAllTables(path);
    std::cout << "Using database '" << currentDB << "'.\n";
}

std::string Database::getDBPath() {
    if (currentDB.empty()) return "";
    return "data/" + currentDB;
}

// --------------------
// TABLE CONTROL
// --------------------
void Database::createTable(std::string tableName) {
    tableName = trim(tableName);
    tableName = removeSemicolon(tableName);

    if (currentDB.empty()) {
        std::cout << "No database selected. Use: USE databaseName;\n";
        return;
    }

    Table t(tableName);
    tables[tableName] = t;

    std::string path = getDBPath() + "/" + tableName + ".json";
    tables[tableName].save(path);

    std::cout << "Table '" << tableName << "' created.\n";
}

Table* Database::getTable(std::string tableName) {
    tableName = trim(tableName);
    tableName = removeSemicolon(tableName);

    if (tables.count(tableName)) return &tables[tableName];
    return nullptr;
}

void Database::loadAllTables(const std::string& folderPath) {
    if (!fs::exists(folderPath)) return;

    for (const auto& file : fs::directory_iterator(folderPath)) {
        if (file.path().extension() == ".json") {
            std::string tableName = file.path().stem().string();
            tableName = removeSemicolon(tableName);

            Table t;
            t.load(file.path().string());

            tables[tableName] = t;
            std::cout << "Loaded table: " << tableName << "\n";
        }
    }
}

void Database::saveTable(const std::string& tableName, const std::string& folderPath) {
    std::string tname = trim(tableName);
    tname = removeSemicolon(tname);

    if (tables.count(tname)) {
        std::string path = folderPath + "/" + tname + ".json";
        tables[tname].save(path);
    }
}

// --------------------
// EXECUTE SQL (simple human-style parsing for SELECT WHERE)
// --------------------
void Database::executeSQL(std::string sql) {
    Parser parser;

    sql = trim(sql);
    sql = removeSemicolon(sql);

    std::string up = parser.upper(sql);

    // CREATE DATABASE
    if (up.rfind("CREATE DATABASE", 0) == 0) {
        std::string name = trim(sql.substr(16));
        name = removeSemicolon(name);
        createDatabase(name);
        return;
    }

    // USE
    if (up.rfind("USE ", 0) == 0) {
        std::string name = trim(sql.substr(4));
        name = removeSemicolon(name);
        useDatabase(name);
        return;
    }

    // CREATE TABLE
    if (up.rfind("CREATE TABLE", 0) == 0) {
        CreateTableCmd cmd = parser.parseCreate(sql);
        if (!cmd.ok) {
            std::cout << "Invalid CREATE TABLE syntax.\n";
            return;
        }

        std::string tbl = trim(cmd.tableName);
        createTable(tbl);

        Table* t = getTable(tbl);
        if (!t) {
            std::cout << "Failed to create/get table.\n";
            return;
        }

        // add columns & types
        for (int i = 0; i < cmd.columns.size(); i++) {
            t->addColumn(trim(cmd.columns[i]), trim(cmd.types[i]));
        }

        saveTable(tbl, getDBPath());
        return;
    }

    // must have a DB selected for the rest
    if (currentDB.empty()) {
        std::cout << "No database selected. Use: USE dbName;\n";
        return;
    }

    // INSERT
    if (up.rfind("INSERT INTO", 0) == 0) {
        InsertCmd cmd = parser.parseInsert(sql);
        if (!cmd.ok) {
            std::cout << "Invalid INSERT syntax.\n";
            return;
        }

        std::string tbl = trim(cmd.tableName);
        Table* t = getTable(tbl);
        if (!t) {
            std::cout << "Table does not exist.\n";
            return;
        }

        // check values count
        if (cmd.values.size() != t->columns.size()) {
            std::cout << "Error: number of values (" << cmd.values.size()
                << ") does not match number of columns (" << t->columns.size() << ")\n";
            return;
        }

        // primary key check (first column)
        for (auto& r : t->rows) {
            if (r.size() > 0 && r[0] == cmd.values[0]) {
                std::cout << "Error: duplicate primary key (" << cmd.values[0] << ")\n";
                return;
            }
        }

        if (!t->insertRow(cmd.values)) {
            std::cout << "Insert failed.\n";
            return;
        }

        saveTable(tbl, getDBPath());
        std::cout << "Row inserted.\n";
        return;
    }

    // SELECT (we parse WHERE locally: SELECT cols FROM tbl [WHERE col = value])
    if (up.rfind("SELECT", 0) == 0) {
        // find FROM and optional WHERE
        std::string upsql = up;
        int posFrom = upsql.find("FROM");
        if (posFrom == std::string::npos) {
            std::cout << "Invalid SELECT syntax (no FROM).\n";
            return;
        }

        std::string colsPart = trim(sql.substr(6, posFrom - 6));
        int posWhere = upsql.find("WHERE");

        std::string tablePart;
        std::string whereCol, whereVal;
        bool hasWhere = false;

        if (posWhere == std::string::npos) {
            tablePart = trim(sql.substr(posFrom + 4));
        }
        else {
            tablePart = trim(sql.substr(posFrom + 4, posWhere - (posFrom + 4)));
            std::string cond = trim(sql.substr(posWhere + 5));
            // cond format expected: column = "value"
            std::vector<std::string> p = split(cond, '=');
            if (p.size() == 2) {
                hasWhere = true;
                whereCol = trim(p[0]);
                whereVal = removeQuotes(trim(p[1]));
            }
        }

        std::string tbl = removeSemicolon(trim(tablePart));
        Table* t = getTable(tbl);
        if (!t) {
            std::cout << "Table does not exist.\n";
            return;
        }

        std::vector<std::string> wantCols;
        bool selectAll = false;
        if (colsPart == "*") {
            selectAll = true;
        }
        else {
            wantCols = split(colsPart, ',');
            for (auto& c : wantCols) c = trim(c);
        }

        int whereIdx = -1;
        if (hasWhere) {
            whereIdx = t->getColumnIndex(whereCol);
            if (whereIdx == -1) {
                std::cout << "Unknown column in WHERE.\n";
                return;
            }
        }

        // print
        for (int r = 0; r < t->rows.size(); r++) {
            if (hasWhere && t->rows[r][whereIdx] != whereVal) continue;

            if (selectAll) {
                for (int c = 0; c < t->rows[r].size(); c++)
                    std::cout << t->rows[r][c] << (c + 1 == t->rows[r].size() ? "" : " ");
                std::cout << "\n";
            }
            else {
                for (int i = 0; i < wantCols.size(); i++) {
                    int idx = t->getColumnIndex(wantCols[i]);
                    if (idx == -1) {
                        std::cout << "[?] ";
                    }
                    else {
                        std::cout << t->rows[r][idx] << (i + 1 == wantCols.size() ? "" : " ");
                    }
                }
                std::cout << "\n";
            }
        }
        return;
    }

    // DELETE
    if (up.rfind("DELETE FROM", 0) == 0) {
        DeleteCmd cmd = parser.parseDelete(sql);
        if (!cmd.ok) {
            std::cout << "Invalid DELETE syntax.\n";
            return;
        }

        std::string tbl = trim(cmd.tableName);
        Table* t = getTable(tbl);
        if (!t) {
            std::cout << "Table does not exist.\n";
            return;
        }

        if (!cmd.hasWhere) {
            t->rows.clear();
        }
        else {
            int colIdx = t->getColumnIndex(cmd.whereColumn);
            if (colIdx == -1) {
                std::cout << "Unknown column in WHERE.\n";
                return;
            }

            std::vector<std::vector<std::string>> newRows;
            for (auto& r : t->rows) {
                if (r[colIdx] != cmd.whereValue) newRows.push_back(r);
            }
            t->rows = newRows;
        }

        saveTable(tbl, getDBPath());
        std::cout << "Delete done.\n";
        return;
    }

    // UPDATE
    if (up.rfind("UPDATE", 0) == 0) {
        UpdateCmd cmd = parser.parseUpdate(sql);
        if (!cmd.ok) {
            std::cout << "Invalid UPDATE syntax.\n";
            return;
        }

        std::string tbl = trim(cmd.tableName);
        Table* t = getTable(tbl);
        if (!t) {
            std::cout << "Table does not exist.\n";
            return;
        }

        int setIdx = t->getColumnIndex(cmd.setColumn);
        if (setIdx == -1) {
            std::cout << "Error: unknown column in SET.\n";
            return;
        }

        // block updating primary key (column 0)
        if (setIdx == 0) {
            std::cout << "Error: cannot update PRIMARY KEY column.\n";
            return;
        }

        int whereIdx = -1;
        if (cmd.hasWhere) {
            whereIdx = t->getColumnIndex(cmd.whereColumn);
            if (whereIdx == -1) {
                std::cout << "Unknown column in WHERE.\n";
                return;
            }
        }

        // perform update
        for (auto& r : t->rows) {
            if (!cmd.hasWhere || r[whereIdx] == cmd.whereValue) {
                r[setIdx] = cmd.setValue;
            }
        }

        saveTable(tbl, getDBPath());
        std::cout << "Update done.\n";
        return;
    }

    std::cout << "Unknown SQL command.\n";
}
