#include "calc_AATR.h"
#include <iostream>
#include <vector>
#include <cstring>
#include "read_DCB.h"
#include "extract_arcs.h"
#include "calc_elevation.h"
#include "detect_cycle_slip.h"
#include "constants.h"
#include <fstream>   
#include <iomanip>  
#include <filesystem> 
#include <cmath>     
#include "gcfg_ppp.h"

void calc_aatr_GPS(
    obs& OBS,
    const std::string& stationName,
    sp3 SP3[],
    const std::string& txt_output_path,
    t_gcfg_ppp& gset,
    gnut::t_gsetgnss& set_gnss,                
    std::shared_ptr<spdlog::logger> logger      
) {

    auto gps_band2freq = [&](gnut::GOBSBAND b)->double {
        switch (b) {
        case gnut::BAND_1: return GPS_F1;
        case gnut::BAND_2: return GPS_F2;
        case gnut::BAND_5: return GPS_F5;
        default:
            throw std::logic_error("Unsupported GPS band: " + gnut::gobsband2str(b));
        }
        };

    const auto bands = set_gnss.band(gnut::GPS);
    const gnut::GOBSBAND bandA = bands.at(0);
    const gnut::GOBSBAND bandB = bands.at(1);
    const double f1 = gps_band2freq(bandA);
    const double f2 = gps_band2freq(bandB);

    logger->info("[{}] GPS AATR uses XML bands = {} {} => f1={} Hz, f2={} Hz",
        stationName, gnut::gobsband2str(bandA), gnut::gobsband2str(bandB), f1, f2);

    const double lambda_wl = c / (f1 - f2);
    double MW[33][2881], GF[33][2881], wlAmb[33][2881];
    double AATR_LGF[35][3000];
    double AATR_ROT[33][2888];
    double cs_epoch[33][2881];

    int arc_min_len = gset.arc_min_length();
    int interval_size = gset.aatr_interval();

    memset(AATR_ROT, 0, sizeof(AATR_ROT));
    memset(cs_epoch, 0, sizeof(cs_epoch));

    for (int i = 1; i <= 32; i++) {
        for (int j = 1; j <= 2880; j++) {
            MW[i][j] = lambda_wl * (OBS.L1[i][j] - OBS.L2[i][j])
                - (f1 * OBS.C1[i][j] + f2 * OBS.C2[i][j]) / (f1 + f2);
            GF[i][j] = OBS.L1[i][j] - f1 * OBS.L2[i][j] / f2;
            wlAmb[i][j] = MW[i][j] * -1.0;
        }
    }

    for (int i = 1; i <= 32; i++) {
        int len = 2881;
        int num_arcs = 0;
        std::vector<std::vector<int>> arcs;
        extract_arcs(MW[i], len, arcs, num_arcs);

        for (int k = 0; k < num_arcs; ++k) {
            if (arcs[k][1] - arcs[k][0] < arc_min_len) {
                for (int e = arcs[k][0]; e <= arcs[k][1]; ++e) {
                    OBS.C1[i][e] = 0;
                    OBS.C2[i][e] = 0;
                    OBS.L1[i][e] = 0;
                    OBS.L2[i][e] = 0;
                }
                arcs.erase(arcs.begin() + k);
                --k;
                --num_arcs;
            }
        }

        int ARCS[33][3000][2];
        ARCS[i][0][0] = num_arcs;

        for (int k = 0; k < num_arcs; ++k) {
            ARCS[i][k + 1][0] = arcs[k][0];
            ARCS[i][k + 1][1] = arcs[k][1];
        }

        detect_cycle_slip(wlAmb[i], GF[i], OBS, i, ARCS, arc_min_len, cs_epoch);

        // Calculate AATR_ROT
        for (int j = 1; j <= 2880; j++) {
            AATR_LGF[i][j] = (c / f1) * OBS.L1[i][j] - (c / f2) * OBS.L2[i][j];
        }

        std::string rot_unit = gset.rot_unit();
        std::transform(rot_unit.begin(), rot_unit.end(), rot_unit.begin(),
            [](unsigned char ch) { return (char)std::tolower(ch); });

        double rotNumber;
        if (rot_unit == "sec") {
            rotNumber = 30.0 * 1e16 * IONO_COEFF * (1.0 / (f1 * f1) - 1.0 / (f2 * f2));
        }
        else {
            rotNumber = 0.5 * 1e16 * IONO_COEFF * (1.0 / (f1 * f1) - 1.0 / (f2 * f2));
        }

        for (int j = 2; j <= 2880; j++) {
            if (AATR_LGF[i][j] != 0 && AATR_LGF[i][j - 1] != 0 && cs_epoch[i][j] != 1) {
                AATR_ROT[i][j] = (AATR_LGF[i][j] - AATR_LGF[i][j - 1]) / rotNumber;
            }
        }
    }

    int nBlocks = (2880 + interval_size - 1) / interval_size;
    std::vector<double> AATR_block(nBlocks, 0.0);
    int block_index = 0;

    for (int j = 1; j <= 2880; j += interval_size) {
        double sum_block = 0;
        int count = 0;

        for (int k = j; k < j + interval_size; k++) {
            for (int i = 1; i <= 32; i++) {
                if (AATR_ROT[i][k] != 0) {
                    double E, A;
                    calc_elevation(SP3[1].X[i][k] * 1000, SP3[1].Y[i][k] * 1000, SP3[1].Z[i][k] * 1000,
                        OBS.X, OBS.Y, OBS.Z, E, A);

                    double E_rad = E * PI / 180.0;
                    double cos_E = cos(E_rad);
                    double tmp = (Re * cos_E) / (Re + h);
                    double M = 1.0 / sqrt(1.0 - tmp * tmp);

                    double AATR = (1.0 / (M * M)) * AATR_ROT[i][k];

                    sum_block += AATR * AATR;
                    count++;
                }
            }
        }

        if (count > 0) AATR_block[block_index] = sqrt(sum_block / count);
        else           AATR_block[block_index] = 0;

        block_index++;
    }

    std::string aatr_out_path = txt_output_path.substr(0, txt_output_path.find_last_of(".")) + "_GPS_AATR.txt";
    std::filesystem::create_directories(std::filesystem::path(aatr_out_path).parent_path());

    std::ofstream out(aatr_out_path);
    if (!out.is_open()) {
        logger->error("Failed to open GPS AATR output file: {}", aatr_out_path);
        return;
    }

    out << std::fixed << std::setprecision(5);
    out << std::setw(6) << "Block" << std::setw(15) << "AATR" << "\n";

    for (int k = 0; k < block_index; ++k) {
        char block_buf[20];
        sprintf(block_buf, "Block %02d:", k + 1);
        double val = AATR_block[k];
        out << std::setw(10) << block_buf << std::setw(12) << val << "\n";
    }

    out.close();
    logger->info("[{}] GPS AATR saved: {}", stationName, aatr_out_path);
}

