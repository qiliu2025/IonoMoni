#include "calc_ROTI.h"
#include "calc_elevation.h"
#include "constants.h"
#include <cmath>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <filesystem>
#include <iostream>   
#include "gset/gsetproc.h"
#include "gcfg_ppp.h"
#include <string>
#include <limits>

#include <vector>
#include "obs.h"  

void calc_roti_GPS(
    const obs& OBS,
    const std::string& stationName,
    const sp3& SP3,
    const std::string& txt_output_path,
    t_gcfg_ppp& gset,
    gnut::t_gsetgnss& set_gnss,
    std::shared_ptr<spdlog::logger> logger
) {

    int window_size = gset.roti_window();
    const auto bands = set_gnss.band(gnut::GPS);   

    const gnut::GOBSBAND bandA = bands.at(0);
    const gnut::GOBSBAND bandB = bands.at(1);

    auto band2str_simple = [](gnut::GOBSBAND b)->std::string {
        return gnut::gobsband2str(b); 
        };

    auto gps_band2freq = [&](gnut::GOBSBAND b)->double {
        switch (b) {
        case gnut::BAND_1: return GPS_F1;
        case gnut::BAND_2: return GPS_F2;
        case gnut::BAND_5: return GPS_F5;
        default:
            throw std::logic_error("Unsupported GPS band: " + gnut::gobsband2str(b));
        }
        };



    const double f1 = gps_band2freq(bandA); 
    const double f2 = gps_band2freq(bandB);

    logger->info("[{}] GPS ROTI using XML bands = {} {} (freq1/freq2), f1={} Hz, f2={} Hz",
        stationName,
        band2str_simple(bandA), band2str_simple(bandB),
        f1, f2);


    double LGF[33][2881] = { 0 };  
    double ROT[33][2881] = { 0 };
    double ROTI[33][2881] = { 0 };

    for (int i = 1; i <= 32; i++) {

        for (int j = 1; j <= 2880; j++) {
            if (OBS.L1[i][j] != 0.0 && OBS.L2[i][j] != 0.0) {
                LGF[i][j] = (c / f1) * OBS.L1[i][j] - (c / f2) * OBS.L2[i][j];
            }
            else {
                LGF[i][j] = 0.0;
            }
        }

        std::string rot_unit = gset.rot_unit();
        std::transform(rot_unit.begin(), rot_unit.end(), rot_unit.begin(), ::tolower);

        double rotNumber = 0.0;
        if (rot_unit == "sec") {
            rotNumber = 30.0 * 1e16 * IONO_COEFF * (1.0 / (f1 * f1) - 1.0 / (f2 * f2));
        }
        else {
            rotNumber = 0.5 * 1e16 * IONO_COEFF * (1.0 / (f1 * f1) - 1.0 / (f2 * f2));
        }

        for (int j = 2; j <= 2880; j++) {
            if (LGF[i][j] != 0.0 && LGF[i][j - 1] != 0.0) {
                ROT[i][j] = (LGF[i][j] - LGF[i][j - 1]) / rotNumber;
            }
        }

        for (int j = window_size; j <= 2880; j++) {
            double sum = 0.0, var = 0.0;
            bool valid = true;

            for (int k = j - window_size + 1; k <= j; k++) {
                if (ROT[i][k] == 0.0) { valid = false; break; }
                sum += ROT[i][k];
            }
            if (!valid) continue;

            double mean = sum / window_size;
            for (int k = j - window_size + 1; k <= j; k++) {
                var += (ROT[i][k] - mean) * (ROT[i][k] - mean);
            }

            ROTI[i][j] = std::sqrt(var / window_size);
        }
    }

    std::string gps_roti_path = txt_output_path.substr(0, txt_output_path.find_last_of(".")) + "_GPS_ROTI.txt";
    std::string gps_rot_path = txt_output_path.substr(0, txt_output_path.find_last_of(".")) + "_GPS_ROT.txt";

    std::ofstream out(gps_roti_path), rot(gps_rot_path);
    if (!out.is_open() || !rot.is_open()) {
        logger->error("Failed to open GPS ROTI/ROT output file: {} / {}", gps_roti_path, gps_rot_path);
        return;
    }

    out << std::fixed << std::setprecision(4);
    rot << std::fixed << std::setprecision(4);

    out << std::setw(12) << "Epoch \\ PRN";
    rot << std::setw(12) << "Epoch \\ PRN";
    for (int i = 1; i <= 32; ++i) {
        char prn_buf[10];
        sprintf(prn_buf, "G%02d", i);
        out << std::setw(11) << prn_buf;
        rot << std::setw(11) << prn_buf;
    }
    out << "\n";
    rot << "\n";

    for (int j = 1; j <= 2880; ++j) {
        char epoch_buf[20];
        sprintf(epoch_buf, "Epoch %04d:", j);
        out << std::setw(12) << epoch_buf;
        rot << std::setw(12) << epoch_buf;

        for (int i = 1; i <= 32; ++i) {
            out << std::setw(11) << ROTI[i][j];
            rot << std::setw(11) << ROT[i][j];
        }
        out << "\n";
        rot << "\n";
    }

    out.close();
    rot.close();

    logger->info("[{}] GPS ROT/ROTI saved: {}, {}", stationName, gps_rot_path, gps_roti_path);
}


