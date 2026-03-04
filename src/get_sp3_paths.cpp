#include "get_sp3_paths.h"
#include "gutils/gtime.h"       
#include "gset/gsetinp.h"         
#include "sp3.h"                 
#include "read_Time_sp3.h"       

#include <algorithm>
#include <fstream>
#include <iostream>

std::vector<std::string> getSortedSp3Paths(gnut::t_gsetbase& gset)
{
    std::vector<std::string> filenamesSp3;
    auto inputs = dynamic_cast<gnut::t_gsetinp&>(const_cast<gnut::t_gsetbase&>(gset)).inputs_all();


    for (const auto& inp : inputs)
    {
        if (inp.first == gnut::IFMT::SP3_3DAY_INP)
        {
            std::string sp3file = inp.second;
            std::string prefix = "file://";
            if (sp3file.compare(0, prefix.length(), prefix) == 0)
                sp3file = sp3file.substr(prefix.length());
            std::replace(sp3file.begin(), sp3file.end(), '\\', '/');
            filenamesSp3.push_back(sp3file);
        }
    }

    std::vector<std::pair<std::string, gnut::t_gtime>> sp3WithTime;

    for (const auto& f : filenamesSp3)
    {
        sp3 s;
        std::ifstream file(f);
        read_Time_sp3(file, f, s);

        gnut::t_gtime t;
        t.from_ymdhms((int)s.sYear, (int)s.sMonth, (int)s.sDay,
            (int)s.sHour, (int)s.sMinute, s.sSecond);

        sp3WithTime.emplace_back(f, t);
    }

    std::sort(sp3WithTime.begin(), sp3WithTime.end(),
        [](const auto& a, const auto& b) {
            return a.second < b.second;
        });

    filenamesSp3.clear();
    for (const auto& p : sp3WithTime)
        filenamesSp3.push_back(p.first);

    return filenamesSp3;
}
