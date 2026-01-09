#include"stringToDouble.h"
#include <sstream> 
double stringToDouble(const std::string& str) {
    std::istringstream iss(str);
    double result = 0.0;
    iss >> std::skipws >> result;
    return result;
}