void calc_roti_BDS(
    const obs& OBS,
    const std::string& stationName,
    const sp3& SP3,
    const std::string& txt_output_path,
    t_gcfg_ppp& gset,
    gnut::t_gsetgnss& set_gnss,
    std::shared_ptr<spdlog::logger> logger
) {
 
    int window_size = gset.roti_window();
    std::vector<gnut::GOBSBAND> bands;
    try {
        bands = set_gnss.band(gnut::BDS);  
    }
    catch (const std::exception& e) {
        logger->error("[{}] Failed to read <bds><band>: {}", stationName, e.what());
        throw; 
    }

    if (bands.size() != 2) {
        logger->error("[{}] Invalid <bds><band>: expect exactly 2 items, but got {}",
            stationName, bands.size());
        throw std::runtime_error("Invalid <bds><band>: must provide exactly 2 items.");
    }

    const gnut::GOBSBAND bandA = bands.at(0);
    const gnut::GOBSBAND bandB = bands.at(1);

    if (bandA == bandB) {
        logger->error("[{}] Invalid <bds><band>: duplicated bands {} {}",
            stationName, gnut::gobsband2str(bandA), gnut::gobsband2str(bandB));
        throw std::runtime_error("Invalid <bds><band>: two bands must be different.");
    }

    auto is_ok_band = [](gnut::GOBSBAND b)->bool {
        return (b == gnut::BAND_2 || b == gnut::BAND_6 || b == gnut::BAND_7);
        };

    if (!is_ok_band(bandA) || !is_ok_band(bandB)) {
        logger->error("[{}] Invalid <bds><band>: only 2/6/7 allowed, but got {} {}",
            stationName, gnut::gobsband2str(bandA), gnut::gobsband2str(bandB));
        throw std::runtime_error("Invalid <bds><band>: only 2/6/7 are allowed.");
    }

  

    auto band2str_simple = [](gnut::GOBSBAND b)->std::string {
        return gnut::gobsband2str(b); 
        };

    auto bds_band2freq = [&](gnut::GOBSBAND b)->double {
        switch (b) {
        case gnut::BAND_2: return BDS_F2;
        case gnut::BAND_6: return BDS_F6;
        case gnut::BAND_7: return BDS_F7;
        default:
            throw std::logic_error("Unsupported BDS band: " + gnut::gobsband2str(b));
        }
        };

    const double f1 = bds_band2freq(bandA);
    const double f2 = bds_band2freq(bandB);

    logger->info("[{}] BDS ROTI using XML bands = {} {} (freq1/freq2), f1={} Hz, f2={} Hz",
        stationName,
        band2str_simple(bandA), band2str_simple(bandB),
        f1, f2);

    double LGF[47][2881] = { 0 };   
    double ROT[47][2881] = { 0 };
    double ROTI[47][2881] = { 0 };

    for (int i = 1; i <= 46; i++) {

        for (int j = 1; j <= 2880; j++) {
            if (OBS.L1[i][j] != 0.0 && OBS.L2[i][j] != 0.0) {
                LGF[i][j] = (c / f1) * OBS.L1[i][j] - (c / f2) * OBS.L2[i][j];
            }
            else {
                LGF[i][j] = 0.0;
            }
        }

        std::string rot_unit = gset.rot_unit();
        std::transform(rot_unit.begin(), rot_unit.end(), rot_unit.begin(), ::tolower);

        double rotNumber = 0.0;
        if (rot_unit == "sec") {
            rotNumber = 30.0 * 1e16 * IONO_COEFF * (1.0 / (f1 * f1) - 1.0 / (f2 * f2));
        }
        else {
            rotNumber = 0.5 * 1e16 * IONO_COEFF * (1.0 / (f1 * f1) - 1.0 / (f2 * f2));
        }

        for (int j = 2; j <= 2880; j++) {
            if (LGF[i][j] != 0.0 && LGF[i][j - 1] != 0.0) {
                ROT[i][j] = (LGF[i][j] - LGF[i][j - 1]) / rotNumber;
            }
        }

        for (int j = window_size; j <= 2880; j++) {
            double sum = 0.0, var = 0.0;
            bool valid = true;

            for (int k = j - window_size + 1; k <= j; k++) {
                if (ROT[i][k] == 0.0) { valid = false; break; }
                sum += ROT[i][k];
            }
            if (!valid) continue;

            double mean = sum / window_size;
            for (int k = j - window_size + 1; k <= j; k++) {
                var += (ROT[i][k] - mean) * (ROT[i][k] - mean);
            }

            ROTI[i][j] = std::sqrt(var / window_size);
        }
    }

    std::string bds_roti_path = txt_output_path.substr(0, txt_output_path.find_last_of(".")) + "_BDS_ROTI.txt";
    std::string bds_rot_path = txt_output_path.substr(0, txt_output_path.find_last_of(".")) + "_BDS_ROT.txt";

    std::ofstream out(bds_roti_path), rot(bds_rot_path);
    if (!out.is_open() || !rot.is_open()) {
        logger->error("Failed to open BDS ROTI/ROT output file: {} / {}", bds_roti_path, bds_rot_path);
        return;
    }

    out << std::fixed << std::setprecision(4);
    rot << std::fixed << std::setprecision(4);

    out << std::setw(12) << "Epoch \\ PRN";
    rot << std::setw(12) << "Epoch \\ PRN";
    for (int i = 1; i <= 46; ++i) {
        char prn_buf[10];
        sprintf(prn_buf, "C%02d", i);
        out << std::setw(11) << prn_buf;
        rot << std::setw(11) << prn_buf;
    }
    out << "\n";
    rot << "\n";

    for (int j = 1; j <= 2880; ++j) {
        char epoch_buf[20];
        sprintf(epoch_buf, "Epoch %04d:", j);
        out << std::setw(12) << epoch_buf;
        rot << std::setw(12) << epoch_buf;

        for (int i = 1; i <= 46; ++i) {
            out << std::setw(11) << ROTI[i][j];
            rot << std::setw(11) << ROT[i][j];
        }
        out << "\n";
        rot << "\n";
    }

    out.close();
    rot.close();

    logger->info("[{}] BDS ROT/ROTI saved: {}, {}", stationName, bds_rot_path, bds_roti_path);
}

