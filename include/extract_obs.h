#pragma once

#include <vector>
#include <string>
#include "gall/gallobs.h"
#include "gset/gsetbase.h"
#include "gset/gsetgen.h"
#include "gutils/gobs.h"
#include "gutils/gtime.h"
#include <memory>          
#include <spdlog/logger.h> 
#include "obs.h"
namespace gnut {

  
    template<typename T>
    void write_matrix_to_txt_raw(const T& filename, const std::vector<std::vector<double>>& matrix)
    {
        std::ofstream fout(filename);
        if (!fout.is_open())
        {
            std::cerr << "Cannot open output file: " << filename << std::endl;
            return;
        }

        for (size_t i = 1; i < matrix.size(); ++i)
        {
            for (size_t j = 1; j < matrix[i].size(); ++j)
            {
                fout << std::fixed << std::setprecision(3) << matrix[i][j];
                if (j < matrix[i].size() - 1)
                    fout << "\t";
            }
            fout << "\n";
        }

        fout.close();
    }

    // GPS


    void extract_GPS_obs(
        t_gallobs* gobs,
        const std::string& station,
        std::vector<std::vector<double>>& C1,
        std::vector<std::vector<double>>& C2,
        std::vector<std::vector<double>>& L1,
        std::vector<std::vector<double>>& L2,
        std::vector<t_gtime>& epochs,
        std::vector<std::string>& sats,
        std::string& G_P1,
        std::string& G_P2,
        std::shared_ptr<spdlog::logger> logger,
        obs& OBS,
        t_gsetgnss& set_gnss
    );



    void extract_GPS_SNR(t_gallobs* gobs, const std::string& station,
        std::vector<std::vector<double>>& GPS_S1,
        std::vector<std::vector<double>>& GPS_S2,
        std::vector<t_gtime>& epochs,
        std::vector<std::string>& GPS_sats,
        std::shared_ptr<spdlog::logger> logger, 
        obs& OBS);



    // BDS
    void extract_BDS_obs(
        t_gallobs* gobs,
        const std::string& station,
        std::vector<std::vector<double>>& BDS_C1,  
        std::vector<std::vector<double>>& BDS_C2,  
        std::vector<std::vector<double>>& BDS_L1,  
        std::vector<std::vector<double>>& BDS_L2,  
        std::vector<t_gtime>& epochs,
        std::vector<std::string>& BDS_sats,
        std::string& C_P1,                         
        std::string& C_P2,                         
        std::shared_ptr<spdlog::logger> logger,
        obs& OBS,
        gnut::t_gsetgnss& set_gnss);

    // GAL
    void extract_GAL_obs(
        t_gallobs* gobs,
        const std::string& station,
        std::vector<std::vector<double>>& GAL_C1,   
        std::vector<std::vector<double>>& GAL_C2,   
        std::vector<std::vector<double>>& GAL_L1,   
        std::vector<std::vector<double>>& GAL_L2,   
        std::vector<t_gtime>& epochs,
        std::vector<std::string>& GAL_sats,
        std::string& E_P1,                          
        std::string& E_P2,                          
        std::shared_ptr<spdlog::logger> logger,
        obs& OBS,
        gnut::t_gsetgnss& set_gnss                  
    );
    //GLO
    void extract_GLO_obs(
        t_gallobs* gobs, const std::string& station,
        std::vector<std::vector<double>>& GLO_C1,
        std::vector<std::vector<double>>& GLO_C2,
        std::vector<std::vector<double>>& GLO_L1,
        std::vector<std::vector<double>>& GLO_L2,
        std::vector<t_gtime>& epochs,
        std::vector<std::string>& GLO_sats,
        std::string& R_P1, std::string& R_P2,                
        std::shared_ptr<spdlog::logger> logger,
        obs& OBS,
        gnut::t_gsetgnss& set_gnss
    );             
        


} // namespace gnut
