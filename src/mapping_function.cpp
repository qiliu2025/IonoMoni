#include "mapping_function.h"
#include "constants.h"
#include <cmath>

// Constant definition
const double rad2semi = 1.0 / PI * 180.0 / 0.5; 

double get_mapping_function(double e, int mf_type)
{
    switch (mf_type) {
    case 0: // SLM (Single-Layer Model)
    {
        double z = PI / 2 - e;
        double z_pie = asin(Re * sin(z) / (Re + h));
        return 1.0 / cos(z_pie);
    }
    case 1: // MSLM (Modified Single-Layer Model)
    {
        double z = PI / 2 - e;
        double alpha = 0.9782;
        return 1.0 / sqrt(1 - pow(Re * sin(alpha * z) / (Re + h), 2));
    }
    // case 2: // KLOBUCHAR
    // {
    //     double e_deg = e * 180.0 / PI;
    //     double x = 0.53 - e_deg / 90.0;
    //
    //     double mf = 1.0 + 16.0 * pow(x, 3);
    //
    //     if (mf > 10.0) mf = 10.0;         // Maximum limit: not more than 10
    //     if (mf < 1.0) mf = 1.0;           // Minimum limit: not less than 1 (theoretical lower bound)
    //
    //     return mf;
    // }

    case 2: // F-K mapping function
    {
        double a = 1 + (Re + h) / Re;
        double b = sin(e) + sqrt(pow((Re + h) / Re, 2) - pow(cos(e), 2));
        return a / b;
    }
    case 3: // Ou Jikun et al. mapping function
    {
        double z = PI / 2 - e;
        double z_pie = asin(Re * sin(z) / (Re + h));
        double mf_temp = 1.0 / cos(z_pie);
        double deg2rad = PI / 180.0;
        if (e < 40 * deg2rad) {
            return sin(e + 50 * deg2rad) * mf_temp;
        }
        else {
            return mf_temp;
        }
    }
    case 4: // Fanselow mapping function
    {
        int h1 = h - 35000; 
        int h2 = h + 70000; 
        double t1 = pow(Re * sin(e), 2.0) + 2 * Re * h2 + h2 * h2;
        double t2 = pow(Re * sin(e), 2.0) + 2 * Re * h1 + h1 * h1;
        return (sqrt(t1) - sqrt(t2)) / (h2 - h1);
    }
    default: // Default to SLM
    {
        double z = PI / 2 - e;
        double z_pie = asin(Re * sin(z) / (Re + h));
        return 1.0 / cos(z_pie);
    }
    }
}
