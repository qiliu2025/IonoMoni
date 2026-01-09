#pragma once
#include <vector>
#include <string>

struct IPP_Point {
    double lat;
    double lon;
};

void calc_ipp_matrix(
    double station_X, double station_Y, double station_Z,
    const double X[35][3288],
    const double Y[35][3288],
    const double Z[35][3288],
    int maxSat,
    int maxEpoch,
    const std::string& systemName,
    const std::string& txt_output_path 
);


