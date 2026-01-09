#include "calc_ipp.h"
#include "ecef2elli.h"
#include "constants.h"
#include "calc_elevation.h"
#include <cmath>
#include <fstream>
#include <iomanip>
#include <string>
#include <iostream>

void calc_ipp_matrix(
    double station_X, double station_Y, double station_Z,
    const double X[35][3288],
    const double Y[35][3288],
    const double Z[35][3288],
    int maxSat,
    int maxEpoch,
    const std::string& systemName,
    const std::string& txt_output_path
)
{
    double sb, sl;
    ecef2elli(station_X, station_Y, station_Z, sb, sl);  // sb = lat, sl = lon

    std::vector<std::vector<IPP_Point>> IPP(maxSat + 1, std::vector<IPP_Point>(maxEpoch + 1));

    for (int j = 1; j <= maxEpoch; ++j) {
        for (int i = 1; i <= maxSat; ++i) {
            double sv_x = X[i][j] * 1000;
            double sv_y = Y[i][j] * 1000;
            double sv_z = Z[i][j] * 1000;

            if (sv_x == 0 && sv_y == 0 && sv_z == 0) {
                IPP[i][j].lat = NAN;
                IPP[i][j].lon = NAN;
                continue;
            }

            // Satellite direction vector ds = sv - st
            double dx = sv_x - station_X;
            double dy = sv_y - station_Y;
            double dz = sv_z - station_Z;
            double ss = std::sqrt(dx * dx + dy * dy + dz * dz);

            // ENU conversion matrix Bt
            double Bt[3][3] = {
                {-std::sin(sl), std::cos(sl), 0},
                {-std::cos(sl) * std::sin(sb), -std::sin(sl) * std::sin(sb), std::cos(sb)},
                { std::cos(sl) * std::cos(sb),  std::sin(sl) * std::cos(sb), std::sin(sb)}
            };

            double enu[3] = {
                Bt[0][0] * dx + Bt[0][1] * dy + Bt[0][2] * dz,
                Bt[1][0] * dx + Bt[1][1] * dy + Bt[1][2] * dz,
                Bt[2][0] * dx + Bt[2][1] * dy + Bt[2][2] * dz
            };

            double az = std::atan2(enu[0], enu[1]);
            if (az < 0) az += 2 * PI;
            double el = std::asin(enu[2] / ss);

            // Calculate zenith angle (central angle)
            double zen = std::asin(a * std::sin(PI / 2.0 - el) / (a + h));

            // Spherical distance pip (central angle from station to pierce point)
            double pip = PI / 2.0 - el - zen;

            // Pierce point latitude plat
            double plat = std::asin(std::sin(sb) * std::cos(pip) + std::cos(sb) * std::sin(pip) * std::cos(az));

            // Pierce point longitude plon
            double plon = sl + std::asin(std::sin(pip) * std::sin(az) / std::cos(plat));

            IPP[i][j].lat = plat;
            IPP[i][j].lon = plon;
        }
    }

    std::string prefix_path = txt_output_path.substr(0, txt_output_path.find_last_of("."));
    std::string lat_filename = prefix_path + "_IPP_lat_" + systemName + ".txt";
    std::string lon_filename = prefix_path + "_IPP_lon_" + systemName + ".txt";

    std::ofstream lat_out(lat_filename);
    std::ofstream lon_out(lon_filename);

    if (!lat_out.is_open() || !lon_out.is_open()) {
        std::cerr << "Cannot open output file!" << std::endl;
        return;
    }

    for (int i = 1; i <= maxSat; ++i) {
        char prn_buf[10];
        sprintf(prn_buf, "PRN%02d", i);
        lat_out << std::setw(12) << prn_buf;
        lon_out << std::setw(12) << prn_buf;
    }
    lat_out << "\n";
    lon_out << "\n";

    for (int j = 1; j <= maxEpoch; ++j) {
        for (int i = 1; i <= maxSat; ++i) {
            double lat_deg = IPP[i][j].lat * RAD_TO_DEG;
            double lon_deg = IPP[i][j].lon * RAD_TO_DEG;
            lat_out << std::setw(12) << std::fixed << std::setprecision(6) << lat_deg;
            lon_out << std::setw(12) << std::fixed << std::setprecision(6) << lon_deg;
        }
        lat_out << "\n";
        lon_out << "\n";
    }

    lat_out.close();
    lon_out.close();
}
