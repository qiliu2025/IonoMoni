﻿#include "extract_obs.h"
#include "gutils/gobs.h"
#include "gutils/gsys.h" 

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <set>
#include "gset/gsetgnss.h"

using namespace std;
using namespace gnut;


//map epoch time to a row index
static inline int epoch_row_from_time(const t_gtime& t, int step_sec = 30)
{
    double sod = t.sod();
    int row = int(std::llround(sod / step_sec)) + 1;
    if (row < 1) row = 1;
    if (row > 86400 / step_sec) row = 86400 / step_sec;
    return row;
}


namespace gnut {


    void extract_GPS_obs(
        t_gallobs* gobs,
        const std::string& station,
        std::vector<std::vector<double>>& GPS_C1,
        std::vector<std::vector<double>>& GPS_C2,
        std::vector<std::vector<double>>& GPS_L1,
        std::vector<std::vector<double>>& GPS_L2,
        std::vector<t_gtime>& epochs,
        std::vector<std::string>& GPS_sats,
        std::string& G_P1,
        std::string& G_P2,
        std::shared_ptr<spdlog::logger> logger,
        obs& OBS,
        gnut::t_gsetgnss& set_gnss
    )
    {

        const std::vector<std::string> C1_priority = { "C1W", "C1P", "C1C", "P1", "C1" };
        const std::vector<std::string> C2_priority = { "C2W", "C2P", "C2X", "C2L", "C2C", "P2", "C2" };
        const std::vector<std::string> C5_priority = { "C5X", "C5Q", "C5" };

        const std::vector<std::string> L1_priority = { "L1W", "L1C", "L1" };
        //const std::vector<std::string> L1_priority = { "L1M", "L1Y","L1W", "L1P","L1X", "L1L","L1S", "L1C", "L1" };
        const std::vector<std::string> L2_priority = { "L2W", "L2C", "L2" };
        //const std::vector<std::string> L1_priority = { "L2M", "L2Y","L2W", "L2P","L2X", "L2L","L2D", "L2C", "L2" };
        const std::vector<std::string> L5_priority = { "L5X", "L5Q", "L5I", "L5" };

        std::vector<gnut::GOBSBAND> bands;
        try {
            bands = set_gnss.band(gnut::GPS);
        }
        catch (const std::exception& e) {
            logger->warn("[{}] <gps><band> not set or invalid, fallback to 1 2. ({})", station, e.what());
        }

        if (bands.size() < 2) {
            bands = { gnut::BAND_1, gnut::BAND_2 };
            logger->warn("[{}] GPS band size < 2, fallback to 1 2", station);
        }
        if (bands.size() > 2) {
            logger->warn("[{}] GPS band size > 2, only first two will be used", station);
            bands.resize(2);
        }
        if (bands[0] == bands[1]) {
            logger->warn("[{}] GPS band duplicated, fallback to 1 2", station);
            bands = { gnut::BAND_1, gnut::BAND_2 };
        }

        auto is_ok_band = [](gnut::GOBSBAND b)->bool {
            return (b == gnut::BAND_1 || b == gnut::BAND_2 || b == gnut::BAND_5);
            };
        if (!is_ok_band(bands[0]) || !is_ok_band(bands[1])) {
            logger->warn("[{}] Unsupported GPS band in XML, fallback to 1 2", station);
            bands = { gnut::BAND_1, gnut::BAND_2 };
        }

        auto band2str_simple = [](gnut::GOBSBAND b)->std::string {

            return gnut::gobsband2str(b);
            };

        const gnut::GOBSBAND bandA = bands[0];
        const gnut::GOBSBAND bandB = bands[1];

        auto pick_code_priority = [&](gnut::GOBSBAND b) -> const std::vector<std::string>&{
            switch (b) {
            case gnut::BAND_1: return C1_priority;
            case gnut::BAND_2: return C2_priority;
            case gnut::BAND_5: return C5_priority;
            default:
                throw std::logic_error("Unsupported GPS band: " + band2str_simple(b));
            }
            };
        auto pick_phase_priority = [&](gnut::GOBSBAND b) -> const std::vector<std::string>&{
            switch (b) {
            case gnut::BAND_1: return L1_priority;
            case gnut::BAND_2: return L2_priority;
            case gnut::BAND_5: return L5_priority;
            default:
                throw std::logic_error("Unsupported GPS band: " + band2str_simple(b));
            }
            };

        const auto& CodeA_pri = pick_code_priority(bandA);
        const auto& CodeB_pri = pick_code_priority(bandB);
        const auto& PhasA_pri = pick_phase_priority(bandA);
        const auto& PhasB_pri = pick_phase_priority(bandB);

        logger->info("[{}] XML GPS bands = {} {} (freq1/freq2)",
            station, band2str_simple(bandA), band2str_simple(bandB));


        const size_t num_epochs = 2881;
        const size_t num_sats = 33;

        GPS_C1.assign(num_epochs, std::vector<double>(num_sats, 0.0));
        GPS_C2.assign(num_epochs, std::vector<double>(num_sats, 0.0));
        GPS_L1.assign(num_epochs, std::vector<double>(num_sats, 0.0));
        GPS_L2.assign(num_epochs, std::vector<double>(num_sats, 0.0));

        epochs.clear();

        std::vector<t_gtime> all_epochs = gobs->epochs(station);

        GPS_sats.clear();
        for (int s = 1; s <= 32; ++s) {
            std::ostringstream oss;
            oss << "G" << std::setw(2) << std::setfill('0') << s;
            GPS_sats.push_back(oss.str());
        }

        auto build_candidates = [](const std::vector<std::string>& types) {
            std::vector<std::pair<std::string, GOBS>> out;
            out.reserve(types.size());
            for (const auto& t : types) {
                try { out.emplace_back(t, str2gobs(t)); }
                catch (...) {}
            }
            return out;
            };

        const auto cand_CodeA = build_candidates(CodeA_pri);
        const auto cand_CodeB = build_candidates(CodeB_pri);
        const auto cand_PhasA = build_candidates(PhasA_pri);
        const auto cand_PhasB = build_candidates(PhasB_pri);

        std::vector<bool> has_CodeA(cand_CodeA.size(), false);
        std::vector<bool> has_CodeB(cand_CodeB.size(), false);
        std::vector<bool> has_PhasA(cand_PhasA.size(), false);
        std::vector<bool> has_PhasB(cand_PhasB.size(), false);

        auto contains_gobs = [](const std::vector<GOBS>& v, const GOBS& x) -> bool {
            return std::find(v.begin(), v.end(), x) != v.end();
            };

        for (size_t i = 0; i < all_epochs.size(); ++i) {
            const auto obsvec = gobs->obs(station, all_epochs[i]);
            for (const auto& ob : obsvec) {
                if (ob.sat().empty() || ob.sat()[0] != 'G') continue;
                const auto& GFst = ob.obs();

                for (size_t k = 0; k < cand_CodeA.size(); ++k)
                    if (!has_CodeA[k] && contains_gobs(GFst, cand_CodeA[k].second)) has_CodeA[k] = true;

                for (size_t k = 0; k < cand_CodeB.size(); ++k)
                    if (!has_CodeB[k] && contains_gobs(GFst, cand_CodeB[k].second)) has_CodeB[k] = true;

                for (size_t k = 0; k < cand_PhasA.size(); ++k)
                    if (!has_PhasA[k] && contains_gobs(GFst, cand_PhasA[k].second)) has_PhasA[k] = true;

                for (size_t k = 0; k < cand_PhasB.size(); ++k)
                    if (!has_PhasB[k] && contains_gobs(GFst, cand_PhasB[k].second)) has_PhasB[k] = true;
            }
        }

        auto pick_best = [](const std::vector<std::pair<std::string, GOBS>>& cand,
            const std::vector<bool>& has) -> std::string {
                for (size_t k = 0; k < cand.size(); ++k)
                    if (has[k]) return cand[k].first;
                return "";
            };

        const std::string chosen_CodeA = pick_best(cand_CodeA, has_CodeA);
        const std::string chosen_CodeB = pick_best(cand_CodeB, has_CodeB);
        const std::string chosen_PhasA = pick_best(cand_PhasA, has_PhasA);
        const std::string chosen_PhasB = pick_best(cand_PhasB, has_PhasB);

        G_P1 = chosen_CodeA;
        G_P2 = chosen_CodeB;

        logger->info("[{}] Selected GPS types by bands({} {}): CodeA={}, CodeB={}, PhasA={}, PhasB={}",
            station, band2str_simple(bandA), band2str_simple(bandB),
            chosen_CodeA, chosen_CodeB, chosen_PhasA, chosen_PhasB);

        if (chosen_CodeA.empty()) logger->warn("[{}] No valid GPS code for band {}", station, band2str_simple(bandA));
        if (chosen_CodeB.empty()) logger->warn("[{}] No valid GPS code for band {}", station, band2str_simple(bandB));
        if (chosen_PhasA.empty()) logger->warn("[{}] No valid GPS phase for band {}", station, band2str_simple(bandA));
        if (chosen_PhasB.empty()) logger->warn("[{}] No valid GPS phase for band {}", station, band2str_simple(bandB));

        const bool use_CodeA = !chosen_CodeA.empty();
        const bool use_CodeB = !chosen_CodeB.empty();
        const bool use_PhasA = !chosen_PhasA.empty();
        const bool use_PhasB = !chosen_PhasB.empty();

        GOBS g_CodeA{}, g_CodeB{}, g_PhasA{}, g_PhasB{};
        if (use_CodeA) g_CodeA = str2gobs(chosen_CodeA);
        if (use_CodeB) g_CodeB = str2gobs(chosen_CodeB);
        if (use_PhasA) g_PhasA = str2gobs(chosen_PhasA);
        if (use_PhasB) g_PhasB = str2gobs(chosen_PhasB);

        for (size_t i = 0; i < std::min<size_t>(all_epochs.size(), num_epochs - 1); ++i) {
            const t_gtime& epo = all_epochs[i];
            int row = epoch_row_from_time(epo, 30);
            if (row < 0 || row >= static_cast<int>(num_epochs)) continue;

            const auto obsvec = gobs->obs(station, epo);
            for (const auto& ob : obsvec) {
                std::string prn = ob.sat();
                if (prn.empty() || prn[0] != 'G') continue;

                auto it = std::find(GPS_sats.begin(), GPS_sats.end(), prn);
                if (it == GPS_sats.end()) continue;

                size_t j = std::distance(GPS_sats.begin(), it) + 1;
                if (j >= num_sats) continue;

                const auto& GFst = ob.obs();

                if (use_CodeA && contains_gobs(GFst, g_CodeA)) GPS_C1[row][j] = ob.getobs(g_CodeA);
                if (use_CodeB && contains_gobs(GFst, g_CodeB)) GPS_C2[row][j] = ob.getobs(g_CodeB);
                if (use_PhasA && contains_gobs(GFst, g_PhasA)) GPS_L1[row][j] = ob.getobs(g_PhasA);
                if (use_PhasB && contains_gobs(GFst, g_PhasB)) GPS_L2[row][j] = ob.getobs(g_PhasB);
            }

            epochs.push_back(epo);
        }

        for (int prn = 1; prn <= 32; ++prn) {
            for (int ep = 1; ep <= 2880; ++ep) {
                OBS.C1[prn][ep] = GPS_C1[ep][prn];
                OBS.C2[prn][ep] = GPS_C2[ep][prn];
                OBS.L1[prn][ep] = GPS_L1[ep][prn];
                OBS.L2[prn][ep] = GPS_L2[ep][prn];
            }
        }
    }


