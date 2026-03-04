#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <functional>
#include <cmath>
#include <cstdio>

#include "obs.h"           
#include "sp3.h"           
#include "calc_elevation.h"
#include "mapping_function.h" 
#include "constants.h" 


// Remove short arcs (less than arc_min_length) in each column (satellite)
void remove_short_arcs_by_column(std::vector<std::vector<double>>& mat, int arc_min_length) {
    int row = mat.size();
    int col = mat[0].size();
    for (int j = 1; j < col; ++j) { // For each satellite (column)
        int start = -1;
        for (int i = 1; i < row; ++i) {
            if (mat[i][j] != 0 && start == -1) {
                start = i;
            }
            if ((mat[i][j] == 0 || i == row - 1) && start != -1) {
                int end = (mat[i][j] == 0) ? i - 1 : i;
                int seg_len = end - start + 1;
                if (seg_len < arc_min_length) {
                    for (int k = start; k <= end; ++k) {
                        mat[k][j] = 0.0;
                    }
                }
                start = -1;
            }
        }
    }
}

// For each satellite, set the first and last N points of each nonzero arc to zero
void remove_arc_edges_by_column(std::vector<std::vector<double>>& mat, int remove_n = 10) {
    int n_epoch = mat.size();
    int n_prn = mat[0].size();

    for (int prn = 1; prn < n_prn; ++prn) {
        int i = 1;
        while (i < n_epoch) {

            while (i < n_epoch && std::fabs(mat[i][prn]) < 1e-8)
                ++i;
            int arc_start = i;

            while (i < n_epoch && std::fabs(mat[i][prn]) >= 1e-8)
                ++i;
            int arc_end = i - 1;

            if (arc_end >= arc_start) {
                // Remove first remove_n points
                for (int k = 0; k < remove_n; ++k) {
                    int idx = arc_start + k;
                    if (idx <= arc_end)
                        mat[idx][prn] = 0.0;
                }
                // Remove last remove_n points
                for (int k = 0; k < remove_n; ++k) {
                    int idx = arc_end - k;
                    if (idx >= arc_start)
                        mat[idx][prn] = 0.0;
                }
            }
        }
    }
}

void output_ppp_vtec_txt(
    const std::string& filepath,
    const std::string& prefix,
    int max_prn,
    const std::vector<std::vector<double>>& PPP_STEC,
    const obs& OBS,
    const sp3* SP3,
    int mf_type,
    std::function<void(const sp3*, int prn, int epoch, double& x, double& y, double& z)> get_sp3_coord_func
) {
    std::ofstream fout(filepath);
    if (!fout.is_open()) {
        std::cerr << "Failed to open output file: " << filepath << std::endl;
        return;
    }

    fout << std::setw(12) << "Epoch \\ PRN";
    for (int i = 1; i <= max_prn; ++i) {
        char prn_buf[10];
        sprintf(prn_buf, "%s%02d", prefix.c_str(), i);
        fout << std::setw(11) << prn_buf;
    }
    fout << "\n";

    for (int j = 1; j <= 2880; ++j) {
        char epoch_buf[20];
        sprintf(epoch_buf, "Epoch %04d:", j);
        fout << std::setw(12) << epoch_buf;

        for (int i = 1; i <= max_prn; ++i) {
            double stec = PPP_STEC[j][i];
            if (fabs(stec) < 1e-8) stec = 0.0;

            double x, y, z;
            get_sp3_coord_func(SP3, i, j, x, y, z);
            x *= 1000; y *= 1000; z *= 1000;

            double sx = OBS.X, sy = OBS.Y, sz = OBS.Z;
            double E_deg, A_deg;
            calc_elevation(x, y, z, sx, sy, sz, E_deg, A_deg);
            double elev_rad = E_deg * PI / 180.0;
            double mf = get_mapping_function(elev_rad, mf_type);

            double vtec = (mf > 0.01) ? (stec / mf) : 0.0;
            if (fabs(vtec) < 1e-8) vtec = 0.0;

            fout << std::setw(11) << std::fixed << std::setprecision(5) << vtec;
        }
        fout << "\n";
    }
    fout.close();
}