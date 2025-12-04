#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

std::string trim(std::string s);
std::string removeQuotes(std::string s);
std::vector<std::string> split(std::string s, char delimiter);

#endif
