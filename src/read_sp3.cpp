#include "read_sp3.h"

void getSp3Data(std::ifstream& file, const std::string& filename, sp3& SP3) {
    double ep = 1, h, m;
    std::string line;

    const int GPS_SAT_MAX = 32;
    const int BDS_SAT_MAX = 46;
    const int GAL_SAT_MAX = 36;
    const int GLO_SAT_MAX = 24;

    file.close();
    file.open(filename);

    if (file.is_open()) {
        while (std::getline(file, line)) {

            if (line[0] == '*') {
                h = stringToDouble(line.substr(14, 2));
                m = stringToDouble(line.substr(17, 2));
                ep = (int)h * 12 + (int)(m) / 5 + 1;
            }
            else if (line[1] == 'G') {
                int prn = (int)stringToDouble(line.substr(2, 2));
                if (prn < 1 || prn > GPS_SAT_MAX) continue;

                if (SP3.X[prn][(int)ep * 10] == 0) {
                    // The == 0 check ensures only the first occurrence of each satellite / epoch data is written.
                    // Some SP3 files may include the 0:00 epoch data from the next day.
                    SP3.X[prn][(int)ep * 10] = stringToDouble(line.substr(4, 14));
                    SP3.Y[prn][(int)ep * 10] = stringToDouble(line.substr(18, 14));
                    SP3.Z[prn][(int)ep * 10] = stringToDouble(line.substr(32, 14));
                }
            }
            else if (line[1] == 'C') {
                int prn = (int)stringToDouble(line.substr(2, 2));
                if (prn < 1 || prn > BDS_SAT_MAX) continue;

                if (SP3.CX[prn][(int)ep * 10] == 0) {
                    SP3.CX[prn][(int)ep * 10] = stringToDouble(line.substr(4, 14));
                    SP3.CY[prn][(int)ep * 10] = stringToDouble(line.substr(18, 14));
                    SP3.CZ[prn][(int)ep * 10] = stringToDouble(line.substr(32, 14));
                }
            }
            else if (line[1] == 'E') {
                int prn = (int)stringToDouble(line.substr(2, 2));
                if (prn < 1 || prn > GAL_SAT_MAX) continue;

                if (SP3.EX[prn][(int)ep * 10] == 0) {
                    SP3.EX[prn][(int)ep * 10] = stringToDouble(line.substr(4, 14));
                    SP3.EY[prn][(int)ep * 10] = stringToDouble(line.substr(18, 14));
                    SP3.EZ[prn][(int)ep * 10] = stringToDouble(line.substr(32, 14));
                }
            }
            else if (line[1] == 'R') {
                int prn = (int)stringToDouble(line.substr(2, 2));
                if (prn < 1 || prn > GLO_SAT_MAX) continue;

                if (SP3.RX[prn][(int)ep * 10] == 0) {
                    SP3.RX[prn][(int)ep * 10] = stringToDouble(line.substr(4, 14));
                    SP3.RY[prn][(int)ep * 10] = stringToDouble(line.substr(18, 14));
                    SP3.RZ[prn][(int)ep * 10] = stringToDouble(line.substr(32, 14));
                }
            }
        }
    }
}