    void extract_BDS_obs(
        t_gallobs* gobs,
        const std::string& station,
        std::vector<std::vector<double>>& BDS_C1,
        std::vector<std::vector<double>>& BDS_C2,
        std::vector<std::vector<double>>& BDS_L1,
        std::vector<std::vector<double>>& BDS_L2,
        std::vector<t_gtime>& epochs,
        std::vector<std::string>& BDS_sats,
        std::string& C_P1,
        std::string& C_P2,
        std::shared_ptr<spdlog::logger> logger,
        obs& OBS,
        gnut::t_gsetgnss& set_gnss
    )
    {

        const std::vector<std::string> C2_priority = { "C2I", "C2X" };
        const std::vector<std::string> C6_priority = { "C6I", "C6X" };
        const std::vector<std::string> C7_priority = { "C7I", "C7X" };

        const std::vector<std::string> L2_priority = { "L2I", "L2X" };
        const std::vector<std::string> L6_priority = { "L6I", "L6X" };
        const std::vector<std::string> L7_priority = { "L7I", "L7X" };


        std::vector<gnut::GOBSBAND> bands;
        try {
            bands = set_gnss.band(gnut::BDS);
        }
        catch (const std::exception& e) {
            logger->warn("[{}] <bds><band> not set or invalid, fallback to 2 7. ({})", station, e.what());
        }


        if (bands.size() < 2) {
            bands = { gnut::BAND_2, gnut::BAND_7 };
            logger->warn("[{}] BDS band size < 2, fallback to 2 7", station);
        }
        if (bands.size() > 2) {
            logger->warn("[{}] BDS band size > 2, only first two will be used", station);
            bands.resize(2);
        }
        if (bands[0] == bands[1]) {
            logger->warn("[{}] BDS band duplicated, fallback to 2 7", station);
            bands = { gnut::BAND_2, gnut::BAND_7 };
        }

        auto is_ok_band = [](gnut::GOBSBAND b)->bool {
            return (b == gnut::BAND_2 || b == gnut::BAND_6 || b == gnut::BAND_7);
            };
        if (!is_ok_band(bands[0]) || !is_ok_band(bands[1])) {
            logger->warn("[{}] Unsupported BDS band in XML, fallback to 2 7", station);
            bands = { gnut::BAND_2, gnut::BAND_7 };
        }

        auto band2str_simple = [](gnut::GOBSBAND b)->std::string {
            return gnut::gobsband2str(b);
            };

        const gnut::GOBSBAND bandA = bands[0];
        const gnut::GOBSBAND bandB = bands[1];

        auto pick_code_priority = [&](gnut::GOBSBAND b) -> const std::vector<std::string>&{
            switch (b) {
            case gnut::BAND_2: return C2_priority;
            case gnut::BAND_6: return C6_priority;
            case gnut::BAND_7: return C7_priority;
            default:
                throw std::logic_error("Unsupported BDS band: " + band2str_simple(b));
            }
            };
        auto pick_phase_priority = [&](gnut::GOBSBAND b) -> const std::vector<std::string>&{
            switch (b) {
            case gnut::BAND_2: return L2_priority;
            case gnut::BAND_6: return L6_priority;
            case gnut::BAND_7: return L7_priority;
            default:
                throw std::logic_error("Unsupported BDS band: " + band2str_simple(b));
            }
            };

        const auto& CodeA_pri = pick_code_priority(bandA);
        const auto& CodeB_pri = pick_code_priority(bandB);
        const auto& PhasA_pri = pick_phase_priority(bandA);
        const auto& PhasB_pri = pick_phase_priority(bandB);

        logger->info("[{}] XML BDS bands = {} {} (freq1/freq2)",
            station, band2str_simple(bandA), band2str_simple(bandB));

        const size_t num_epochs = 2881;
        const size_t num_sats = 47;

        BDS_C1.assign(num_epochs, std::vector<double>(num_sats, 0.0));
        BDS_C2.assign(num_epochs, std::vector<double>(num_sats, 0.0));
        BDS_L1.assign(num_epochs, std::vector<double>(num_sats, 0.0));
        BDS_L2.assign(num_epochs, std::vector<double>(num_sats, 0.0));

        epochs.clear();

        std::vector<t_gtime> all_epochs = gobs->epochs(station);

        BDS_sats.clear();
        for (int s = 1; s <= 46; ++s) {
            std::ostringstream oss;
            oss << "C" << std::setw(2) << std::setfill('0') << s;
            BDS_sats.push_back(oss.str());
        }

        auto build_candidates = [](const std::vector<std::string>& types) {
            std::vector<std::pair<std::string, GOBS>> out;
            out.reserve(types.size());
            for (const auto& t : types) {
                try { out.emplace_back(t, str2gobs(t)); }
                catch (...) {}
            }
            return out;
            };

        const auto cand_CodeA = build_candidates(CodeA_pri);
        const auto cand_CodeB = build_candidates(CodeB_pri);
        const auto cand_PhasA = build_candidates(PhasA_pri);
        const auto cand_PhasB = build_candidates(PhasB_pri);

        std::vector<bool> has_CodeA(cand_CodeA.size(), false);
        std::vector<bool> has_CodeB(cand_CodeB.size(), false);
        std::vector<bool> has_PhasA(cand_PhasA.size(), false);
        std::vector<bool> has_PhasB(cand_PhasB.size(), false);

        auto contains_gobs = [](const std::vector<GOBS>& v, const GOBS& x) -> bool {
            return std::find(v.begin(), v.end(), x) != v.end();
            };

        for (size_t i = 0; i < all_epochs.size(); ++i) {
            const auto obsvec = gobs->obs(station, all_epochs[i]);
            for (const auto& ob : obsvec) {
                if (ob.sat().empty() || ob.sat()[0] != 'C') continue;
                const auto& GFst = ob.obs();

                for (size_t k = 0; k < cand_CodeA.size(); ++k)
                    if (!has_CodeA[k] && contains_gobs(GFst, cand_CodeA[k].second)) has_CodeA[k] = true;
                for (size_t k = 0; k < cand_CodeB.size(); ++k)
                    if (!has_CodeB[k] && contains_gobs(GFst, cand_CodeB[k].second)) has_CodeB[k] = true;
                for (size_t k = 0; k < cand_PhasA.size(); ++k)
                    if (!has_PhasA[k] && contains_gobs(GFst, cand_PhasA[k].second)) has_PhasA[k] = true;
                for (size_t k = 0; k < cand_PhasB.size(); ++k)
                    if (!has_PhasB[k] && contains_gobs(GFst, cand_PhasB[k].second)) has_PhasB[k] = true;
            }
        }

        auto pick_best = [](const std::vector<std::pair<std::string, GOBS>>& cand,
            const std::vector<bool>& has) -> std::string {
                for (size_t k = 0; k < cand.size(); ++k)
                    if (has[k]) return cand[k].first;
                return "";
            };

        const std::string chosen_CodeA = pick_best(cand_CodeA, has_CodeA);
        const std::string chosen_CodeB = pick_best(cand_CodeB, has_CodeB);
        const std::string chosen_PhasA = pick_best(cand_PhasA, has_PhasA);
        const std::string chosen_PhasB = pick_best(cand_PhasB, has_PhasB);

        C_P1 = chosen_CodeA;
        C_P2 = chosen_CodeB;

        logger->info("[{}] Selected BDS types by bands({} {}): CodeA={}, CodeB={}, PhasA={}, PhasB={}",
            station, band2str_simple(bandA), band2str_simple(bandB),
            chosen_CodeA, chosen_CodeB, chosen_PhasA, chosen_PhasB);

        if (chosen_CodeA.empty()) logger->warn("[{}] No valid BDS code for band {}", station, band2str_simple(bandA));
        if (chosen_CodeB.empty()) logger->warn("[{}] No valid BDS code for band {}", station, band2str_simple(bandB));
        if (chosen_PhasA.empty()) logger->warn("[{}] No valid BDS phase for band {}", station, band2str_simple(bandA));
        if (chosen_PhasB.empty()) logger->warn("[{}] No valid BDS phase for band {}", station, band2str_simple(bandB));

        const bool use_CodeA = !chosen_CodeA.empty();
        const bool use_CodeB = !chosen_CodeB.empty();
        const bool use_PhasA = !chosen_PhasA.empty();
        const bool use_PhasB = !chosen_PhasB.empty();

        GOBS g_CodeA{}, g_CodeB{}, g_PhasA{}, g_PhasB{};
        if (use_CodeA) g_CodeA = str2gobs(chosen_CodeA);
        if (use_CodeB) g_CodeB = str2gobs(chosen_CodeB);
        if (use_PhasA) g_PhasA = str2gobs(chosen_PhasA);
        if (use_PhasB) g_PhasB = str2gobs(chosen_PhasB);

        for (size_t i = 0; i < std::min<size_t>(all_epochs.size(), num_epochs - 1); ++i) {
            const t_gtime& epo = all_epochs[i];
            int row = epoch_row_from_time(epo, 30);
            if (row < 0 || row >= static_cast<int>(num_epochs)) continue;

            const auto obsvec = gobs->obs(station, epo);
            for (const auto& ob : obsvec) {
                std::string prn = ob.sat();
                if (prn.empty() || prn[0] != 'C') continue;

                auto it = std::find(BDS_sats.begin(), BDS_sats.end(), prn);
                if (it == BDS_sats.end()) continue;

                size_t j = std::distance(BDS_sats.begin(), it) + 1; // 1..46
                if (j >= num_sats) continue;

                const auto& GFst = ob.obs();

                if (use_CodeA && contains_gobs(GFst, g_CodeA)) BDS_C1[row][j] = ob.getobs(g_CodeA);
                if (use_CodeB && contains_gobs(GFst, g_CodeB)) BDS_C2[row][j] = ob.getobs(g_CodeB);
                if (use_PhasA && contains_gobs(GFst, g_PhasA)) BDS_L1[row][j] = ob.getobs(g_PhasA);
                if (use_PhasB && contains_gobs(GFst, g_PhasB)) BDS_L2[row][j] = ob.getobs(g_PhasB);
            }

            epochs.push_back(epo);
        }

        for (int prn = 1; prn <= 46; ++prn) {
            for (int ep = 1; ep <= 2880; ++ep) {
                OBS.C1[prn][ep] = BDS_C1[ep][prn];
                OBS.C2[prn][ep] = BDS_C2[ep][prn];
                OBS.L1[prn][ep] = BDS_L1[ep][prn];
                OBS.L2[prn][ep] = BDS_L2[ep][prn];
            }
        }
    }

