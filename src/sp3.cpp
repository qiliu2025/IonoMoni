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

// ！ Internal utilities in anonymous namespace ！ 1 Hz interpolation
namespace {

    // Detect anchor cadence: 5 min (300 s) or 15 min (900 s)
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
            int colStep = secondCol - firstCol; // 30 s per column
            int secStep = colStep * 30;
            if (secStep == 300 || secStep == 900) return secStep;
        }
        return 900;
    }

    // Map "seconds-of-day" to the current 30 s grid column index
    // ep = h*12 + m/5 + 1; col = ep*10
    static inline int tsec_to_col_1based_ep10(int tsec_in_day)
    {
        int tsec = std::clamp(tsec_in_day, 0, 86399);
        int h = tsec / 3600;
        int m = (tsec % 3600) / 60;
        int ep = h * 12 + m / 5 + 1;     // 1-based
        return ep * 10;                  // column index (5 min corresponds to 10-column step)
    }

    // Given absolute seconds-of-day (relative to SP3[1] 00:00, can be negative or −86400), decide which day it belongs to
    // dayIdx （ {0,1,2} -> SP3[0]/SP3[1]/SP3[2]
    static inline void which_day_and_local_t(int tsec_abs, int& dayIdx, int& tloc)
    {
        if (tsec_abs < 0) { dayIdx = 0; tloc = tsec_abs + 86400; }
        else if (tsec_abs >= 86400) { dayIdx = 2; tloc = tsec_abs - 86400; }
        else { dayIdx = 1; tloc = tsec_abs; }
    }

    // Read one anchor from three-day SP3 (GPS)
    // return true = success (non-zero)
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

    // Barycentric Lagrange interpolation: return y_i if query hits a node
    static inline double barycentric_interpolate(const double* x, const double* y, int n, double xq)
    {
        // Return directly when hitting a node
        for (int i = 0; i < n; ++i) {
            if (xq == x[i]) return y[i];
        }
        // Compute weights w_i = 1 / Π_{j!=i} (x_i - x_j)
        double w[16]; // n<=10 is sufficient
        for (int i = 0; i < n; ++i) {
            double prod = 1.0;
            for (int j = 0; j < n; ++j) {
                if (i == j) continue;
                prod *= (x[i] - x[j]);
            }
            w[i] = 1.0 / prod;
        }
        // Evaluate f(xq) = (Σ w_i y_i / (xq - x_i)) / (Σ w_i / (xq - x_i))
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

//// ！ Statistics counters: for debug output ！
static long g_cnt_fallback_head = 0;        // number of fallbacks at start of day
static long g_cnt_fallback_tail = 0;        // number of fallbacks at end of day
static long g_cnt_fail_even_fallback = 0;   // number of failures even after fallback

bool interpolateHour1Hz_GPS_to(const sp3 SP3[3], int hour_idx, sp3_1s& out)
{
    if (hour_idx < 1 || hour_idx > 24)
        return false;

    out.reset();
    out.year = SP3[1].sYear;
    out.month = SP3[1].sMonth;
    out.day = SP3[1].sDay;
    out.hour = hour_idx;

    const int step_sec = detect_anchor_step_sec(SP3[1]);  // 300 or 900
    out.step_sec = step_sec;
    out.is5min = (step_sec == 300);

    // step_sec must divide 86400
    if (86400 % step_sec != 0) return false;

    // ！ Tunable window ！
    const int L = 4, R = 5;                 // total 10 anchors (asymmetric window [-4,+5])
    const int targetN = L + R + 1;          // 10
    const int t0 = (hour_idx - 1) * 3600;   // start second within the day for this hour
    const int per_day = 86400 / step_sec;   // anchors per day (5 min = 300 => 288)

    // Helper to collect anchors: try to gather targetN usable anchors within the specified window
    auto try_collect = [&](int prn, int s_min, int s_max,
        double xk[16], double yx[16], double yy[16], double yz[16]) -> int
        {
            int n = 0;
            for (int ss = s_min; ss <= s_max; ++ss)
            {
                const int t_anchor = ss * step_sec;   // allow cross-day
                double X, Y, Z;
                if (!read_anchor_xyz_GPS(SP3, prn, t_anchor, X, Y, Z))
                    continue;   // skip missing anchor

                xk[n] = static_cast<double>(t_anchor);
                yx[n] = X;  yy[n] = Y;  yz[n] = Z;
                if (++n == targetN) break;
            }
            return n;
        };

    // Interpolate per satellite and per second (GPS 1..32)
    for (int prn = 1; prn <= 32; ++prn)
    {
        for (int s = 0; s < 3600; ++s)
        {
            const int t = t0 + s;          // seconds within the day
            const int k = t / step_sec;    // anchor segment index: floor(t/step)

            // If exactly on an anchor, copy directly to keep perfect alignment
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
                // If reading the anchor fails, continue with interpolation fallback
            }

            // First try window [-L, +R]
            int s_min = k - L;
            int s_max = k + R;

            double xk[16], yx[16], yy[16], yz[16];
            int n = try_collect(prn, s_min, s_max, xk, yx, yy, yz);

            // ！ If fewer than targetN anchors, do "single-day fallback" ！
            if (n < targetN)
            {
                // Start of day: use [0..targetN-1]; End of day: use [per_day-targetN..per_day-1]
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
                    // Middle of day but cross-day anchors missing: collect only within the current day
                    s_min = std::max(0, k - L);
                    s_max = std::min(per_day - 1, k + R);
                }

                n = try_collect(prn, s_min, s_max, xk, yx, yy, yz);
            }

            if (n != targetN)
            {
                // Still not enough: keep NaN (out.reset already set NaN)
                ++g_cnt_fail_even_fallback;
                continue;
            }

            const double tt = static_cast<double>(t);
            // Use barycentric Lagrange interpolation
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
    //    // Additional summary file: record fallback/failure stats + first-epoch alignment self-check
    //    const std::string fs = "interp_SUMMARY_" + base + ".txt";
    //    std::ofstream s(fs);
    //    s << "# Interp summary for " << base << "\n";
    //    s << "step_sec = " << step_sec << "\n";
    //    s << "fallback_head = " << g_cnt_fallback_head << "\n";
    //    s << "fallback_tail = " << g_cnt_fallback_tail << "\n";
    //    s << "fail_even_fallback = " << g_cnt_fail_even_fallback << "\n";
    //    // Self-check: G01 00:00
    //    {
    //        const int prn = 1; // G01
    //        const int col = 10; // 5 min anchor column of 00:00
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
