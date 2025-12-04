#include <iostream>
#include "Database.h"

int main() {
    Database db;

    db.loadAllTables("data");

    std::cout << "MiniDBMS Started.\n";
    std::cout << "Enter SQL commands:\n\n";

    std::string sql;

    while (true) {
        std::cout << "DB> ";
        std::getline(std::cin, sql);

        if (sql == "exit") break;

        db.executeSQL(sql);
    }

    return 0;
}
