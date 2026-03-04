#pragma once
#include <algorithm>


#include <cmath>
#if defined(_MSC_VER)
#include <float.h>
#endif

inline bool IsFinite(double v)
{
#if defined(_MSC_VER) && !defined(__clang__)
    return _finite(v) != 0;
#else
    return std::isfinite(v);
#endif
}



struct sp3 {
    double sYear, sMonth, sDay, sHour, sMinute, sSecond;
    double X[35][3288], Y[35][3288], Z[35][3288];
    double CX[49][3288], CY[49][3288], CZ[49][3288];
    double EX[39][3288], EY[39][3288], EZ[39][3288];
    double RX[27][3288], RY[27][3288], RZ[27][3288];

    sp3(); 
};



#pragma once
#include <limits>

//1hz
struct sp3_1s
{
    int year = 0, month = 0, day = 0;  
    int hour = 0;                      
    int step_sec = 0;                  
    bool is5min = false;                

    double X[33][3600];
    double Y[33][3600];
    double Z[33][3600];

    sp3_1s() { reset(); }

    void reset()
    {
        const double NaN = std::numeric_limits<double>::quiet_NaN();
        for (int i = 0; i <= 32; ++i)
            for (int s = 0; s < 3600; ++s)
                X[i][s] = Y[i][s] = Z[i][s] = NaN;
    }

    static bool inRangePRN(int prn) { return prn >= 1 && prn <= 32; }
    static bool inRangeSec(int s) { return s >= 0 && s < 3600; }

    bool isValid(int prn, int s) const
    {
        if (!inRangePRN(prn) || !inRangeSec(s)) return false;
        return std::isfinite(X[prn][s]) && std::isfinite(Y[prn][s]) && std::isfinite(Z[prn][s]);
    }
};


bool interpolateHour1Hz_GPS_to(const sp3 SP3[3], int hour_idx, sp3_1s& out);