    void extract_GAL_obs(
        t_gallobs* gobs,
        const std::string& station,
        std::vector<std::vector<double>>& GAL_C1,
        std::vector<std::vector<double>>& GAL_C2,
        std::vector<std::vector<double>>& GAL_L1,
        std::vector<std::vector<double>>& GAL_L2,
        std::vector<t_gtime>& epochs,
        std::vector<std::string>& GAL_sats,
        std::string& E_P1,
        std::string& E_P2,
        std::shared_ptr<spdlog::logger> logger,
        obs& OBS,
        gnut::t_gsetgnss& set_gnss
    )
    {

        const std::vector<std::string> C1_priority = { "C1X", "C1C", "C1" };
        const std::vector<std::string> C5_priority = { "C5X", "C5Q", "C5I", "P5" , "C5" };
        const std::vector<std::string> C7_priority = { "C7X", "C7Q", "C7I", "C7" };
        const std::vector<std::string> C8_priority = { "C8X", "C8Q", "C8I", "C8" };
        const std::vector<std::string> C6_priority = { "C6Z", "C6X", "C6C", "C6B", "C6A", "C6" };

        const std::vector<std::string> L1_priority = { "L1Z", "L1X", "L1C", "L1B", "L1A", "L1" };
        const std::vector<std::string> L5_priority = { "L5X", "L5Q", "L5I", "L5" };
        const std::vector<std::string> L7_priority = { "L7X", "L7Q", "L7I", "L7" };
        const std::vector<std::string> L8_priority = { "L8X", "L8Q", "L8I", "L8" };
        const std::vector<std::string> L6_priority = { "L6Z", "L6X", "L6C", "L6B", "L6A", "L6" };

        std::vector<gnut::GOBSBAND> bands;
        try {
            bands = set_gnss.band(gnut::GAL);
        }
        catch (const std::exception& e) {
            logger->warn("[{}] <gal><band> not set or invalid, fallback to 1 5. ({})", station, e.what());
        }

        if (bands.size() < 2) {
            bands = { gnut::BAND_1, gnut::BAND_5 };
            logger->warn("[{}] GAL band size < 2, fallback to 1 5", station);
        }
        if (bands.size() > 2) {
            logger->warn("[{}] GAL band size > 2, only first two will be used", station);
            bands.resize(2);
        }
        if (bands[0] == bands[1]) {
            logger->warn("[{}] GAL band duplicated, fallback to 1 5", station);
            bands = { gnut::BAND_1, gnut::BAND_5 };
        }

        auto is_ok_band = [](gnut::GOBSBAND b)->bool {
            return (b == gnut::BAND_1 || b == gnut::BAND_5 || b == gnut::BAND_6 ||
                b == gnut::BAND_7 || b == gnut::BAND_8);
            };
        if (!is_ok_band(bands[0]) || !is_ok_band(bands[1])) {
            logger->warn("[{}] Unsupported GAL band in XML, fallback to 1 5", station);
            bands = { gnut::BAND_1, gnut::BAND_5 };
        }

        auto band2str_simple = [](gnut::GOBSBAND b)->std::string {
            return gnut::gobsband2str(b);
            };

        const gnut::GOBSBAND bandA = bands[0];
        const gnut::GOBSBAND bandB = bands[1];

        auto pick_code_priority = [&](gnut::GOBSBAND b) -> const std::vector<std::string>&{
            switch (b) {
            case gnut::BAND_1: return C1_priority;
            case gnut::BAND_5: return C5_priority;
            case gnut::BAND_6: return C6_priority;
            case gnut::BAND_7: return C7_priority;
            case gnut::BAND_8: return C8_priority;
            default:
                throw std::logic_error("Unsupported GAL band: " + band2str_simple(b));
            }
            };
        auto pick_phase_priority = [&](gnut::GOBSBAND b) -> const std::vector<std::string>&{
            switch (b) {
            case gnut::BAND_1: return L1_priority;
            case gnut::BAND_5: return L5_priority;
            case gnut::BAND_6: return L6_priority;
            case gnut::BAND_7: return L7_priority;
            case gnut::BAND_8: return L8_priority;
            default:
                throw std::logic_error("Unsupported GAL band: " + band2str_simple(b));
            }
            };

        const auto& CodeA_pri = pick_code_priority(bandA);
        const auto& CodeB_pri = pick_code_priority(bandB);
        const auto& PhasA_pri = pick_phase_priority(bandA);
        const auto& PhasB_pri = pick_phase_priority(bandB);

        logger->info("[{}] XML GAL bands = {} {} (freq1/freq2)",
            station, band2str_simple(bandA), band2str_simple(bandB));

        const size_t num_epochs = 2881;
        const size_t num_sats = 37;

        GAL_C1.assign(num_epochs, std::vector<double>(num_sats, 0.0));
        GAL_C2.assign(num_epochs, std::vector<double>(num_sats, 0.0));
        GAL_L1.assign(num_epochs, std::vector<double>(num_sats, 0.0));
        GAL_L2.assign(num_epochs, std::vector<double>(num_sats, 0.0));

        epochs.clear();

        std::vector<t_gtime> all_epochs = gobs->epochs(station);

        GAL_sats.clear();
        for (int s = 1; s <= 36; ++s) {
            std::ostringstream oss;
            oss << "E" << std::setw(2) << std::setfill('0') << s;
            GAL_sats.push_back(oss.str());
        }

        auto build_candidates = [](const std::vector<std::string>& types) {
            std::vector<std::pair<std::string, GOBS>> out;
            out.reserve(types.size());
            for (const auto& t : types) {
                try { out.emplace_back(t, str2gobs(t)); }
                catch (...) {}
            }
            return out;
            };

        const auto cand_CodeA = build_candidates(CodeA_pri);
        const auto cand_CodeB = build_candidates(CodeB_pri);
        const auto cand_PhasA = build_candidates(PhasA_pri);
        const auto cand_PhasB = build_candidates(PhasB_pri);

        std::vector<bool> has_CodeA(cand_CodeA.size(), false);
        std::vector<bool> has_CodeB(cand_CodeB.size(), false);
        std::vector<bool> has_PhasA(cand_PhasA.size(), false);
        std::vector<bool> has_PhasB(cand_PhasB.size(), false);

        auto contains_gobs = [](const std::vector<GOBS>& v, const GOBS& x) -> bool {
            return std::find(v.begin(), v.end(), x) != v.end();
            };

        for (size_t i = 0; i < all_epochs.size(); ++i) {
            const auto obsvec = gobs->obs(station, all_epochs[i]);
            for (const auto& ob : obsvec) {
                if (ob.sat().empty() || ob.sat()[0] != 'E') continue;
                const auto& GFst = ob.obs();

                for (size_t k = 0; k < cand_CodeA.size(); ++k)
                    if (!has_CodeA[k] && contains_gobs(GFst, cand_CodeA[k].second)) has_CodeA[k] = true;
                for (size_t k = 0; k < cand_CodeB.size(); ++k)
                    if (!has_CodeB[k] && contains_gobs(GFst, cand_CodeB[k].second)) has_CodeB[k] = true;
                for (size_t k = 0; k < cand_PhasA.size(); ++k)
                    if (!has_PhasA[k] && contains_gobs(GFst, cand_PhasA[k].second)) has_PhasA[k] = true;
                for (size_t k = 0; k < cand_PhasB.size(); ++k)
                    if (!has_PhasB[k] && contains_gobs(GFst, cand_PhasB[k].second)) has_PhasB[k] = true;
            }
        }

        auto pick_best = [](const std::vector<std::pair<std::string, GOBS>>& cand,
            const std::vector<bool>& has) -> std::string {
                for (size_t k = 0; k < cand.size(); ++k)
                    if (has[k]) return cand[k].first;
                return "";
            };

        const std::string chosen_CodeA = pick_best(cand_CodeA, has_CodeA);
        const std::string chosen_CodeB = pick_best(cand_CodeB, has_CodeB);
        const std::string chosen_PhasA = pick_best(cand_PhasA, has_PhasA);
        const std::string chosen_PhasB = pick_best(cand_PhasB, has_PhasB);

        E_P1 = chosen_CodeA;
        E_P2 = chosen_CodeB;

        logger->info("[{}] Selected GAL types by bands({} {}): CodeA={}, CodeB={}, PhasA={}, PhasB={}",
            station, band2str_simple(bandA), band2str_simple(bandB),
            chosen_CodeA, chosen_CodeB, chosen_PhasA, chosen_PhasB);

        if (chosen_CodeA.empty()) logger->warn("[{}] No valid GAL code for band {}", station, band2str_simple(bandA));
        if (chosen_CodeB.empty()) logger->warn("[{}] No valid GAL code for band {}", station, band2str_simple(bandB));
        if (chosen_PhasA.empty()) logger->warn("[{}] No valid GAL phase for band {}", station, band2str_simple(bandA));
        if (chosen_PhasB.empty()) logger->warn("[{}] No valid GAL phase for band {}", station, band2str_simple(bandB));

        const bool use_CodeA = !chosen_CodeA.empty();
        const bool use_CodeB = !chosen_CodeB.empty();
        const bool use_PhasA = !chosen_PhasA.empty();
        const bool use_PhasB = !chosen_PhasB.empty();

        GOBS g_CodeA{}, g_CodeB{}, g_PhasA{}, g_PhasB{};
        if (use_CodeA) g_CodeA = str2gobs(chosen_CodeA);
        if (use_CodeB) g_CodeB = str2gobs(chosen_CodeB);
        if (use_PhasA) g_PhasA = str2gobs(chosen_PhasA);
        if (use_PhasB) g_PhasB = str2gobs(chosen_PhasB);


        for (size_t i = 0; i < std::min<size_t>(all_epochs.size(), num_epochs - 1); ++i) {
            const t_gtime& epo = all_epochs[i];
            int row = epoch_row_from_time(epo, 30);
            if (row < 0 || row >= static_cast<int>(num_epochs)) continue;

            const auto obsvec = gobs->obs(station, epo);
            for (const auto& ob : obsvec) {
                std::string prn = ob.sat();
                if (prn.empty() || prn[0] != 'E') continue;

                auto it = std::find(GAL_sats.begin(), GAL_sats.end(), prn);
                if (it == GAL_sats.end()) continue;

                size_t j = std::distance(GAL_sats.begin(), it) + 1;
                if (j >= num_sats) continue;

                const auto& GFst = ob.obs();

                if (use_CodeA && contains_gobs(GFst, g_CodeA)) GAL_C1[row][j] = ob.getobs(g_CodeA);
                if (use_CodeB && contains_gobs(GFst, g_CodeB)) GAL_C2[row][j] = ob.getobs(g_CodeB);
                if (use_PhasA && contains_gobs(GFst, g_PhasA)) GAL_L1[row][j] = ob.getobs(g_PhasA);
                if (use_PhasB && contains_gobs(GFst, g_PhasB)) GAL_L2[row][j] = ob.getobs(g_PhasB);
            }

            epochs.push_back(epo);
        }


        for (int prn = 1; prn <= 36; ++prn) {
            for (int ep = 1; ep <= 2880; ++ep) {
                OBS.C1[prn][ep] = GAL_C1[ep][prn];
                OBS.C2[prn][ep] = GAL_C2[ep][prn];
                OBS.L1[prn][ep] = GAL_L1[ep][prn];
                OBS.L2[prn][ep] = GAL_L2[ep][prn];
            }
        }
    }

