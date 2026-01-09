#include "read_sp3.h"

void getSp3Data(std::ifstream& file, const std::string& filename, sp3& SP3) {
    double ep=1, h, m;
    std::string line;
    file.close();
    file.open(filename);
    if (file.is_open()) {
        while (std::getline(file, line)) {
       
            if (line[0] == '*') {
                h = stringToDouble(line.substr(14, 2));
                m = stringToDouble(line.substr(17, 2));
                ep = (int)h * 12 + (int)(m) / 5 + 1;
             
            }
            else if (line[1] == 'G' && SP3.X[(int)stringToDouble(line.substr(2, 2))][(int)ep * 10] == 0) {
                //The == 0 check ensures only the first occurrence of each satellite / epoch data is written.Some SP3 files may include the 0:00 epoch data from the next day.
                SP3.X[(int)stringToDouble(line.substr(2, 2))][(int)ep * 10] = stringToDouble(line.substr(4, 14));
                SP3.Y[(int)stringToDouble(line.substr(2, 2))][(int)ep * 10] = stringToDouble(line.substr(18, 14));
                SP3.Z[(int)stringToDouble(line.substr(2, 2))][(int)ep * 10] = stringToDouble(line.substr(32, 14));
            }
            else if (line[1] == 'C' && SP3.CX[(int)stringToDouble(line.substr(2, 2))][(int)ep * 10] == 0) {
                SP3.CX[(int)stringToDouble(line.substr(2, 2))][(int)ep * 10] = stringToDouble(line.substr(4, 14));
                SP3.CY[(int)stringToDouble(line.substr(2, 2))][(int)ep * 10] = stringToDouble(line.substr(18, 14));
                SP3.CZ[(int)stringToDouble(line.substr(2, 2))][(int)ep * 10] = stringToDouble(line.substr(32, 14));
            }
            else if (line[1] == 'E' && SP3.EX[(int)stringToDouble(line.substr(2, 2))][(int)ep * 10] == 0) {
                SP3.EX[(int)stringToDouble(line.substr(2, 2))][(int)ep * 10] = stringToDouble(line.substr(4, 14));
                SP3.EY[(int)stringToDouble(line.substr(2, 2))][(int)ep * 10] = stringToDouble(line.substr(18, 14));
                SP3.EZ[(int)stringToDouble(line.substr(2, 2))][(int)ep * 10] = stringToDouble(line.substr(32, 14));
            }
            else if (line[1] == 'R' && SP3.RX[(int)stringToDouble(line.substr(2, 2))][(int)ep * 10] == 0) {
                SP3.RX[(int)stringToDouble(line.substr(2, 2))][(int)ep * 10] = stringToDouble(line.substr(4, 14));
                SP3.RY[(int)stringToDouble(line.substr(2, 2))][(int)ep * 10] = stringToDouble(line.substr(18, 14));
                SP3.RZ[(int)stringToDouble(line.substr(2, 2))][(int)ep * 10] = stringToDouble(line.substr(32, 14));
            }
        }
    }
}
