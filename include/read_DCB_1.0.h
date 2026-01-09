#pragma once

#include <string>
#include <map>

//double getDCBCorrection(
//    const std::string& satSys,           
//    int prn,                              
//    const std::string& code1,             
//    const std::string& code2,             
//    const std::map<std::string, double>& satelliteDCBMap,   
//    const std::map<std::string, double>& receiverDCBMap,   
//    const std::string& receiverName    
//);
void read_dcb_to_map(
    const std::string& dcbFile,                          
    std::map<std::string, double>& satelliteDCBMap,      
    std::map<std::string, double>& receiverDCBMap       
);
