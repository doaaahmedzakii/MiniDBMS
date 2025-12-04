#include "Utils.h"
#include <cctype>

std::string trim(std::string s) {
    int i = 0, j = s.size() - 1;

    while (i <= j && isspace(s[i])) i++;
    while (j >= i && isspace(s[j])) j--;

    return s.substr(i, j - i + 1);
}

std::string removeQuotes(std::string s) {
    s = trim(s);
    if (s.size() >= 2 && s[0] == '"' && s[s.size() - 1] == '"')
        return s.substr(1, s.size() - 2);
    return s;
}

std::vector<std::string> split(std::string s, char delimiter) {
    std::vector<std::string> res;
    std::string cur = "";

    for (int i = 0; i < s.size(); i++) {
        if (s[i] == delimiter) {
            if (!trim(cur).empty())
                res.push_back(trim(cur));
            cur = "";
        }
        else {
            cur += s[i];
        }
    }

    if (!trim(cur).empty())
        res.push_back(trim(cur));

    return res;
}
