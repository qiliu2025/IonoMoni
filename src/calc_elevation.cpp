#include "calc_elevation.h"
#include <cmath>
#include "ecef2elli.h"

void calc_elevation(double x, double y, double z, double sx, double sy, double sz, double& E, double& A) {

    if (x == 0.0 && y == 0.0 && z == 0.0) {
        E = -90.0;  // Not visible
        A = 0.0;
        return;
    }

    double sb, sl;
    ecef2elli(sx, sy, sz, sb, sl);  

    // Coordinate transformation matrix (ECEF to local NEU)
    double T[3][3] = {
        {-sin(sb) * cos(sl), -sin(sb) * sin(sl), cos(sb)},
        {-sin(sl),            cos(sl),           0      },
        { cos(sb) * cos(sl),  cos(sb) * sin(sl), sin(sb)}
    };

    double delta_xyz[3] = { x - sx, y - sy, z - sz };
    double NEU[3];

    // Transform to local NEU coordinates
    for (int i = 0; i < 3; ++i) {
        NEU[i] = T[i][0] * delta_xyz[0] + T[i][1] * delta_xyz[1] + T[i][2] * delta_xyz[2];
    }

    // Calculate elevation angle (degrees)
    E = atan2(NEU[2], sqrt(NEU[0] * NEU[0] + NEU[1] * NEU[1])) * RAD_TO_DEG;

    // Calculate azimuth angle (degrees)
    double az = atan2(NEU[1], NEU[0]);
    if (az < 0) az += 2 * PI;
    A = az * RAD_TO_DEG;
}