    void extract_GLO_obs(
        t_gallobs* gobs, const std::string& station,
        std::vector<std::vector<double>>& GLO_C1,
        std::vector<std::vector<double>>& GLO_C2,
        std::vector<std::vector<double>>& GLO_L1,
        std::vector<std::vector<double>>& GLO_L2,
        std::vector<t_gtime>& epochs,
        std::vector<std::string>& GLO_sats,
        std::string& R_P1, std::string& R_P2,
        std::shared_ptr<spdlog::logger> logger,
        obs& OBS,
        gnut::t_gsetgnss& set_gnss
    ) {

        const std::vector<std::string> C1_priority = { "C1P", "C1C", "P1", "C1" };
        const std::vector<std::string> C2_priority = { "C2P", "C2C", "P2", "C2" };

        const std::vector<std::string> L1_priority = { "L1C", "L1P", "L1" };
        const std::vector<std::string> L2_priority = { "L2P", "L2C", "L2" };

        std::vector<gnut::GOBSBAND> bands;
        try { bands = set_gnss.band(gnut::GLO); }
        catch (const std::exception& e) {
            logger->warn("[{}] <glo><band> not set or invalid, fallback to 1 2. ({})", station, e.what());
        }


        if (bands.size() < 2) {
            bands = { gnut::BAND_1, gnut::BAND_2 };
            logger->warn("[{}] GLO band size < 2, fallback 1 2", station);
        }
        if (bands.size() > 2) {
            logger->warn("[{}] GLO band size > 2, only first two used", station);
            bands.resize(2);
        }
        if (bands[0] == bands[1]) {
            logger->warn("[{}] GLO band duplicated, fallback 1 2", station);
            bands = { gnut::BAND_1, gnut::BAND_2 };
        }

        auto is_ok_band = [](gnut::GOBSBAND b)->bool { return (b == gnut::BAND_1 || b == gnut::BAND_2); };
        if (!is_ok_band(bands[0]) || !is_ok_band(bands[1])) {
            logger->warn("[{}] Unsupported GLO band, fallback 1 2", station);
            bands = { gnut::BAND_1, gnut::BAND_2 };
        }

        const gnut::GOBSBAND bandA = bands[0];
        const gnut::GOBSBAND bandB = bands[1];

        auto band2str_simple = [](gnut::GOBSBAND b)->std::string {
            return gnut::gobsband2str(b);
            };

        logger->info("[{}] XML GLO bands = {} {} (freq1/freq2)",
            station, band2str_simple(bandA), band2str_simple(bandB));

        auto pick_code_priority = [&](gnut::GOBSBAND b)->const std::vector<std::string>&{
            switch (b) {
            case gnut::BAND_1: return C1_priority;
            case gnut::BAND_2: return C2_priority;
            default:
                throw std::logic_error("Unsupported GLO band: " + band2str_simple(b));
            }
            };

        auto pick_phase_priority = [&](gnut::GOBSBAND b)->const std::vector<std::string>&{
            switch (b) {
            case gnut::BAND_1: return L1_priority;
            case gnut::BAND_2: return L2_priority;
            default:
                throw std::logic_error("Unsupported GLO band: " + band2str_simple(b));
            }
            };

        const auto& CodeA_pri = pick_code_priority(bandA);
        const auto& CodeB_pri = pick_code_priority(bandB);
        const auto& PhasA_pri = pick_phase_priority(bandA);
        const auto& PhasB_pri = pick_phase_priority(bandB);


        const size_t num_epochs = 2881, num_sats = 25;
        GLO_C1.assign(num_epochs, std::vector<double>(num_sats, 0.0));
        GLO_C2.assign(num_epochs, std::vector<double>(num_sats, 0.0));
        GLO_L1.assign(num_epochs, std::vector<double>(num_sats, 0.0));
        GLO_L2.assign(num_epochs, std::vector<double>(num_sats, 0.0));
        epochs.clear();

        std::vector<t_gtime> all_epochs = gobs->epochs(station);

        GLO_sats.clear();
        for (int r = 1; r <= 24; ++r) {
            std::ostringstream oss;
            oss << "R" << std::setw(2) << std::setfill('0') << r;
            GLO_sats.push_back(oss.str());
        }

        auto build_candidates = [](const std::vector<std::string>& types) {
            std::vector<std::pair<std::string, GOBS>> out;
            out.reserve(types.size());
            for (const auto& t : types) {
                try { out.emplace_back(t, str2gobs(t)); }
                catch (...) {}
            }
            return out;
            };

        const auto cand_CodeA = build_candidates(CodeA_pri);
        const auto cand_CodeB = build_candidates(CodeB_pri);
        const auto cand_PhasA = build_candidates(PhasA_pri);
        const auto cand_PhasB = build_candidates(PhasB_pri);

        std::vector<bool> has_CodeA(cand_CodeA.size(), false), has_CodeB(cand_CodeB.size(), false);
        std::vector<bool> has_PhasA(cand_PhasA.size(), false), has_PhasB(cand_PhasB.size(), false);

        auto contains_gobs = [](const std::vector<GOBS>& v, const GOBS& x)->bool {
            return std::find(v.begin(), v.end(), x) != v.end();
            };

        for (size_t i = 0; i < all_epochs.size(); ++i) {
            const auto obsvec = gobs->obs(station, all_epochs[i]);
            for (const auto& ob : obsvec) {
                if (ob.sat().empty() || ob.sat()[0] != 'R') continue;
                const auto& GFst = ob.obs();

                for (size_t k = 0; k < cand_CodeA.size(); ++k)
                    if (!has_CodeA[k] && contains_gobs(GFst, cand_CodeA[k].second)) has_CodeA[k] = true;

                for (size_t k = 0; k < cand_CodeB.size(); ++k)
                    if (!has_CodeB[k] && contains_gobs(GFst, cand_CodeB[k].second)) has_CodeB[k] = true;

                for (size_t k = 0; k < cand_PhasA.size(); ++k)
                    if (!has_PhasA[k] && contains_gobs(GFst, cand_PhasA[k].second)) has_PhasA[k] = true;

                for (size_t k = 0; k < cand_PhasB.size(); ++k)
                    if (!has_PhasB[k] && contains_gobs(GFst, cand_PhasB[k].second)) has_PhasB[k] = true;
            }
        }

        auto pick_best = [](const std::vector<std::pair<std::string, GOBS>>& cand,
            const std::vector<bool>& has)->std::string {
                for (size_t k = 0; k < cand.size(); ++k)
                    if (has[k]) return cand[k].first;
                return "";
            };

        const std::string chosen_CodeA = pick_best(cand_CodeA, has_CodeA);
        const std::string chosen_CodeB = pick_best(cand_CodeB, has_CodeB);
        const std::string chosen_PhasA = pick_best(cand_PhasA, has_PhasA);
        const std::string chosen_PhasB = pick_best(cand_PhasB, has_PhasB);


        R_P1 = chosen_CodeA;
        R_P2 = chosen_CodeB;

        if (chosen_CodeA.empty()) logger->warn("[{}] No valid GLO code for band {}", station, band2str_simple(bandA));
        if (chosen_CodeB.empty()) logger->warn("[{}] No valid GLO code for band {}", station, band2str_simple(bandB));
        if (chosen_PhasA.empty()) logger->warn("[{}] No valid GLO phase for band {}", station, band2str_simple(bandA));
        if (chosen_PhasB.empty()) logger->warn("[{}] No valid GLO phase for band {}", station, band2str_simple(bandB));

        logger->info("[{}] Selected GLO types by bands({} {}): CodeA={}, CodeB={}, PhasA={}, PhasB={}",
            station, band2str_simple(bandA), band2str_simple(bandB),
            chosen_CodeA, chosen_CodeB, chosen_PhasA, chosen_PhasB);

        const bool use_CodeA = !chosen_CodeA.empty(), use_CodeB = !chosen_CodeB.empty();
        const bool use_PhasA = !chosen_PhasA.empty(), use_PhasB = !chosen_PhasB.empty();

        GOBS g_CodeA{}, g_CodeB{}, g_PhasA{}, g_PhasB{};
        if (use_CodeA) g_CodeA = str2gobs(chosen_CodeA);
        if (use_CodeB) g_CodeB = str2gobs(chosen_CodeB);
        if (use_PhasA) g_PhasA = str2gobs(chosen_PhasA);
        if (use_PhasB) g_PhasB = str2gobs(chosen_PhasB);

        for (size_t i = 0; i < std::min<size_t>(all_epochs.size(), num_epochs - 1); ++i) {
            const t_gtime& epo = all_epochs[i];
            int row = epoch_row_from_time(epo, 30);
            if (row < 0 || row >= (int)num_epochs) continue;

            const auto obsvec = gobs->obs(station, epo);
            for (const auto& ob : obsvec) {
                std::string prn = ob.sat();
                if (prn.empty() || prn[0] != 'R') continue;

                auto it = std::find(GLO_sats.begin(), GLO_sats.end(), prn);
                if (it == GLO_sats.end()) continue;

                size_t j = std::distance(GLO_sats.begin(), it) + 1;
                if (j >= num_sats) continue;

                const auto& GFst = ob.obs();
                if (use_CodeA && contains_gobs(GFst, g_CodeA)) GLO_C1[row][j] = ob.getobs(g_CodeA);
                if (use_CodeB && contains_gobs(GFst, g_CodeB)) GLO_C2[row][j] = ob.getobs(g_CodeB);
                if (use_PhasA && contains_gobs(GFst, g_PhasA)) GLO_L1[row][j] = ob.getobs(g_PhasA);
                if (use_PhasB && contains_gobs(GFst, g_PhasB)) GLO_L2[row][j] = ob.getobs(g_PhasB);
            }
            epochs.push_back(epo);
        }

        for (int prn = 1; prn <= 24; ++prn)
            for (int ep = 1; ep <= 2880; ++ep) {
                OBS.C1[prn][ep] = GLO_C1[ep][prn];
                OBS.C2[prn][ep] = GLO_C2[ep][prn];
                OBS.L1[prn][ep] = GLO_L1[ep][prn];
                OBS.L2[prn][ep] = GLO_L2[ep][prn];
            }
    }


