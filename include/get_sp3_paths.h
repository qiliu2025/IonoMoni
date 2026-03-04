#pragma once
#ifndef GET_SP3_PATHS_H
#define GET_SP3_PATHS_H

#include <vector>
#include <string>
#include "gset/gsetbase.h"
#include "gutils/gtime.h"   
#include "gset/gsetinp.h"   


std::vector<std::string> getSortedSp3Paths(gnut::t_gsetbase& gset);
#endif 