void calc_aatr_BDS(
    obs& OBS,
    const std::string& stationName,
    sp3 SP3[],
    const std::string& txt_output_path,
    t_gcfg_ppp& gset,
    gnut::t_gsetgnss& set_gnss,                 
    std::shared_ptr<spdlog::logger> logger      
) {
    auto bds_band2freq = [&](gnut::GOBSBAND b)->double {
        switch (b) {
        case gnut::BAND_2: return BDS_F2;
        case gnut::BAND_6: return BDS_F6;
        case gnut::BAND_7: return BDS_F7;
        default:
            throw std::logic_error("Unsupported BDS band: " + gnut::gobsband2str(b));
        }
        };

    const auto bands = set_gnss.band(gnut::BDS);  

    if (bands.size() != 2) {
        logger->error("[{}] Invalid <bds><band>: expect exactly 2 items, but got {}",
            stationName, bands.size());
        throw std::runtime_error("Invalid <bds><band>: must provide exactly 2 items.");
    }

    const gnut::GOBSBAND bandA = bands.at(0);
    const gnut::GOBSBAND bandB = bands.at(1);

    auto is_ok_band = [](gnut::GOBSBAND b)->bool {
        return (b == gnut::BAND_2 || b == gnut::BAND_6 || b == gnut::BAND_7);
        };

    if (!is_ok_band(bandA) || !is_ok_band(bandB)) {
        logger->error("[{}] Invalid <bds><band>: only 2/6/7 allowed, but got {} {}",
            stationName, gnut::gobsband2str(bandA), gnut::gobsband2str(bandB));
        throw std::runtime_error("Invalid <bds><band>: only 2/6/7 are allowed.");
    }


    const double f1 = bds_band2freq(bandA);
    const double f2 = bds_band2freq(bandB);

    logger->info("[{}] BDS AATR uses XML bands = {} {} => f1={} Hz, f2={} Hz",
        stationName, gnut::gobsband2str(bandA), gnut::gobsband2str(bandB), f1, f2);

    if (f1 == f2) {
        throw std::runtime_error("BDS band invalid: f1 == f2");
    }

    const double lambda_wl = c / (f1 - f2);
    int arc_min_len = gset.arc_min_length();
    int interval_size = gset.aatr_interval();

    double MW[47][2881], GF[47][2881], wlAmb[47][2881];
    double AATR_LGF[49][3000];
    double AATR_ROT[47][2888];
    double cs_epoch[47][2881];

    memset(AATR_ROT, 0, sizeof(AATR_ROT));
    memset(cs_epoch, 0, sizeof(cs_epoch));

    for (int i = 1; i <= 46; i++) {
        for (int j = 1; j <= 2880; j++) {
            MW[i][j] = lambda_wl * (OBS.L1[i][j] - OBS.L2[i][j])
                - (f1 * OBS.C1[i][j] + f2 * OBS.C2[i][j]) / (f1 + f2);
            GF[i][j] = OBS.L1[i][j] - f1 * OBS.L2[i][j] / f2;
            wlAmb[i][j] = MW[i][j] * -1.0;
        }
    }

    for (int i = 1; i <= 46; i++) {
        int len = 2881;
        int num_arcs = 0;
        std::vector<std::vector<int>> arcs;

        extract_arcs(MW[i], len, arcs, num_arcs);

        for (int k = 0; k < num_arcs; ++k) {
            if (arcs[k][1] - arcs[k][0] < arc_min_len) {
                for (int e = arcs[k][0]; e <= arcs[k][1]; ++e) {
                    OBS.C1[i][e] = 0;
                    OBS.C2[i][e] = 0;
                    OBS.L1[i][e] = 0;
                    OBS.L2[i][e] = 0;
                }
                arcs.erase(arcs.begin() + k);
                --k;
                --num_arcs;
            }
        }

        int ARCS[47][3000][2];
        ARCS[i][0][0] = num_arcs;
        for (int k = 0; k < num_arcs; ++k) {
            ARCS[i][k + 1][0] = arcs[k][0];
            ARCS[i][k + 1][1] = arcs[k][1];
        }

        detect_cycle_slip(wlAmb[i], GF[i], OBS, i, ARCS, arc_min_len, cs_epoch);

        for (int j = 1; j <= 2880; j++) {
            AATR_LGF[i][j] = (c / f1) * OBS.L1[i][j] - (c / f2) * OBS.L2[i][j];
        }

        std::string rot_unit = gset.rot_unit();
        std::transform(rot_unit.begin(), rot_unit.end(), rot_unit.begin(),
            [](unsigned char ch) { return (char)std::tolower(ch); });

        double rotNumber;
        if (rot_unit == "sec") {
            rotNumber = 30.0 * 1e16 * IONO_COEFF * (1.0 / (f1 * f1) - 1.0 / (f2 * f2));
        }
        else {
            rotNumber = 0.5 * 1e16 * IONO_COEFF * (1.0 / (f1 * f1) - 1.0 / (f2 * f2));
        }

        for (int j = 2; j <= 2880; j++) {
            if (AATR_LGF[i][j] != 0 && AATR_LGF[i][j - 1] != 0 && cs_epoch[i][j] != 1) {
                AATR_ROT[i][j] = (AATR_LGF[i][j] - AATR_LGF[i][j - 1]) / rotNumber;
            }
        }
    }

    int nBlocks = (2880 + interval_size - 1) / interval_size;
    std::vector<double> AATR_block(nBlocks, 0.0);
    int block_index = 0;

    for (int j = 1; j <= 2880; j += interval_size) {
        double sum_block = 0;
        int count = 0;

        for (int k = j; k < j + interval_size; k++) {
            for (int i = 1; i <= 46; i++) {
                if (AATR_ROT[i][k] != 0) {
                    double E, A;
                    calc_elevation(SP3[1].CX[i][k] * 1000, SP3[1].CY[i][k] * 1000, SP3[1].CZ[i][k] * 1000,
                        OBS.X, OBS.Y, OBS.Z, E, A);

                    double E_rad = E * PI / 180.0;
                    double cos_E = cos(E_rad);
                    double tmp = (Re * cos_E) / (Re + h);
                    double M = 1.0 / sqrt(1.0 - tmp * tmp);

                    double AATR = (1.0 / (M * M)) * AATR_ROT[i][k];

                    sum_block += AATR * AATR;
                    count++;
                }
            }
        }

        if (count > 0) AATR_block[block_index] = sqrt(sum_block / count);
        else           AATR_block[block_index] = 0;

        block_index++;
    }

    std::string aatr_out_path = txt_output_path.substr(0, txt_output_path.find_last_of(".")) + "_BDS_AATR.txt";
    std::filesystem::create_directories(std::filesystem::path(aatr_out_path).parent_path());

    std::ofstream out(aatr_out_path);
    if (!out.is_open()) {
        logger->error("Failed to open BDS AATR output file: {}", aatr_out_path);
        return;
    }

    out << std::fixed << std::setprecision(5);
    out << std::setw(6) << "Block" << std::setw(15) << "AATR" << "\n";

    for (int k = 0; k < block_index; ++k) {
        char block_buf[20];
        sprintf(block_buf, "Block %02d:", k + 1);
        out << std::setw(10) << block_buf << std::setw(12) << AATR_block[k] << "\n";
    }

    out.close();
    logger->info("[{}] BDS AATR saved: {}", stationName, aatr_out_path);
}


