#pragma once

#include <string>
#include <vector>
#include <memory>

#include "obs.h"
#include "sp3.h"
#include "gcfg_ppp.h"
#include "spdlog/spdlog.h"

namespace fs = std::filesystem;
using std::string;
using std::vector;
using std::shared_ptr;

extern double h;  

void run_ccl_stec(t_gcfg_ppp& gset, shared_ptr<spdlog::logger> my_logger, const string& output_dir);

void run_ppp_stec(t_gcfg_ppp& gset, shared_ptr<spdlog::logger> my_logger);

void run_roti(t_gcfg_ppp& gset, shared_ptr<spdlog::logger> my_logger);

void run_aatr(t_gcfg_ppp& gset, shared_ptr<spdlog::logger> my_logger);

void apply_elevation_mask(obs& OBS, const sp3& SP3_data, double elevCutoffDeg, int maxSat, char sys);