    //void extract_GPS_SNR(
    //    t_gallobs* gobs,
    //    const std::string& station,
    //    std::vector<std::vector<double>>& GPS_S1,
    //    std::vector<std::vector<double>>& GPS_S2,
    //    std::vector<t_gtime>& epochs,
    //    std::vector<std::string>& GPS_sats,
    //    std::shared_ptr<spdlog::logger> logger,
    //    obs& OBS)
    //{
    //    // S1 priority: S1C > S1W > S1
    //    std::vector<std::string> S1_priority = { "S1C", "S1W", "S1" };
    //    // S2 priority: S2W > S2X > S2P > S2C > S2
    //    std::vector<std::string> S2_priority = { "S2W", "S2X", "S2P", "S2C", "S2" };

    //    const size_t num_epochs = 3601;
    //    const size_t num_sats = 33;

    //    // Initialize output arrays with zeros
    //    GPS_S1.assign(num_epochs, std::vector<double>(num_sats, 0.0));
    //    GPS_S2.assign(num_epochs, std::vector<double>(num_sats, 0.0));

    //    std::vector<t_gtime> all_epochs = gobs->epochs(station);

    //    GPS_sats.clear();
    //    for (int s = 1; s <= 32; ++s) {
    //        std::ostringstream oss;
    //        oss << "G" << std::setw(2) << std::setfill('0') << s;
    //        GPS_sats.push_back(oss.str());
    //    }

