
#pragma once
#include <fstream>
#include <string>
#include "sp3.h"  
#include "stringToDouble.h"  

void getSp3Data(std::ifstream& file, const std::string& filename, sp3& SP3);
