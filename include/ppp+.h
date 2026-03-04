#pragma once

#include <vector>
#include <string>
#include <functional>
#include "obs.h"
#include "sp3.h"

void remove_short_arcs_by_column(std::vector<std::vector<double>>& mat, int arc_min_length);

void remove_arc_edges_by_column(std::vector<std::vector<double>>& mat, int remove_n = 10);

void output_ppp_vtec_txt(
    const std::string& filepath,
    const std::string& prefix,
    int max_prn,
    const std::vector<std::vector<double>>& PPP_STEC,
    const obs& OBS,
    const sp3* SP3,
    int mf_type,
    std::function<void(const sp3*, int prn, int epoch, double& x, double& y, double& z)> get_sp3_coord_func
);