    //    // Auto-select S1/S2 observation code based on availability and priority
    //    std::string chosen_S1 = "";
    //    std::string chosen_S2 = "";

    //    for (size_t i = 0; i < all_epochs.size(); ++i) {
    //        std::vector<t_gsatdata> obsvec = gobs->obs(station, all_epochs[i]);
    //        for (const auto& obs : obsvec) {
    //            if (obs.sat()[0] != 'G') continue;
    //            const auto& GFst = obs.obs();

    //            if (chosen_S1.empty()) {
    //                for (const auto& type : S1_priority) {
    //                    if (std::find(GFst.begin(), GFst.end(), str2gobs(type)) != GFst.end()) {
    //                        chosen_S1 = type;
    //                        break;
    //                    }
    //                }
    //            }
    //            if (chosen_S2.empty()) {
    //                for (const auto& type : S2_priority) {
    //                    if (std::find(GFst.begin(), GFst.end(), str2gobs(type)) != GFst.end()) {
    //                        chosen_S2 = type;
    //                        break;
    //                    }
    //                }
    //            }
    //            if (!chosen_S1.empty() && !chosen_S2.empty()) break;
    //        }
    //        if (!chosen_S1.empty() && !chosen_S2.empty()) break;
    //    }

    //    logger->info("[{}] Selected SNR types: S1={}, S2={}", station, chosen_S1, chosen_S2);

