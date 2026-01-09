#include "read_DCB.h"
#include <fstream>
#include <string>
#include <vector>
#include <set>


bool readGPSDCB(const std::string& dcbFilePath,
    const std::string& stationName,
    std::vector<double>& satelliteDCB,
    double& receiverDCB,
    const std::string& G_P1,                 
    const std::string& G_P2,                
    std::set<int>& missingPRNs,
    std::shared_ptr<spdlog::logger> my_logger)
{
    std::ifstream file(dcbFilePath);
    if (!file.is_open()) {
        my_logger->error("Failed to open DCB file: {}", dcbFilePath);
        return false;
    }

    auto to_upper = [](std::string s) {
        for (auto& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        return s;
        };

    // rinex2 to rinex3
    auto normalize_code = [&](std::string code) -> std::string {
        code = to_upper(code);

        if (code == "P1") return "C1W";
        if (code == "C1") return "C1C";
        if (code == "P2") return "C2W";
        if (code == "C2") return "C2C";
        if (code == "P5") return "C5W";
        if (code == "C5") return "C5Q";

        return code; 
        };

    std::string siteKey = to_upper(stationName);
    if (siteKey.size() > 4) siteKey = siteKey.substr(0, 4);

    std::string code1 = normalize_code(G_P1);
    std::string code2 = normalize_code(G_P2);
    std::string pairKey = code1 + "  " + code2; 

    my_logger->info("[{}] Read GPS DCB using code pair: {}",
        siteKey, pairKey);

    satelliteDCB.assign(33, 0.0);
    receiverDCB = 0.0;

    bool foundReceiver = false;
    std::vector<bool> foundSat(33, false);

    std::string line;
    while (std::getline(file, line)) {
        if (line.length() < 91) continue;

        const bool isSatLine = (line.substr(0, 7) == " DSB  G" || line.substr(0, 7) == " DCB  G");
        const bool isRecLine = (line.substr(0, 12) == " DSB  G    G" || line.substr(0, 12) == " DCB  G    G");

        std::string pairField = line.substr(25, 13);

        if (isSatLine &&
            pairField.find(pairKey) != std::string::npos &&
            line.substr(15, 4) == "    ")
        {
            int prn = 0;
            try {
                prn = std::stoi(line.substr(12, 2));
            }
            catch (...) {
                continue;
            }

            double dcbValue = 0.0;
            try {
                dcbValue = std::stod(line.substr(70, 22));
            }
            catch (...) {
                continue;
            }

            if (prn >= 1 && prn <= 32) {
                satelliteDCB[prn] = dcbValue;
                foundSat[prn] = true;
            }
            continue;
        }


        if (isRecLine &&
            pairField.find(pairKey) != std::string::npos)
        {
            std::string site = to_upper(line.substr(15, 4));
            if (site == siteKey) {
                try {
                    receiverDCB = std::stod(line.substr(70, 22));
                    foundReceiver = true;
                }
                catch (...) {
                }
            }
            continue;
        }
    }

    file.close();

    missingPRNs.clear();
    for (int prn = 1; prn <= 32; ++prn) {
        if (!foundSat[prn]) {
            my_logger->warn("Satellite DCB not found for GPS PRN {} (pair {}).", prn, pairKey);
            missingPRNs.insert(prn);
        }
    }

    if (!foundReceiver) {
        my_logger->error("[{}] Receiver DCB not found (pair {}).", siteKey, pairKey);
    }

    return foundReceiver;
}



bool readBDSDCB(const std::string& dcbFilePath,
    const std::string& stationName,
    std::vector<double>& satelliteDCB,
    double& receiverDCB,
    const std::string& C_P1,
    const std::string& C_P2,
    std::set<int>& missingPRNs,
    std::shared_ptr<spdlog::logger> my_logger)
{
    std::ifstream file(dcbFilePath);
    if (!file.is_open()) {
        my_logger->error("Failed to open DCB file: {}", dcbFilePath);
        return false;
    }

    auto to_upper = [](std::string s) {
        for (auto& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        return s;
        };

    auto normalize_code = [&](std::string code) -> std::string {
        code = to_upper(code);
        // For BDS, no rinex2 to rinex3 mapping is performed for now
        return code;
        };


    std::string siteKey = to_upper(stationName);
    if (siteKey.size() > 4) siteKey = siteKey.substr(0, 4);

    std::string code1 = normalize_code(C_P1);
    std::string code2 = normalize_code(C_P2);
    std::string pairKey = code1 + "  " + code2;  

    my_logger->info("[{}] Read BDS DCB using code pair: {}", siteKey, pairKey);

    satelliteDCB.assign(47, 0.0);
    receiverDCB = 0.0;

    bool foundReceiver = false;
    std::vector<bool> foundSat(47, false);

    std::string line;
    while (std::getline(file, line)) {
        if (line.length() < 91) continue;

        const bool isSatLine = (line.substr(0, 7) == " DSB  C" || line.substr(0, 7) == " DCB  C");
        const bool isRecLine = (line.substr(0, 12) == " DSB  C    C" || line.substr(0, 12) == " DCB  C    C");

        std::string pairField = line.substr(25, 13);

        if (isSatLine &&
            pairField.find(pairKey) != std::string::npos &&
            line.substr(15, 4) == "    ")
        {
            int prn = 0;
            try {
                prn = std::stoi(line.substr(12, 2));
            }
            catch (...) {
                continue;
            }

            double dcbValue = 0.0;
            try {
                dcbValue = std::stod(line.substr(70, 22));
            }
            catch (...) {
                continue;
            }

            if (prn >= 1 && prn <= 46) {
                satelliteDCB[prn] = dcbValue;
                foundSat[prn] = true;
            }
            continue;
        }

        if (isRecLine &&
            pairField.find(pairKey) != std::string::npos)
        {
            std::string site = to_upper(line.substr(15, 4));
            if (site == siteKey) {
                try {
                    receiverDCB = std::stod(line.substr(70, 22));
                    foundReceiver = true;
                }
                catch (...) {
                }
            }
            continue;
        }
    }

    file.close();

    missingPRNs.clear();
    for (int prn = 1; prn <= 46; ++prn) {
        if (!foundSat[prn]) {
            my_logger->warn("Satellite DCB not found for BDS PRN {} (pair {}).", prn, pairKey);
            missingPRNs.insert(prn);
        }
    }

    if (!foundReceiver) {
        my_logger->error("[{}] Receiver DCB not found (pair {}).", siteKey, pairKey);
    }

    return foundReceiver;
}

bool readGALDCB(const std::string& dcbFilePath,
    const std::string& stationName,
    std::vector<double>& satelliteDCB,
    double& receiverDCB,
    const std::string& E_P1,
    const std::string& E_P2,
    std::set<int>& missingPRNs,
    std::shared_ptr<spdlog::logger> my_logger)
{
    std::ifstream file(dcbFilePath);
    if (!file.is_open()) {
        my_logger->error("Failed to open DCB file: {}", dcbFilePath);
        return false;
    }

    auto to_upper = [](std::string s) {
        for (auto& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        return s;
        };


    auto normalize_code = [&](std::string code) -> std::string {
        code = to_upper(code);

        // rinex2 to rinex3
        if (code == "C1") return "C1X";
        if (code == "C6") return "C6X";
        if (code == "C5") return "C5I";
        if (code == "C7") return "C7I";
        if (code == "C8") return "C8I";
        if (code == "P5") return "C5Q";

        return code;
        };

    std::string siteKey = to_upper(stationName);
    if (siteKey.size() > 4) siteKey = siteKey.substr(0, 4);

    std::string code1 = normalize_code(E_P1);
    std::string code2 = normalize_code(E_P2);
    std::string pairKey = code1 + "  " + code2;  
    my_logger->info("[{}] Read GAL DCB using code pair: {}", siteKey, pairKey);

    satelliteDCB.assign(37, 0.0);
    receiverDCB = 0.0;

    bool foundReceiver = false;
    std::vector<bool> foundSat(37, false);

    std::string line;
    while (std::getline(file, line)) {
        if (line.length() < 91) continue;

        const bool isSatLine = (line.substr(0, 7) == " DSB  E" || line.substr(0, 7) == " DCB  E");
        const bool isRecLine = (line.substr(0, 12) == " DSB  E    E" || line.substr(0, 12) == " DCB  E    E");

        std::string pairField = line.substr(25, 13);

        if (isSatLine &&
            pairField.find(pairKey) != std::string::npos &&
            line.substr(15, 4) == "    ")
        {
            int prn = 0;
            try { prn = std::stoi(line.substr(12, 2)); }
            catch (...) { continue; }

            double dcbValue = 0.0;
            try { dcbValue = std::stod(line.substr(70, 22)); }
            catch (...) { continue; }

            if (prn >= 1 && prn <= 36) {
                satelliteDCB[prn] = dcbValue;
                foundSat[prn] = true;
            }
            continue;
        }
        if (isRecLine &&
            pairField.find(pairKey) != std::string::npos)
        {
            std::string site = to_upper(line.substr(15, 4));
            if (site == siteKey) {
                try {
                    receiverDCB = std::stod(line.substr(70, 22));
                    foundReceiver = true;
                }
                catch (...) {
                }
            }
            continue;
        }
    }

    file.close();

    missingPRNs.clear();
    for (int prn = 1; prn <= 36; ++prn) {
        if (!foundSat[prn]) {
            my_logger->warn("Satellite DCB not found for GAL PRN {} (pair {}).", prn, pairKey);
            missingPRNs.insert(prn);
        }
    }

    if (!foundReceiver) {
        my_logger->error("[{}] Receiver DCB not found (pair {}).", siteKey, pairKey);
    }

    return foundReceiver;
}

bool readGLODCB(const std::string& dcbFilePath,
    const std::string& stationName,
    std::vector<double>& satelliteDCB,
    double& receiverDCB,
    const std::string& R_P1,
    const std::string& R_P2,
    std::set<int>& missingPRNs,
    std::shared_ptr<spdlog::logger> my_logger)
{
    std::ifstream file(dcbFilePath);
    if (!file.is_open()) {
        my_logger->error("Failed to open DCB file: {}", dcbFilePath);
        return false;
    }

    auto to_upper = [](std::string s) {
        for (auto& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        return s;
        };

    auto normalize_code = [&](std::string code) -> std::string {
        code = to_upper(code);

        // rinex2 to rinex3
        if (code == "P1") return "C1P";
        if (code == "P2") return "C2P";
        if (code == "C1") return "C1C";
        if (code == "C2") return "C2C";

        return code; 
        };

    std::string siteKey = to_upper(stationName);
    if (siteKey.size() > 4) siteKey = siteKey.substr(0, 4);

    std::string code1 = normalize_code(R_P1);
    std::string code2 = normalize_code(R_P2);
    std::string pairKey = code1 + "  " + code2; 

    my_logger->info("[{}] Read GLO DCB using code pair: {}", siteKey, pairKey);
    satelliteDCB.assign(25, 0.0);
    receiverDCB = 0.0;

    bool foundReceiver = false;
    std::vector<bool> foundSat(25, false);

    std::string line;
    while (std::getline(file, line)) {
        if (line.length() < 91) continue;

        const bool isSatLine = (line.substr(0, 7) == " DSB  R" || line.substr(0, 7) == " DCB  R");
        const bool isRecLine = (line.substr(0, 12) == " DSB  R    R" || line.substr(0, 12) == " DCB  R    R");

        std::string pairField = line.substr(25, 13);

        if (isSatLine &&
            pairField.find(pairKey) != std::string::npos &&
            line.substr(15, 4) == "    ")
        {
            int prn = 0;
            try { prn = std::stoi(line.substr(12, 2)); }
            catch (...) { continue; }

            double dcbValue = 0.0;
            try { dcbValue = std::stod(line.substr(70, 22)); }
            catch (...) { continue; }

            if (prn >= 1 && prn <= 24) {
                satelliteDCB[prn] = dcbValue;
                foundSat[prn] = true;
            }
            continue;
        }
        if (isRecLine &&
            pairField.find(pairKey) != std::string::npos)
        {
            std::string site = to_upper(line.substr(15, 4));
            if (site == siteKey) {
                try {
                    receiverDCB = std::stod(line.substr(70, 22));
                    foundReceiver = true;
                }
                catch (...) {
                }
            }
            continue;
        }
    }

    file.close();

    missingPRNs.clear();
    for (int prn = 1; prn <= 24; ++prn) {
        if (!foundSat[prn]) {
            my_logger->warn("Satellite DCB not found for GLO PRN {} (pair {}).", prn, pairKey);
            missingPRNs.insert(prn);
        }
    }

    if (!foundReceiver) {
        my_logger->error("[{}] Receiver DCB not found (pair {}).", siteKey, pairKey);
    }

    return foundReceiver;
}
