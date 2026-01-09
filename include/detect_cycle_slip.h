#pragma once
#pragma once

#include "obs.h"

void detect_cycle_slip(double wlAmb[], double GF[], obs& OBS, int sat_index,
    int ARCS[][3000][2], int arc_min_len, double cs_epoch[][2881]);
