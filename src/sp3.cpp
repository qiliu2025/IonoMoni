#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "sp3.h"
#include <algorithm>
#include <cmath>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <limits>
#include "LagrangeInterpolation.h"

sp3::sp3()
    : sYear(0), sMonth(0), sDay(0), sHour(0), sMinute(0), sSecond(0) {
    for (int i = 0; i < 35; ++i) {
        std::fill(X[i], X[i] + 3288, 0.0);
        std::fill(Y[i], Y[i] + 3288, 0.0);
        std::fill(Z[i], Z[i] + 3288, 0.0);
    }

    for (int i = 0; i < 49; ++i) {
        std::fill(CX[i], CX[i] + 3288, 0.0);
        std::fill(CY[i], CY[i] + 3288, 0.0);
        std::fill(CZ[i], CZ[i] + 3288, 0.0);
    }

    for (int i = 0; i < 39; ++i) {
        std::fill(EX[i], EX[i] + 3288, 0.0);
        std::fill(EY[i], EY[i] + 3288, 0.0);
        std::fill(EZ[i], EZ[i] + 3288, 0.0);
    }

    for (int i = 0; i < 27; ++i) {
        std::fill(RX[i], RX[i] + 3288, 0.0);
        std::fill(RY[i], RY[i] + 3288, 0.0);
        std::fill(RZ[i], RZ[i] + 3288, 0.0);
    }
}

// 1 Hz interpolation utilities
namespace {

    // Detect anchor step: 5 min or 15 min
    static inline int detect_anchor_step_sec(const sp3& D)
    {
        int firstCol = -1, secondCol = -1;
        for (int c = 1; c < 400; ++c) {
            if (D.X[1][c] != 0.0) {
                if (firstCol < 0) firstCol = c;
                else { secondCol = c; break; }
            }
        }
        if (firstCol > 0 && secondCol > 0) {
            int colStep = secondCol - firstCol;
            int secStep = colStep * 30;
            if (secStep == 300 || secStep == 900) return secStep;
        }
        return 900;
    }

    // Convert seconds-of-day to SP3 column index
    static inline int tsec_to_col_1based_ep10(int tsec_in_day)
    {
        int tsec = std::clamp(tsec_in_day, 0, 86399);
        int h = tsec / 3600;
        int m = (tsec % 3600) / 60;
        int ep = h * 12 + m / 5 + 1;
        return ep * 10;
    }

    // Determine which day in the 3-day SP3 buffer
    static inline void which_day_and_local_t(int tsec_abs, int& dayIdx, int& tloc)
    {
        if (tsec_abs < 0) { dayIdx = 0; tloc = tsec_abs + 86400; }
        else if (tsec_abs >= 86400) { dayIdx = 2; tloc = tsec_abs - 86400; }
        else { dayIdx = 1; tloc = tsec_abs; }
    }

    // Read one GPS anchor from 3-day SP3
    static inline bool read_anchor_xyz_GPS(const sp3 SP3[3], int sat, int tsec_abs,
        double& X, double& Y, double& Z)
    {
        int dayIdx, tloc;
        which_day_and_local_t(tsec_abs, dayIdx, tloc);
        const sp3& D = SP3[dayIdx];

        int col = tsec_to_col_1based_ep10(tloc);
        assert(col >= 0 && col < 3288);

        X = D.X[sat][col];
        Y = D.Y[sat][col];
        Z = D.Z[sat][col];

        return (X != 0.0 && Y != 0.0 && Z != 0.0);
    }

    // Barycentric Lagrange interpolation
    static inline double barycentric_interpolate(const double* x, const double* y, int n, double xq)
    {
        for (int i = 0; i < n; ++i) {
            if (xq == x[i]) return y[i];
        }

        double w[16];
        for (int i = 0; i < n; ++i) {
            double prod = 1.0;
            for (int j = 0; j < n; ++j) {
                if (i == j) continue;
                prod *= (x[i] - x[j]);
            }
            w[i] = 1.0 / prod;
        }

        double num = 0.0, den = 0.0;
        for (int i = 0; i < n; ++i) {
            double di = (xq - x[i]);
            double ti = w[i] / di;
            num += ti * y[i];
            den += ti;
        }
        return num / den;
    }

} // namespace

// Debug counters
static long g_cnt_fallback_head = 0;
static long g_cnt_fallback_tail = 0;
static long g_cnt_fail_even_fallback = 0;

