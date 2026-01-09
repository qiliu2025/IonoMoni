#pragma once
#include <memory>
#include "sp3.h"
#include "obs.h"
#include <string>
#include "gcfg_ppp.h"
#include "spdlog/spdlog.h"

void G_prepro(
    obs& OBS,
    const std::string& stationName,
    const std::string& G_P1,
    const std::string& G_P2,
    const std::string& dcbFilePath,
    const std::string& txt_output_path,
    t_gcfg_ppp& gset,
    const sp3* SP3,
    int mf_type,
    std::shared_ptr<spdlog::logger> my_logger,
    gnut::t_gsetgnss& set_gnss
);


void R_prepro(
    obs& OBS,
    const std::string& stationName,
    const std::string& R_P1,                 
    const std::string& R_P2,                 
    const std::string& dcbFilePath,
    const std::string& txt_output_path,
    t_gcfg_ppp& gset,
    const sp3* SP3,
    int mf_type,
    std::shared_ptr<spdlog::logger> my_logger,
    gnut::t_gsetgnss& set_gnss                
);

void E_prepro(
    obs& OBS,
    const std::string& stationName,
    const std::string& E_P1,                
    const std::string& E_P2,                
    const std::string& dcbFilePath,
    const std::string& txt_output_path,
    t_gcfg_ppp& gset,
    const sp3* SP3,
    int mf_type,
    std::shared_ptr<spdlog::logger> my_logger,
    gnut::t_gsetgnss& set_gnss               
);

void C_prepro(
    obs& OBS,
    const std::string& stationName,
    const std::string& C_P1,                
    const std::string& C_P2,                
    const std::string& dcbFilePath,
    const std::string& txt_output_path,
    t_gcfg_ppp& gset,
    const sp3* SP3,
    int mf_type,
    std::shared_ptr<spdlog::logger> my_logger,
    gnut::t_gsetgnss& set_gnss              
);


void compute_MW_GF_wlAmb(double L1[], double L2[], double C1[], double C2[], double f1, double f2, double lambda_wl, double MW[], double GF[], double wlAmb[]);
void split_and_filter_arcs(double* MW, int sat_index, int arc_min_len, int ARCS[][3000][2], obs& OBS);
