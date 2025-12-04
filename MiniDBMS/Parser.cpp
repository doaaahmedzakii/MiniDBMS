#include "Parser.h"
#include "Utils.h"

std::string Parser::upper(std::string s) {
    for (int i = 0; i < s.size(); i++)
        if (s[i] >= 'a' && s[i] <= 'z')
            s[i] -= 32;
    return s;
}

// ---------------------------
// CREATE TABLE
// ---------------------------
CreateTableCmd Parser::parseCreate(std::string sql) {
    CreateTableCmd cmd;
    cmd.ok = false;

    std::string up = upper(sql);

    if (up.find("CREATE TABLE") != 0)
        return cmd;

    // ??????? ??? ??????
    int pos1 = up.find("TABLE") + 5;
    int pos2 = sql.find("(");

    cmd.tableName = trim(sql.substr(pos1, pos2 - pos1));

    // ??????? ???????
    int pos3 = sql.find("(");
    int pos4 = sql.rfind(")");

    std::string inside = sql.substr(pos3 + 1, pos4 - pos3 - 1);

    std::vector<std::string> parts = split(inside, ',');

    for (int i = 0; i < parts.size(); i++) {
        std::vector<std::string> p = split(parts[i], ' ');
        if (p.size() == 2) {
            cmd.columns.push_back(p[0]);
            cmd.types.push_back(p[1]);
        }
    }

    cmd.ok = true;
    return cmd;
}

// ---------------------------
// INSERT
// ---------------------------
InsertCmd Parser::parseInsert(std::string sql) {
    InsertCmd cmd;
    cmd.ok = false;

    std::string up = upper(sql);

    if (up.find("INSERT INTO") != 0)
        return cmd;

    int posInto = up.find("INTO") + 4;
    int posValues = up.find("VALUES");

    cmd.tableName = trim(sql.substr(posInto, posValues - posInto));

    int pos1 = sql.find("(");
    int pos2 = sql.find(")");

    std::string inside = sql.substr(pos1 + 1, pos2 - pos1 - 1);

    cmd.values = split(inside, ',');

    for (int i = 0; i < cmd.values.size(); i++)
        cmd.values[i] = removeQuotes(cmd.values[i]);

    cmd.ok = true;
    return cmd;
}

// ---------------------------
// SELECT
// ---------------------------
SelectCmd Parser::parseSelect(std::string sql) {
    SelectCmd cmd;
    cmd.ok = false;
    cmd.selectAll = false;

    std::string up = upper(sql);

    if (up.find("SELECT") != 0)
        return cmd;

    int posFrom = up.find("FROM");

    std::string cols = trim(sql.substr(6, posFrom - 6));

    if (cols == "*")
        cmd.selectAll = true;
    else
        cmd.columns = split(cols, ',');

    cmd.tableName = trim(sql.substr(posFrom + 4));

    cmd.ok = true;
    return cmd;
}

// ---------------------------
// DELETE
// ---------------------------
DeleteCmd Parser::parseDelete(std::string sql) {
    DeleteCmd cmd;
    cmd.ok = false;
    cmd.hasWhere = false;

    std::string up = upper(sql);

    if (up.find("DELETE FROM") != 0)
        return cmd;

    int posFrom = up.find("FROM") + 4;
    int posWhere = up.find("WHERE");

    if (posWhere == -1) {
        cmd.tableName = trim(sql.substr(posFrom));
    }
    else {
        cmd.tableName = trim(sql.substr(posFrom, posWhere - posFrom));

        std::string cond = sql.substr(posWhere + 5);
        std::vector<std::string> p = split(cond, '=');

        if (p.size() == 2) {
            cmd.hasWhere = true;
            cmd.whereColumn = trim(p[0]);
            cmd.whereValue = removeQuotes(trim(p[1]));
        }
    }

    cmd.ok = true;
    return cmd;
}

// ---------------------------
// UPDATE
// ---------------------------
UpdateCmd Parser::parseUpdate(std::string sql) {
    UpdateCmd cmd;
    cmd.ok = false;
    cmd.hasWhere = false;

    std::string up = upper(sql);

    if (up.find("UPDATE") != 0)
        return cmd;

    int posSet = up.find("SET");
    int posWhere = up.find("WHERE");

    cmd.tableName = trim(sql.substr(6, posSet - 6));

    std::string setPart;

    if (posWhere == -1)
        setPart = trim(sql.substr(posSet + 3));
    else
        setPart = trim(sql.substr(posSet + 3, posWhere - (posSet + 3)));

    std::vector<std::string> p = split(setPart, '=');

    cmd.setColumn = trim(p[0]);
    cmd.setValue = removeQuotes(trim(p[1]));

    if (posWhere != -1) {
        cmd.hasWhere = true;

        std::string cond = sql.substr(posWhere + 5);
        std::vector<std::string> pp = split(cond, '=');

        cmd.whereColumn = trim(pp[0]);
        cmd.whereValue = removeQuotes(trim(pp[1]));
    }

    cmd.ok = true;
    return cmd;
}