    //    if (chosen_S1.empty()) logger->warn("[{}] No S1 SNR observation type found!", station);
    //    if (chosen_S2.empty()) logger->warn("[{}] No S2 SNR observation type found!", station);

    //    // Fill the 2D SNR arrays for each epoch and satellite
    //    for (size_t i = 0; i < std::min<size_t>(all_epochs.size(), num_epochs - 1); ++i) {
    //        const t_gtime& epo = all_epochs[i];
    //        std::vector<t_gsatdata> obsvec = gobs->obs(station, epo);

    //        int row = epoch_row_from_time(epo, 24);

    //        for (const auto& obs : obsvec) {
    //            std::string prn = obs.sat();
    //            if (prn[0] != 'G') continue;

    //            auto it = std::find(GPS_sats.begin(), GPS_sats.end(), prn);
    //            if (it == GPS_sats.end()) continue;
    //            size_t j = std::distance(GPS_sats.begin(), it) + 1;
    //            if (j >= num_sats) continue;

    //            const auto& GFst = obs.obs();

    //            if (!chosen_S1.empty() && std::find(GFst.begin(), GFst.end(), str2gobs(chosen_S1)) != GFst.end())
    //                GPS_S1[row][j] = obs.getobs(str2gobs(chosen_S1));

    //            if (!chosen_S2.empty() && std::find(GFst.begin(), GFst.end(), str2gobs(chosen_S2)) != GFst.end())
    //                GPS_S2[row][j] = obs.getobs(str2gobs(chosen_S2));
    //        }
    //        epochs.push_back(epo);
    //    }