void calc_roti_GAL(
    const obs& OBS,
    const std::string& stationName,
    const sp3& SP3,
    const std::string& txt_output_path,
    t_gcfg_ppp& gset,
    gnut::t_gsetgnss& set_gnss,
    std::shared_ptr<spdlog::logger> logger
) {
    (void)SP3;

    int window_size = gset.roti_window();

    const auto bands = set_gnss.band(gnut::GAL);  
    const gnut::GOBSBAND bandA = bands.at(0);
    const gnut::GOBSBAND bandB = bands.at(1);
    auto band2str_simple = [](gnut::GOBSBAND b)->std::string { return gnut::gobsband2str(b); };
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



    const double f1 = gal_band2freq(bandA);
    const double f2 = gal_band2freq(bandB);

    logger->info("[{}] GAL ROTI using XML bands = {} {} (freq1/freq2), f1={} Hz, f2={} Hz",
        stationName, band2str_simple(bandA), band2str_simple(bandB), f1, f2);

    double LGF[37][2881] = { 0 };
    double ROT[37][2881] = { 0 };
    double ROTI[37][2881] = { 0 };

    for (int i = 1; i <= 36; i++) {

        for (int j = 1; j <= 2880; j++) {
            if (OBS.L1[i][j] != 0.0 && OBS.L2[i][j] != 0.0) LGF[i][j] = (c / f1) * OBS.L1[i][j] - (c / f2) * OBS.L2[i][j];
            else LGF[i][j] = 0.0;
        }

        std::string rot_unit = gset.rot_unit();
        std::transform(rot_unit.begin(), rot_unit.end(), rot_unit.begin(), ::tolower);

        double rotNumber = 0.0;
        if (rot_unit == "sec") rotNumber = 30.0 * 1e16 * IONO_COEFF * (1.0 / (f1 * f1) - 1.0 / (f2 * f2));
        else rotNumber = 0.5 * 1e16 * IONO_COEFF * (1.0 / (f1 * f1) - 1.0 / (f2 * f2));

        for (int j = 2; j <= 2880; j++) {
            if (LGF[i][j] != 0.0 && LGF[i][j - 1] != 0.0) ROT[i][j] = (LGF[i][j] - LGF[i][j - 1]) / rotNumber;
        }

        for (int j = window_size; j <= 2880; j++) {
            double sum = 0.0, var = 0.0; bool valid = true;
            for (int k = j - window_size + 1; k <= j; k++) { if (ROT[i][k] == 0.0) { valid = false; break; } sum += ROT[i][k]; }
            if (!valid) continue;

            double mean = sum / window_size;
            for (int k = j - window_size + 1; k <= j; k++) var += (ROT[i][k] - mean) * (ROT[i][k] - mean);
            ROTI[i][j] = std::sqrt(var / window_size);
        }
    }

    std::string gal_roti_path = txt_output_path.substr(0, txt_output_path.find_last_of(".")) + "_GAL_ROTI.txt";
    std::string gal_rot_path = txt_output_path.substr(0, txt_output_path.find_last_of(".")) + "_GAL_ROT.txt";

    std::ofstream out(gal_roti_path), rot(gal_rot_path);
    if (!out.is_open() || !rot.is_open()) { logger->error("Failed to open GAL ROTI/ROT output file: {} / {}", gal_roti_path, gal_rot_path); return; }

    out << std::fixed << std::setprecision(4);
    rot << std::fixed << std::setprecision(4);

    out << std::setw(12) << "Epoch \\ PRN";
    rot << std::setw(12) << "Epoch \\ PRN";
    for (int i = 1; i <= 36; ++i) {
        char prn_buf[10]; sprintf(prn_buf, "E%02d", i);
        out << std::setw(11) << prn_buf;
        rot << std::setw(11) << prn_buf;
    }
    out << "\n"; rot << "\n";

    for (int j = 1; j <= 2880; ++j) {
        char epoch_buf[20]; sprintf(epoch_buf, "Epoch %04d:", j);
        out << std::setw(12) << epoch_buf;
        rot << std::setw(12) << epoch_buf;

        for (int i = 1; i <= 36; ++i) {
            out << std::setw(11) << ROTI[i][j];
            rot << std::setw(11) << ROT[i][j];
        }
        out << "\n"; rot << "\n";
    }

    out.close(); rot.close();
    logger->info("[{}] GAL ROT/ROTI saved: {}, {}", stationName, gal_rot_path, gal_roti_path);
}