void calc_aatr_GAL(
    obs& OBS,
    const std::string& stationName,
    sp3 SP3[],
    const std::string& txt_output_path,
    t_gcfg_ppp& gset,
    gnut::t_gsetgnss& set_gnss,
    std::shared_ptr<spdlog::logger> logger
) {

    auto gal_band2freq = [&](gnut::GOBSBAND b)->double {
        switch (b) {
        case gnut::BAND_1: return GAL_F1;
        case gnut::BAND_5: return GAL_F5;
        case gnut::BAND_7: return GAL_F7;
        case gnut::BAND_8: return GAL_F8;
        case gnut::BAND_6: return GAL_F6;
        default:
            throw std::logic_error("Unsupported GAL band: " + gnut::gobsband2str(b));
        }
        };
    const auto bands = set_gnss.band(gnut::GAL);
    const gnut::GOBSBAND bandA = bands.at(0);  
    const gnut::GOBSBAND bandB = bands.at(1);   
    const double f1 = gal_band2freq(bandA);
    const double f2 = gal_band2freq(bandB);

    logger->info("[{}] GAL AATR uses XML bands = {} {} => f1={} Hz, f2={} Hz", stationName, gnut::gobsband2str(bandA), gnut::gobsband2str(bandB), f1, f2);
    

    const double lambda_wl = c / (f1 - f2); 

    double MW[37][2881], GF[37][2881], wlAmb[37][2881];
    double AATR_LGF[39][3000];
    double AATR_ROT[37][2888];
    double cs_epoch[37][2881];

    int arc_min_len = gset.arc_min_length();
    int interval_size = gset.aatr_interval();

    memset(AATR_ROT, 0, sizeof(AATR_ROT));
    memset(cs_epoch, 0, sizeof(cs_epoch));

    for (int i = 1; i <= 36; i++) {
        for (int j = 1; j <= 2880; j++) {
            MW[i][j] = lambda_wl * (OBS.L1[i][j] - OBS.L2[i][j]) - (f1 * OBS.C1[i][j] + f2 * OBS.C2[i][j]) / (f1 + f2);
            GF[i][j] = OBS.L1[i][j] - f1 * OBS.L2[i][j] / f2;
            wlAmb[i][j] = MW[i][j] * -1.0;
        }
    }

    for (int i = 1; i <= 36; i++) {
        int len = 2881, num_arcs = 0;
        std::vector<std::vector<int>> arcs;
        extract_arcs(MW[i], len, arcs, num_arcs);

        for (int k = 0; k < num_arcs; ++k) {
            if (arcs[k][1] - arcs[k][0] < arc_min_len) {
                for (int e = arcs[k][0]; e <= arcs[k][1]; ++e) { OBS.C1[i][e] = 0; OBS.C2[i][e] = 0; OBS.L1[i][e] = 0; OBS.L2[i][e] = 0; }
                arcs.erase(arcs.begin() + k); --k; --num_arcs;
            }
        }

        int ARCS[37][3000][2]; ARCS[i][0][0] = num_arcs;
        for (int k = 0; k < num_arcs; ++k) { ARCS[i][k + 1][0] = arcs[k][0]; ARCS[i][k + 1][1] = arcs[k][1]; }

        detect_cycle_slip(wlAmb[i], GF[i], OBS, i, ARCS, arc_min_len, cs_epoch);

        for (int j = 1; j <= 2880; j++) AATR_LGF[i][j] = (c / f1) * OBS.L1[i][j] - (c / f2) * OBS.L2[i][j];

        std::string rot_unit = gset.rot_unit();
        std::transform(rot_unit.begin(), rot_unit.end(), rot_unit.begin(), [](unsigned char ch) { return (char)std::tolower(ch); });

        double rotNumber;
        if (rot_unit == "sec") {
            rotNumber = 30.0 * 1e16 * IONO_COEFF * (1.0 / (f1 * f1) - 1.0 / (f2 * f2));
        }
        else {
            rotNumber = 0.5 * 1e16 * IONO_COEFF * (1.0 / (f1 * f1) - 1.0 / (f2 * f2));
        }

        for (int j = 2; j <= 2880; j++) if (AATR_LGF[i][j] != 0 && AATR_LGF[i][j - 1] != 0 && cs_epoch[i][j] != 1) AATR_ROT[i][j] = (AATR_LGF[i][j] - AATR_LGF[i][j - 1]) / rotNumber;
    }

    int nBlocks = (2880 + interval_size - 1) / interval_size;
    std::vector<double> AATR_block(nBlocks, 0.0);
    int block_index = 0;

    for (int j = 1; j <= 2880; j += interval_size) {
        double sum_block = 0; int count = 0;

        for (int k = j; k < j + interval_size; k++) {
            for (int i = 1; i <= 36; i++) {
                if (AATR_ROT[i][k] != 0) {
                    double E, A;
                    calc_elevation(SP3[1].EX[i][k] * 1000, SP3[1].EY[i][k] * 1000, SP3[1].EZ[i][k] * 1000, OBS.X, OBS.Y, OBS.Z, E, A);

                    double E_rad = E * PI / 180.0, cos_E = cos(E_rad);
                    double tmp = (Re * cos_E) / (Re + h);
                    double M = 1.0 / sqrt(1.0 - tmp * tmp);

                    double AATR = (1.0 / (M * M)) * AATR_ROT[i][k];
                    sum_block += AATR * AATR; count++;
                }
            }
        }

        AATR_block[block_index] = (count > 0) ? sqrt(sum_block / count) : 0.0;
        block_index++;
    }

    std::string aatr_out_path = txt_output_path.substr(0, txt_output_path.find_last_of(".")) + "_GAL_AATR.txt";
    std::filesystem::create_directories(std::filesystem::path(aatr_out_path).parent_path());

    std::ofstream out(aatr_out_path);
    if (!out.is_open()) { logger->error("Failed to open GAL AATR output file: {}", aatr_out_path); return; }

    out << std::fixed << std::setprecision(5);
    out << std::setw(6) << "Block" << std::setw(15) << "AATR" << "\n";
    for (int k = 0; k < block_index; ++k) { char block_buf[20]; sprintf(block_buf, "Block %02d:", k + 1); out << std::setw(10) << block_buf << std::setw(12) << AATR_block[k] << "\n"; }
    out.close();

    logger->info("[{}] GAL AATR saved: {}", stationName, aatr_out_path);
}