    //    // Optionally: map to OBS struct (indices: [sat][epoch])
    //    for (int i = 1; i <= 32; i++) {
    //        for (int j = 1; j <= 3600; j++) {
    //            OBS.S1[i][j] = GPS_S1[j][i];
    //            OBS.S2[i][j] = GPS_S2[j][i];
    //        }
    //    }

    //    //// Output S1.txt file: rows for epochs, columns for PRNs
    //    //std::ofstream fout_s1("S1.txt");
    //    //for (int j = 1; j <= 3600; j++) {
    //    //    for (int i = 1; i <= 32; i++) {
    //    //        fout_s1 << OBS.S1[i][j];
    //    //        if (i < 32) fout_s1 << "\t";
    //    //    }
    //    //    fout_s1 << std::endl;
    //    //}
    //    //fout_s1.close();

    //    //// Output S2.txt file: rows for epochs, columns for PRNs
    //    //std::ofstream fout_s2("S2.txt");
    //    //for (int j = 1; j <= 3600; j++) {
    //    //    for (int i = 1; i <= 32; i++) {
    //    //        fout_s2 << OBS.S2[i][j];
    //    //        if (i < 32) fout_s2 << "\t";
    //    //    }
    //    //    fout_s2 << std::endl;
    //    //}
    //    //fout_s2.close();

    //}

}