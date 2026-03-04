#pragma once

#include <vector>
#include <string>
#include <set>
#include <memory>
#include "spdlog/spdlog.h"

bool readGPSDCB(const std::string& dcbFilePath, 
    const std::string& stationName,
    std::vector<double>& satelliteDCB, double& receiverDCB,
    const std::string& G_P1,                 
    const std::string& G_P2,                 
    std::set<int>& missingPRNs,
    std::shared_ptr<spdlog::logger> my_logger);


bool readBDSDCB(const std::string& dcbFilePath,
    const std::string& stationName,
    std::vector<double>& satelliteDCB,
    double& receiverDCB,
    const std::string& C_P1,
    const std::string& C_P2,
    std::set<int>& missingPRNs,
    std::shared_ptr<spdlog::logger> my_logger);

bool readGLODCB(const std::string& dcbFilePath,
    const std::string& stationName,
    std::vector<double>& satelliteDCB,
    double& receiverDCB,
    const std::string& R_P1,
    const std::string& R_P2,
    std::set<int>& missingPRNs,
    std::shared_ptr<spdlog::logger> my_logger);

bool readGALDCB(const std::string& dcbFilePath,
    const std::string& stationName,
    std::vector<double>& satelliteDCB,
    double& receiverDCB,
    const std::string& E_P1,
    const std::string& E_P2,
    std::set<int>& missingPRNs,
    std::shared_ptr<spdlog::logger> my_logger);

