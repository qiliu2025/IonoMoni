#pragma once

#include "gall/gallobs.h"
#include "src/libGREAT/gcfg_ppp.h"
#include "spdlog/spdlog.h"

namespace gnut {

    t_gallobs* decode_obs(t_gcfg_ppp& gset, std::shared_ptr<spdlog::logger> my_logger); 

}
