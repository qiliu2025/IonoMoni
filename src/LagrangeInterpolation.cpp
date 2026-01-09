
#include "LagrangeInterpolation.h"

double LagrangeInterpolation(const double xk[], const double yk[], int n, double x) {
    double result = 0.0;
    for (int k = 0; k < n; ++k) {
        double term = yk[k];
        for (int i = 0; i < n; ++i) {
            if (i != k) {
                term *= (x - xk[i]) / (xk[k] - xk[i]);
            }
        }
        result += term;
    }
    return result;
}