void calc_roti_GLO(
    const obs& OBS,
    const std::string& stationName,
    const sp3& SP3,
    const std::string& txt_output_path,
    t_gcfg_ppp& gset,
    gnut::t_gsetgnss& set_gnss,
    std::shared_ptr<spdlog::logger> logger
) {
    (void)SP3;

    int window_size = gset.roti_window();

    const auto bands = set_gnss.band(gnut::GLO); 
    const gnut::GOBSBAND bandA = bands.at(0);
    const gnut::GOBSBAND bandB = bands.at(1);

    auto glo_band2freq = [&](gnut::GOBSBAND b, int k)->double {
        switch (b) {
        case gnut::BAND_1: return GLO_G1_BASE + k * GLO_G1_STEP; 
        case gnut::BAND_2: return GLO_G2_BASE + k * GLO_G2_STEP;
        default:
            throw std::logic_error("Unsupported GLO band: " + gnut::gobsband2str(b));
        }
        };

    auto band2str_simple = [](gnut::GOBSBAND b)->std::string { return gnut::gobsband2str(b); };
       
    logger->info("[{}] GLO ROTI using XML bands = {} {} (freq1/freq2)",
        stationName, band2str_simple(bandA), band2str_simple(bandB));

    const int fre[25] = { 99, 1, -4, 5, 6, 1, -4, 5, 6, -2, -7, 0, -1, -2, -7, 0, -1, 4, -3, 3, 2, 4, -3, 3, 2 };

    double LGF[25][2881] = { 0 };
    double ROT[25][2881] = { 0 };
    double ROTI[25][2881] = { 0 };

    std::string rot_unit = gset.rot_unit();
    std::transform(rot_unit.begin(), rot_unit.end(), rot_unit.begin(), ::tolower);

    for (int i = 1; i <= 24; i++) {

        const double f1 = glo_band2freq(bandA, fre[i]);
        const double f2 = glo_band2freq(bandB, fre[i]);


        for (int j = 1; j <= 2880; j++) {
            if (OBS.L1[i][j] != 0.0 && OBS.L2[i][j] != 0.0)
                LGF[i][j] = (c / f1) * OBS.L1[i][j] - (c / f2) * OBS.L2[i][j];
            else
                LGF[i][j] = 0.0;
        }

        double rotNumber = 0.0;
        if (rot_unit == "sec") rotNumber = 30.0 * 1e16 * IONO_COEFF * (1.0 / (f1 * f1) - 1.0 / (f2 * f2));
        else                  rotNumber = 0.5 * 1e16 * IONO_COEFF * (1.0 / (f1 * f1) - 1.0 / (f2 * f2));

        for (int j = 2; j <= 2880; j++) {
            if (LGF[i][j] != 0.0 && LGF[i][j - 1] != 0.0)
                ROT[i][j] = (LGF[i][j] - LGF[i][j - 1]) / rotNumber;
        }

        for (int j = window_size; j <= 2880; j++) {
            double sum = 0.0, var = 0.0; bool valid = true;
            for (int k = j - window_size + 1; k <= j; k++) { if (ROT[i][k] == 0.0) { valid = false; break; } sum += ROT[i][k]; }
            if (!valid) continue;

            double mean = sum / window_size;
            for (int k = j - window_size + 1; k <= j; k++) var += (ROT[i][k] - mean) * (ROT[i][k] - mean);
            ROTI[i][j] = std::sqrt(var / window_size);
        }
    }

    std::string glo_roti_path = txt_output_path.substr(0, txt_output_path.find_last_of(".")) + "_GLO_ROTI.txt";
    std::string glo_rot_path = txt_output_path.substr(0, txt_output_path.find_last_of(".")) + "_GLO_ROT.txt";

    std::ofstream out(glo_roti_path), rot(glo_rot_path);
    if (!out.is_open() || !rot.is_open()) { logger->error("Failed to open GLO ROTI/ROT output file: {} / {}", glo_roti_path, glo_rot_path); return; }

    out << std::fixed << std::setprecision(4);
    rot << std::fixed << std::setprecision(4);

    out << std::setw(12) << "Epoch \\ PRN";
    rot << std::setw(12) << "Epoch \\ PRN";
    for (int i = 1; i <= 24; ++i) { char prn_buf[10]; sprintf(prn_buf, "R%02d", i); out << std::setw(11) << prn_buf; rot << std::setw(11) << prn_buf; }
    out << "\n"; rot << "\n";

    for (int j = 1; j <= 2880; ++j) {
        char epoch_buf[20]; sprintf(epoch_buf, "Epoch %04d:", j);
        out << std::setw(12) << epoch_buf; rot << std::setw(12) << epoch_buf;
        for (int i = 1; i <= 24; ++i) { out << std::setw(11) << ROTI[i][j]; rot << std::setw(11) << ROT[i][j]; }
        out << "\n"; rot << "\n";
    }

    out.close(); rot.close();
    logger->info("[{}] GLO ROT/ROTI saved: {}, {}", stationName, glo_rot_path, glo_roti_path);
}

