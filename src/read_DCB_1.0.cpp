#include "read_DCB_1.0.h"
#include <fstream>
#include <regex>
#include <iostream>

void read_dcb_to_map(
    const std::string& dcbFile,
    std::map<std::string, double>& satelliteDCBMap,
    std::map<std::string, double>& receiverDCBMap)
{
    std::ifstream fin(dcbFile);
    if (!fin.is_open()) {
        std::cerr << "Failed to open DCB file: " << dcbFile << std::endl;
        return;
    }

    std::string line;
    std::regex sat_pattern(R"((?:DSB|DCB)\s+\S+\s+([GREJC]\d{2})\s+([A-Z0-9]{3})\s+([A-Z0-9]{3})\s+[\d: ]+\s+[\d: ]+\s+\w+\s+([-\d.E+]+))");
    std::regex rec_pattern(
        R"((?:DSB|DCB)\s+([A-Z])\s+[A-Z]\s+([A-Z0-9]{4})(?:\s+[A-Z0-9]{9})?\s+([A-Z0-9]{3})\s+([A-Z0-9]{3})\s+[\d: ]+\s+[\d: ]+\s+\w+\s+([-\d.E+]+)\s+([-\d.E+]+))"
    );

    std::smatch match;
    while (std::getline(fin, line)) {
        // Parse satellite DCB entry
        if (std::regex_search(line, match, sat_pattern)) {
            std::string key = match[1].str() + "_" + match[2].str() + match[3].str(); // G01_C1CC1W
            double value = std::stod(match[4].str());
            satelliteDCBMap[key] = value;
            //std::cout << "[satelliteDCBMap] key = " << key << " value = " << value << " ns" << std::endl;
            continue;
        }

        // Parse receiver DCB entry
        if (std::regex_search(line, match, rec_pattern)) {
            std::string key = match[1].str() + "_" + match[2].str() + "_" + match[3].str() + match[4].str(); // G_BJFS_C1CC1W
            double value = std::stod(match[5].str());
            receiverDCBMap[key] = value;
            //std::cout << "[receiverDCBMap] key = " << key << " value = " << value << " ns" << std::endl;
            continue;
        }
    }

    fin.close();
}

