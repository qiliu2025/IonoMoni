#include "get_dcb_path.h"
#include "gset/gsetinp.h"  
#include <iostream>
#include <algorithm>

std::string getDcbFilePath(gnut::t_gsetbase& gset)
{
    std::string dcbFilePath;
    auto inputs = dynamic_cast<gnut::t_gsetinp&>(gset).inputs_all();

    for (const auto& inp : inputs)
    {
        if (inp.first == gnut::IFMT::BIAS_INP || inp.first == gnut::IFMT::BIASINEX_INP)
        {
            dcbFilePath = inp.second;
            break;
        }
    }

    std::string prefix = "file://";
    if (dcbFilePath.compare(0, prefix.length(), prefix) == 0)
    {
        dcbFilePath = dcbFilePath.substr(prefix.length());
    }

    std::replace(dcbFilePath.begin(), dcbFilePath.end(), '\\', '/');
    return dcbFilePath;
}
