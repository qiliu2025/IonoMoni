#include "read_Time_sp3.h"

void read_Time_sp3(std::ifstream& file, const std::string& filename, sp3& SP3) {
    file.close();
    file.open(filename);
    std::string line;
    if (file.is_open()) {
        while (std::getline(file, line)) {
            if (line[0] == '*') {
                SP3.sYear = stringToDouble(line.substr(3, 4));
                SP3.sMonth = stringToDouble(line.substr(8, 2));
                SP3.sDay = stringToDouble(line.substr(11, 2));
                SP3.sHour = stringToDouble(line.substr(14, 2));
                SP3.sMinute = stringToDouble(line.substr(17, 2));
                SP3.sSecond = stringToDouble(line.substr(20, 7));
                break;
            }
        }
    }
}