//void calc_S4C(const obs& OBS,
//    int numSats, int numEpochs,
//    int n_trend, int L_stat,
//    const std::string& txt_output_path,
//    const std::string& systemTag,
//    int hourWanted)
//{
//    const double NaN = std::numeric_limits<double>::quiet_NaN();
//
//    std::vector<std::vector<double>> S4C_S1(numSats + 1, std::vector<double>(numEpochs + 1, 0.0));
//    std::vector<std::vector<double>> S4C_S2(numSats + 1, std::vector<double>(numEpochs + 1, 0.0));
//
//    for (int prn = 1; prn <= numSats; ++prn)
//    {
//        // (1) dB-Hz -> linear
//        std::vector<double> s1_lin(numEpochs + 1, NaN), s2_lin(numEpochs + 1, NaN);
//        for (int k = 1; k <= numEpochs; ++k) {
//            double c1 = OBS.S1[prn][k], c2 = OBS.S2[prn][k];
//            if (std::isfinite(c1) && c1 > 0.0) s1_lin[k] = std::pow(10.0, 0.1 * c1);
//            if (std::isfinite(c2) && c2 > 0.0) s2_lin[k] = std::pow(10.0, 0.1 * c2);
//        }
//
//        // (2) Detrending: x[k] = s_lin[k] / mean( s_lin[k-n_trend ... k-1] )
//        std::vector<double> x1(numEpochs + 1, NaN), x2(numEpochs + 1, NaN);
//        double sumPrev1 = 0.0, sumPrev2 = 0.0;
//        int    cntPrev1 = 0, cntPrev2 = 0;
//
//        for (int k = 1; k <= numEpochs; ++k)
//        {
//            if (k - 1 >= 1) {
//                if (std::isfinite(s1_lin[k - 1])) { sumPrev1 += s1_lin[k - 1]; cntPrev1++; }
//                if (std::isfinite(s2_lin[k - 1])) { sumPrev2 += s2_lin[k - 1]; cntPrev2++; }
//            }
//            if (k - n_trend - 1 >= 1) {
//                if (std::isfinite(s1_lin[k - n_trend - 1])) { sumPrev1 -= s1_lin[k - n_trend - 1]; cntPrev1--; }
//                if (std::isfinite(s2_lin[k - n_trend - 1])) { sumPrev2 -= s2_lin[k - n_trend - 1]; cntPrev2--; }
//            }
//
//            if (cntPrev1 == n_trend && std::isfinite(s1_lin[k]) && sumPrev1 > 0.0)
//                x1[k] = s1_lin[k] / (sumPrev1 / n_trend);
//            if (cntPrev2 == n_trend && std::isfinite(s2_lin[k]) && sumPrev2 > 0.0)
//                x2[k] = s2_lin[k] / (sumPrev2 / n_trend);
//        }
//
//        // (3) S4C: sliding window of length L_stat on x
//        double sumX1 = 0.0, sumX1sq = 0.0; int cntX1 = 0;
//        double sumX2 = 0.0, sumX2sq = 0.0; int cntX2 = 0;
//
//        for (int k = 1; k <= numEpochs; ++k)
//        {
//            if (std::isfinite(x1[k])) { sumX1 += x1[k]; sumX1sq += x1[k] * x1[k]; cntX1++; }
//            if (std::isfinite(x2[k])) { sumX2 += x2[k]; sumX2sq += x2[k] * x2[k]; cntX2++; }
//
//            if (k - L_stat >= 1) {
//                if (std::isfinite(x1[k - L_stat])) { sumX1 -= x1[k - L_stat]; sumX1sq -= x1[k - L_stat] * x1[k - L_stat]; cntX1--; }
//                if (std::isfinite(x2[k - L_stat])) { sumX2 -= x2[k - L_stat]; sumX2sq -= x2[k - L_stat] * x2[k - L_stat]; cntX2--; }
//            }
//
//            // First computable epoch: k >= n_trend + L_stat, and the window has L_stat valid samples
//            if (k >= n_trend + L_stat)
//            {
//                if (cntX1 == L_stat) {
//                    double m1 = sumX1 / L_stat;
//                    double v1 = std::fmax(sumX1sq / L_stat - m1 * m1, 0.0);
//                    S4C_S1[prn][k] = (m1 > 0.0) ? std::sqrt(v1) / m1 : 0.0;
//                }
//                if (cntX2 == L_stat) {
//                    double m2 = sumX2 / L_stat;
//                    double v2 = std::fmax(sumX2sq / L_stat - m2 * m2, 0.0);
//                    S4C_S2[prn][k] = (m2 > 0.0) ? std::sqrt(v2) / m2 : 0.0;
//                }
//            }
//        }
//    }
//
//    namespace fs = std::filesystem;
//    fs::path inpath(txt_output_path);
//    fs::path dir = inpath.parent_path();
//    std::error_code ec;
//    fs::create_directories(dir, ec);
//
//    std::string stem = inpath.stem().string();
//    size_t us = stem.find('_');
//    std::string station = (us == std::string::npos) ? stem : stem.substr(0, us);
//
//    // hourWanted: "H01".."H24"
//    std::ostringstream hs;
//    hs << 'H' << std::setw(2) << std::setfill('0') << std::clamp(hourWanted, 1, 24);
//    std::string hourTag = hs.str();
//
//    fs::path pathS1 = dir / (station + "_" + systemTag + "_S4C_S1_" + hourTag + ".txt");
//    fs::path pathS2 = dir / (station + "_" + systemTag + "_S4C_S2_" + hourTag + ".txt");
//
//    std::ofstream fout1(pathS1.string()), fout2(pathS2.string());
//    if (!fout1.is_open() || !fout2.is_open()) {
//        std::cerr << "Failed to open S4C output files: "
//            << pathS1 << " / " << pathS2 << "\n";
//        return;
//    }
//
//    fout1 << std::fixed << std::setprecision(5);
//    fout2 << std::fixed << std::setprecision(5);
//
//    auto write_header = [&](std::ofstream& f) {
//        f << std::setw(12) << "Epoch \\ PRN";
//        for (int i = 1; i <= numSats; ++i) {
//            char prn_buf[10];
//            std::sprintf(prn_buf, "PRN%02d", i); // If you need to distinguish systems, change to G/C/E/R
//            f << std::setw(11) << prn_buf;
//        }
//        f << "\n";
//        };
//    write_header(fout1);
//    write_header(fout2);
//
//    for (int j = 1; j <= numEpochs; ++j)
//    {
//        char epoch_buf[20];
//        std::sprintf(epoch_buf, "Epoch %04d:", j);
//        fout1 << std::setw(12) << epoch_buf;
//        fout2 << std::setw(12) << epoch_buf;
//
//        for (int prn = 1; prn <= numSats; ++prn)
//        {
//            fout1 << std::setw(11) << S4C_S1[prn][j];
//            fout2 << std::setw(11) << S4C_S2[prn][j];
//        }
//        fout1 << "\n";
//        fout2 << "\n";
//    }
//
//    fout1.close();
//    fout2.close();
//}