void calc_aatr_GLO(
    obs& OBS,
    const std::string& stationName,
    sp3 SP3[],
    const std::string& txt_output_path,
    t_gcfg_ppp& gset,
    gnut::t_gsetgnss& set_gnss,                 
    std::shared_ptr<spdlog::logger> logger      
) {
    double AATR_LGF[27][3000];
    double AATR_ROT[25][2888];
    double cs_epoch[25][2881];

    memset(AATR_ROT, 0, sizeof(AATR_ROT));
    memset(cs_epoch, 0, sizeof(cs_epoch));

    int arc_min_len = gset.arc_min_length();
    int interval_size = gset.aatr_interval();

    int Fre[25] = { 99, 1, -4, 5, 6, 1, -4, 5, 6, -2, -7, 0, -1, -2, -7, 0, -1, 4, -3, 3, 2, 4, -3, 3, 2 };

    double f1[25], f2[25], lambda_wl[25];
    double MW[25][2881], GF[25][2881], wlAmb[25][2881];
    auto glo_band2freq = [&](gnut::GOBSBAND b, int k)->double {
        switch (b) {
        case gnut::BAND_1: return GLO_G1_BASE + k * GLO_G1_STEP;
        case gnut::BAND_2: return GLO_G2_BASE + k * GLO_G2_STEP;
        default:
            throw std::logic_error("Unsupported GLO band: " + gnut::gobsband2str(b));
        }
        };


    const auto bands = set_gnss.band(gnut::GLO);

    const gnut::GOBSBAND bandA = bands.at(0);   
    const gnut::GOBSBAND bandB = bands.at(1);  


    logger->info("[{}] GLO AATR uses XML bands = {} {} (FDMA per-satellite frequencies)",
        stationName, gnut::gobsband2str(bandA), gnut::gobsband2str(bandB));

    for (int i = 1; i <= 24; i++) {
        f1[i] = glo_band2freq(bandA, Fre[i]);
        f2[i] = glo_band2freq(bandB, Fre[i]);


        lambda_wl[i] = c / (f1[i] - f2[i]);

        for (int j = 1; j <= 2880; j++) {
            MW[i][j] = lambda_wl[i] * (OBS.L1[i][j] - OBS.L2[i][j])
                - (f1[i] * OBS.C1[i][j] + f2[i] * OBS.C2[i][j]) / (f1[i] + f2[i]);
            GF[i][j] = OBS.L1[i][j] - f1[i] * OBS.L2[i][j] / f2[i];
            wlAmb[i][j] = MW[i][j] * -1.0;
        }

        int len = 2881;
        int num_arcs = 0;
        std::vector<std::vector<int>> arcs;
        extract_arcs(MW[i], len, arcs, num_arcs);

        for (int k = 0; k < num_arcs; ++k) {
            if (arcs[k][1] - arcs[k][0] < arc_min_len) {
                for (int e = arcs[k][0]; e <= arcs[k][1]; ++e) {
                    OBS.C1[i][e] = 0;
                    OBS.C2[i][e] = 0;
                    OBS.L1[i][e] = 0;
                    OBS.L2[i][e] = 0;
                }
                arcs.erase(arcs.begin() + k);
                --k;
                --num_arcs;
            }
        }

        int ARCS[25][3000][2];
        ARCS[i][0][0] = num_arcs;
        for (int k = 0; k < num_arcs; ++k) {
            ARCS[i][k + 1][0] = arcs[k][0];
            ARCS[i][k + 1][1] = arcs[k][1];
        }

        detect_cycle_slip(wlAmb[i], GF[i], OBS, i, ARCS, arc_min_len, cs_epoch);

        for (int j = 1; j <= 2880; j++) {
            AATR_LGF[i][j] = (c / f1[i]) * OBS.L1[i][j] - (c / f2[i]) * OBS.L2[i][j];
        }

        std::string rot_unit = gset.rot_unit();
        std::transform(rot_unit.begin(), rot_unit.end(), rot_unit.begin(),
            [](unsigned char ch) { return (char)std::tolower(ch); });

        double rotNumber = (rot_unit == "sec")
            ? (30.0 * 1e16 * IONO_COEFF * (1.0 / (f1[i] * f1[i]) - 1.0 / (f2[i] * f2[i])))
            : (0.5 * 1e16 * IONO_COEFF * (1.0 / (f1[i] * f1[i]) - 1.0 / (f2[i] * f2[i])));

        for (int j = 2; j <= 2880; j++) {
            if (AATR_LGF[i][j] != 0 && AATR_LGF[i][j - 1] != 0 && cs_epoch[i][j] != 1) {
                AATR_ROT[i][j] = (AATR_LGF[i][j] - AATR_LGF[i][j - 1]) / rotNumber;
            }
        }
    }

    int nBlocks = (2880 + interval_size - 1) / interval_size;   
    std::vector<double> AATR_block(nBlocks, 0.0);

    int block_index = 0;

    for (int j = 1; j <= 2880; j += interval_size) {
        double sum_block = 0.0;
        int count = 0;

        for (int k = j; k < j + interval_size; k++) {
            for (int i = 1; i <= 24; i++) {
                if (AATR_ROT[i][k] != 0) {
                    double E, A;
                    calc_elevation(SP3[1].RX[i][k] * 1000, SP3[1].RY[i][k] * 1000, SP3[1].RZ[i][k] * 1000,
                        OBS.X, OBS.Y, OBS.Z, E, A);

                    double E_rad = E * PI / 180.0;
                    double cos_E = cos(E_rad);
                    double tmp = (Re * cos_E) / (Re + h);
                    double M = 1.0 / sqrt(1.0 - tmp * tmp);

                    double AATR = (1.0 / (M * M)) * AATR_ROT[i][k];
                    sum_block += AATR * AATR;
                    count++;
                }
            }
        }

        AATR_block[block_index] = (count > 0) ? sqrt(sum_block / count) : 0.0;
        block_index++;
    }

    std::string aatr_out_path =
        txt_output_path.substr(0, txt_output_path.find_last_of(".")) + "_GLO_AATR.txt";
    std::filesystem::create_directories(std::filesystem::path(aatr_out_path).parent_path());

    std::ofstream out(aatr_out_path);
    if (!out.is_open()) {
        logger->error("Failed to open GLO AATR output file: {}", aatr_out_path);
        return;
    }

    out << std::fixed << std::setprecision(5);
    out << std::setw(6) << "Block" << std::setw(15) << "AATR" << "\n";

    for (int k = 0; k < block_index; ++k) {
        char block_buf[20];
        sprintf(block_buf, "Block %02d:", k + 1);
        out << std::setw(10) << block_buf << std::setw(12) << AATR_block[k] << "\n";
    }
    out.close();

    logger->info("[{}] GLO AATR saved: {}", stationName, aatr_out_path);
}
