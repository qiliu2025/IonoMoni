#include "mapping_function.h"
#include "constants.h"
#include <cmath>


double get_mapping_function(double e, int mf_type)
{
    double z = PI / 2.0 - e; 
    switch (mf_type) {

    case 0:  // SLM (Single-Layer Model)
    {
        double alpha = 1.0;
        return 1.0 / sqrt(1.0 - pow(Re * sin(alpha * z) / (Re + h), 2));
    }
    case 1:// MSLM (Modified Single-Layer Model)
    {
        double alpha = 0.9782;
        return 1.0 / sqrt(1.0 - pow(Re * sin(alpha * z) / (Re + h), 2));
    }
    case 2: // F&K mapping function
    {
        double a = 1 + (Re + h) / Re;
        double b = sin(e) + sqrt(pow((Re + h) / Re, 2) - pow(cos(e), 2));
        return a / b;
    }
    case 3: // Ou Jikun mapping function
    {
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
        double h1 = h - 35000.0;
        double h2 = h + 70000.0;
        double t1 = pow(Re * sin(e), 2.0) + 2 * Re * h2 + h2 * h2;
        double t2 = pow(Re * sin(e), 2.0) + 2 * Re * h1 + h1 * h1;
        return (sqrt(t1) - sqrt(t2)) / (h2 - h1);
    }

    case 5: // Klobuchar mapping function
    {
        double e_deg = e * 180.0 / PI;     // rad -> deg
        double x = 0.53 - e_deg / 180.0;   // deg -> semicircle

        double mf = 1.0 + 16.0 * pow(x, 3);
        return mf;
    }
    default: // Default to SLM
    {
        double z_pie = asin(Re * sin(z) / (Re + h));
        return 1.0 / cos(z_pie);
    }
    }
}
