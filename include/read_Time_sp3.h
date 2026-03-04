
#pragma once
#include <fstream>
#include <string>
#include "sp3.h" 
#include "stringToDouble.h"  

void read_Time_sp3(std::ifstream& file, const std::string& filename, sp3& SP3);