bool interpolateHour1Hz_GPS_to(const sp3 SP3[3], int hour_idx, sp3_1s& out)
{
    if (hour_idx < 1 || hour_idx > 24)
        return false;

    out.reset();
    out.year = SP3[1].sYear;
    out.month = SP3[1].sMonth;
    out.day = SP3[1].sDay;
    out.hour = hour_idx;

    const int step_sec = detect_anchor_step_sec(SP3[1]);
    out.step_sec = step_sec;
    out.is5min = (step_sec == 300);

    if (86400 % step_sec != 0) return false;

    const int L = 4, R = 5;
    const int targetN = L + R + 1;
    const int t0 = (hour_idx - 1) * 3600;
    const int per_day = 86400 / step_sec;

    auto try_collect = [&](int prn, int s_min, int s_max,
        double xk[16], double yx[16], double yy[16], double yz[16]) -> int
        {
            int n = 0;
            for (int ss = s_min; ss <= s_max; ++ss)
            {
                const int t_anchor = ss * step_sec;
                double X, Y, Z;
                if (!read_anchor_xyz_GPS(SP3, prn, t_anchor, X, Y, Z))
                    continue;

                xk[n] = static_cast<double>(t_anchor);
                yx[n] = X;  yy[n] = Y;  yz[n] = Z;
                if (++n == targetN) break;
            }
            return n;
        };

    for (int prn = 1; prn <= 32; ++prn)
    {
        for (int s = 0; s < 3600; ++s)
        {
            const int t = t0 + s;
            const int k = t / step_sec;

            if (t % step_sec == 0)
            {
                double X0, Y0, Z0;
                if (read_anchor_xyz_GPS(SP3, prn, t, X0, Y0, Z0))
                {
                    out.X[prn][s] = X0;
                    out.Y[prn][s] = Y0;
                    out.Z[prn][s] = Z0;
                    continue;
                }
            }

            int s_min = k - L;
            int s_max = k + R;

            double xk[16], yx[16], yy[16], yz[16];
            int n = try_collect(prn, s_min, s_max, xk, yx, yy, yz);

            if (n < targetN)
            {
                if (k < L) {
                    ++g_cnt_fallback_head;
                    s_min = 0;
                    s_max = targetN - 1;
                }
                else if (k > per_day - 1 - R) {
                    ++g_cnt_fallback_tail;
                    s_min = per_day - targetN;
                    s_max = per_day - 1;
                }
                else {
                    s_min = std::max(0, k - L);
                    s_max = std::min(per_day - 1, k + R);
                }

                n = try_collect(prn, s_min, s_max, xk, yx, yy, yz);
            }

            if (n != targetN)
            {
                ++g_cnt_fail_even_fallback;
                continue;
            }

            const double tt = static_cast<double>(t);
            out.X[prn][s] = barycentric_interpolate(xk, yx, n, tt);
            out.Y[prn][s] = barycentric_interpolate(xk, yy, n, tt);
            out.Z[prn][s] = barycentric_interpolate(xk, yz, n, tt);
        }
    }

    // ==== debug export
    //{
    //    std::ostringstream tag;
    //    tag << std::setfill('0') << std::setw(4) << out.year
    //        << std::setw(2) << out.month
    //        << std::setw(2) << out.day
    //        << "_H" << std::setw(2) << out.hour;
    //    const std::string base = tag.str();
    //
    //    auto dump_matrix = [&](const char* fname, auto& A)
    //        {
    //            std::ofstream f(fname);
    //            f.setf(std::ios::fixed);
    //            f << std::setprecision(9);
    //            f << "# " << fname << " | unit=km | rows=3600(s), cols=PRN(1..32)\n";
    //            for (int s = 0; s < 3600; ++s)
    //            {
    //                for (int prn = 1; prn <= 32; ++prn)
    //                {
    //                    double v = A[prn][s];
    //                    if (!std::isfinite(v)) f << "NaN";
    //                    else                    f << v;
    //                    if (prn < 32) f << '\t';
    //                }
    //                f << '\n';
    //            }
    //        };
    //
    //    const std::string fx = "interp_X_" + base + ".txt";
    //    const std::string fy = "interp_Y_" + base + ".txt";
    //    const std::string fz = "interp_Z_" + base + ".txt";
    //    dump_matrix(fx.c_str(), out.X);
    //    dump_matrix(fy.c_str(), out.Y);
    //    dump_matrix(fz.c_str(), out.Z);
    //
    //    const std::string fs = "interp_SUMMARY_" + base + ".txt";
    //    std::ofstream s(fs);
    //    s << "# Interp summary for " << base << "\n";
    //    s << "step_sec = " << step_sec << "\n";
    //    s << "fallback_head = " << g_cnt_fallback_head << "\n";
    //    s << "fallback_tail = " << g_cnt_fallback_tail << "\n";
    //    s << "fail_even_fallback = " << g_cnt_fail_even_fallback << "\n";
    //    {
    //        const int prn = 1;
    //        const int col = 10;
    //        double sp3_x = SP3[1].X[prn][col];
    //        double out_x = out.X[prn][0];
    //        if (std::isfinite(sp3_x) && std::isfinite(out_x)) {
    //            s << std::setprecision(12)
    //                << "check_G01_t0_SP3X=" << sp3_x
    //                << "  interpX=" << out_x
    //                << "  diff=" << std::abs(sp3_x - out_x) << "\n";
    //        }
    //    }
    //}
    // ==== debug export end ====

    return true;
}