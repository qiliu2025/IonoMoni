#include "sp3.h"
#include "LagrangeInterpolation.h"  
#include <cstring>

void mergeSP3Data(sp3 SP3[3]) {
    if (SP3[1].X[1][20] != 0) {
        for (int i = 2880; i >= 10; i -= 10) {
            for (int j = 1; j <= 32; j++) {
                SP3[1].X[j][i + 40] = SP3[1].X[j][i];
                SP3[1].Y[j][i + 40] = SP3[1].Y[j][i];
                SP3[1].Z[j][i + 40] = SP3[1].Z[j][i];
            }
        }
        for (int i = 2880; i >= 10; i -= 10) {
            for (int j = 1; j <= 46; j++) {
                SP3[1].CX[j][i + 40] = SP3[1].CX[j][i];
                SP3[1].CY[j][i + 40] = SP3[1].CY[j][i];
                SP3[1].CZ[j][i + 40] = SP3[1].CZ[j][i];
            }
        }
        for (int i = 2880; i >= 10; i -= 10) {
            for (int j = 1; j <= 36; j++) {
                SP3[1].EX[j][i + 40] = SP3[1].EX[j][i];
                SP3[1].EY[j][i + 40] = SP3[1].EY[j][i];
                SP3[1].EZ[j][i + 40] = SP3[1].EZ[j][i];
            }
        }
        for (int i = 2880; i >= 10; i -= 10) {
            for (int j = 1; j <= 24; j++) {
                SP3[1].RX[j][i + 40] = SP3[1].RX[j][i];
                SP3[1].RY[j][i + 40] = SP3[1].RY[j][i];
                SP3[1].RZ[j][i + 40] = SP3[1].RZ[j][i];
            }
        }

        //============================================================
        for (int i = 0; i < 4; i++) {
            for (int j = 1; j <= 32; j++) {
                SP3[1].X[j][10 + i * 10] = SP3[0].X[j][2850 + i * 10];
                SP3[1].Y[j][10 + i * 10] = SP3[0].Y[j][2850 + i * 10];
                SP3[1].Z[j][10 + i * 10] = SP3[0].Z[j][2850 + i * 10];
            }

        }
        for (int i = 0; i < 4; i++) {
            for (int j = 1; j <= 46; j++) {
                SP3[1].CX[j][10 + i * 10] = SP3[0].CX[j][2850 + i * 10];
                SP3[1].CY[j][10 + i * 10] = SP3[0].CY[j][2850 + i * 10];
                SP3[1].CZ[j][10 + i * 10] = SP3[0].CZ[j][2850 + i * 10];
            }

        }
        for (int i = 0; i < 4; i++) {
            for (int j = 1; j <= 36; j++) {
                SP3[1].EX[j][10 + i * 10] = SP3[0].EX[j][2850 + i * 10];
                SP3[1].EY[j][10 + i * 10] = SP3[0].EY[j][2850 + i * 10];
                SP3[1].EZ[j][10 + i * 10] = SP3[0].EZ[j][2850 + i * 10];
            }

        }
        for (int i = 0; i < 4; i++) {
            for (int j = 1; j <= 24; j++) {
                SP3[1].RX[j][10 + i * 10] = SP3[0].RX[j][2850 + i * 10];
                SP3[1].RY[j][10 + i * 10] = SP3[0].RY[j][2850 + i * 10];
                SP3[1].RZ[j][10 + i * 10] = SP3[0].RZ[j][2850 + i * 10];
            }

        }
        //============================================================
        for (int i = 0; i < 5; i++) {
            for (int j = 1; j <= 32; j++) {
                SP3[1].X[j][2930 + i * 10] = SP3[2].X[j][10 + i * 10];
                SP3[1].Y[j][2930 + i * 10] = SP3[2].Y[j][10 + i * 10];
                SP3[1].Z[j][2930 + i * 10] = SP3[2].Z[j][10 + i * 10];
            }

        }
        for (int i = 0; i < 5; i++) {
            for (int j = 1; j <= 46; j++) {
                SP3[1].CX[j][2930 + i * 10] = SP3[2].CX[j][10 + i * 10];
                SP3[1].CY[j][2930 + i * 10] = SP3[2].CY[j][10 + i * 10];
                SP3[1].CZ[j][2930 + i * 10] = SP3[2].CZ[j][10 + i * 10];
            }

        }
        for (int i = 0; i < 5; i++) {
            for (int j = 1; j <= 36; j++) {
                SP3[1].EX[j][2930 + i * 10] = SP3[2].EX[j][10 + i * 10];
                SP3[1].EY[j][2930 + i * 10] = SP3[2].EY[j][10 + i * 10];
                SP3[1].EZ[j][2930 + i * 10] = SP3[2].EZ[j][10 + i * 10];
            }

        }
        for (int i = 0; i < 5; i++) {
            for (int j = 1; j <= 24; j++) {
                SP3[1].RX[j][2930 + i * 10] = SP3[2].RX[j][10 + i * 10];
                SP3[1].RY[j][2930 + i * 10] = SP3[2].RY[j][10 + i * 10];
                SP3[1].RZ[j][2930 + i * 10] = SP3[2].RZ[j][10 + i * 10];
            }

        }
    }
    else {
        // Move only SP3[1] data, shifting by 20 positions backwards
        int k = 1;

        for (int i = 2880; i >= 10; i -= 10) {
            for (int j = 1; j <= 32; j++) {
                SP3[k].X[j][i + 20] = SP3[k].X[j][i];
                SP3[k].Y[j][i + 20] = SP3[k].Y[j][i];
                SP3[k].Z[j][i + 20] = SP3[k].Z[j][i];
            }
        }

        for (int i = 2880; i >= 10; i -= 10) {
            for (int j = 1; j <= 46; j++) {
                SP3[k].CX[j][i + 20] = SP3[k].CX[j][i];
                SP3[k].CY[j][i + 20] = SP3[k].CY[j][i];
                SP3[k].CZ[j][i + 20] = SP3[k].CZ[j][i];
            }
        }

        for (int i = 2880; i >= 10; i -= 10) {
            for (int j = 1; j <= 36; j++) {
                SP3[k].EX[j][i + 20] = SP3[k].EX[j][i];
                SP3[k].EY[j][i + 20] = SP3[k].EY[j][i];
                SP3[k].EZ[j][i + 20] = SP3[k].EZ[j][i];
            }
        }

        for (int i = 2880; i >= 10; i -= 10) {
            for (int j = 1; j <= 24; j++) {
                SP3[k].RX[j][i + 20] = SP3[k].RX[j][i];
                SP3[k].RY[j][i + 20] = SP3[k].RY[j][i];
                SP3[k].RZ[j][i + 20] = SP3[k].RZ[j][i];
            }
        }
        // Move SP3[1] data by an additional 120 positions
        for (int i = 2900; i >= 30; i -= 10) {
            for (int j = 1; j <= 32; j++) {
                SP3[1].X[j][i + 120] = SP3[1].X[j][i];
                SP3[1].Y[j][i + 120] = SP3[1].Y[j][i];
                SP3[1].Z[j][i + 120] = SP3[1].Z[j][i];
            }
        }

        for (int i = 2900; i >= 30; i -= 10) {
            for (int j = 1; j <= 46; j++) {
                SP3[1].CX[j][i + 120] = SP3[1].CX[j][i];
                SP3[1].CY[j][i + 120] = SP3[1].CY[j][i];
                SP3[1].CZ[j][i + 120] = SP3[1].CZ[j][i];
            }
        }

        for (int i = 2900; i >= 30; i -= 10) {
            for (int j = 1; j <= 36; j++) {
                SP3[1].EX[j][i + 120] = SP3[1].EX[j][i];
                SP3[1].EY[j][i + 120] = SP3[1].EY[j][i];
                SP3[1].EZ[j][i + 120] = SP3[1].EZ[j][i];
            }
        }

        for (int i = 2900; i >= 30; i -= 10) {
            for (int j = 1; j <= 24; j++) {
                SP3[1].RX[j][i + 120] = SP3[1].RX[j][i];
                SP3[1].RY[j][i + 120] = SP3[1].RY[j][i];
                SP3[1].RZ[j][i + 120] = SP3[1].RZ[j][i];
            }
        }

        // Fill data from SP3[0] into SP3[1] at indices 0, 30, 60, 90
        for (int m = 0; m < 4; m++) {
            int target_i = m * 30;
            int source_i = 2770 + m * 30;


            for (int j = 1; j <= 32; j++) {
                SP3[1].X[j][target_i] = SP3[0].X[j][source_i];
                SP3[1].Y[j][target_i] = SP3[0].Y[j][source_i];
                SP3[1].Z[j][target_i] = SP3[0].Z[j][source_i];
            }

            for (int j = 1; j <= 46; j++) {
                SP3[1].CX[j][target_i] = SP3[0].CX[j][source_i];
                SP3[1].CY[j][target_i] = SP3[0].CY[j][source_i];
                SP3[1].CZ[j][target_i] = SP3[0].CZ[j][source_i];
            }

            for (int j = 1; j <= 36; j++) {
                SP3[1].EX[j][target_i] = SP3[0].EX[j][source_i];
                SP3[1].EY[j][target_i] = SP3[0].EY[j][source_i];
                SP3[1].EZ[j][target_i] = SP3[0].EZ[j][source_i];
            }

            for (int j = 1; j <= 24; j++) {
                SP3[1].RX[j][target_i] = SP3[0].RX[j][source_i];
                SP3[1].RY[j][target_i] = SP3[0].RY[j][source_i];
                SP3[1].RZ[j][target_i] = SP3[0].RZ[j][source_i];
            }
        }

        // Fill data from SP3[2] into SP3[1] at indices 2820, 2850, 2880, 2910, and 2940
        for (int m = 0; m < 5; m++) {
            int target_i = 3030 + m * 30;
            int source_i = 10 + m * 30;

            for (int j = 1; j <= 32; j++) {
                SP3[1].X[j][target_i] = SP3[2].X[j][source_i];
                SP3[1].Y[j][target_i] = SP3[2].Y[j][source_i];
                SP3[1].Z[j][target_i] = SP3[2].Z[j][source_i];
            }

            for (int j = 1; j <= 46; j++) {
                SP3[1].CX[j][target_i] = SP3[2].CX[j][source_i];
                SP3[1].CY[j][target_i] = SP3[2].CY[j][source_i];
                SP3[1].CZ[j][target_i] = SP3[2].CZ[j][source_i];
            }

            for (int j = 1; j <= 36; j++) {
                SP3[1].EX[j][target_i] = SP3[2].EX[j][source_i];
                SP3[1].EY[j][target_i] = SP3[2].EY[j][source_i];
                SP3[1].EZ[j][target_i] = SP3[2].EZ[j][source_i];
            }

            for (int j = 1; j <= 24; j++) {
                SP3[1].RX[j][target_i] = SP3[2].RX[j][source_i];
                SP3[1].RY[j][target_i] = SP3[2].RY[j][source_i];
                SP3[1].RZ[j][target_i] = SP3[2].RZ[j][source_i];
            }
        }
    }
}

// ========== Interpolation Completion for Each GNSS System ==========

// Fill data
void G_fillArray(double A[35][3288]) {
    if (A[1][20] != 0) {
        for (int i = 1; i <= 32; ++i) {
            for (int j = 51; j <= 2929; ++j) {
                int k = j / 10;
                int s_min = k - 4;
                int s_max = k + 5;

                if (s_min < 1) {
                    s_min = 1;
                    s_max = s_min + 9;
                }
                if (s_max > 297) {
                    s_max = 297;
                    s_min = s_max - 9;
                }

                double xk[10], yk[10];
                int n = 0;
                for (int s = s_min; s <= s_max; ++s) {
                    int x_s = 10 * s;
                    xk[n] = static_cast<double>(x_s);
                    yk[n] = A[i][x_s];
                    n++;
                }
                A[i][j] = LagrangeInterpolation(xk, yk, n, static_cast<double>(j));
            }
        }
    }
    else {
        for (int i = 1; i <= 32; ++i) {
            for (int j = 91; j <= 3029; ++j) {  
                int k = j / 30; 
                int s_min = k - 4;
                int s_max = k + 5;

                // Boundary protection
                if (s_min < 0) {
                    s_min = 0;
                    s_max = s_min + 9; 
                }
                if (s_max > 105) { 
                    s_max = 105;
                    s_min = s_max - 9;
                }


                double xk[10], yk[10];
                int n = 0;
                for (int s = s_min; s <= s_max; ++s) {
                    int x_s = 30 * s; 
                    xk[n] = static_cast<double>(x_s);
                    yk[n] = A[i][x_s];
                    n++;
                }

                A[i][j] = LagrangeInterpolation(xk, yk, n, static_cast<double>(j));
            }
        }
    }
}
void C_fillArray(double A[49][3288]) {
    if (A[1][20] != 0) {
        for (int i = 1; i <= 46; ++i) {
            for (int j = 51; j <= 2929; ++j) {
                int k = j / 10;
                int s_min = k - 4;
                int s_max = k + 5;
                if (s_min < 1) { s_min = 1; s_max = s_min + 9; }
                if (s_max > 297) { s_max = 297; s_min = s_max - 9; }

                double xk[10], yk[10];
                int n = 0;
                for (int s = s_min; s <= s_max; ++s) {
                    int x_s = 10 * s;
                    xk[n] = static_cast<double>(x_s);
                    yk[n] = A[i][x_s];
                    n++;
                }
                A[i][j] = LagrangeInterpolation(xk, yk, n, static_cast<double>(j));
            }
        }
    }
    else {
        for (int i = 1; i <= 46; ++i) {
            for (int j = 91; j <= 3029; ++j) {
                int k = j / 30;
                int s_min = k - 4;
                int s_max = k + 5;
                if (s_min < 0) { s_min = 0; s_max = s_min + 9; }
                if (s_max > 105) { s_max = 105; s_min = s_max - 9; }

                double xk[10], yk[10];
                int n = 0;
                for (int s = s_min; s <= s_max; ++s) {
                    int x_s = 30 * s;
                    xk[n] = static_cast<double>(x_s);
                    yk[n] = A[i][x_s];
                    n++;
                }
                A[i][j] = LagrangeInterpolation(xk, yk, n, static_cast<double>(j));
            }
        }
    }
}

void E_fillArray(double A[39][3288]) {
    if (A[1][20] != 0) {
        for (int i = 1; i <= 36; ++i) {
            for (int j = 51; j <= 2929; ++j) {
                int k = j / 10;
                int s_min = k - 4;
                int s_max = k + 5;
                if (s_min < 1) { s_min = 1; s_max = s_min + 9; }
                if (s_max > 297) { s_max = 297; s_min = s_max - 9; }

                double xk[10], yk[10];
                int n = 0;
                for (int s = s_min; s <= s_max; ++s) {
                    int x_s = 10 * s;
                    xk[n] = static_cast<double>(x_s);
                    yk[n] = A[i][x_s];
                    n++;
                }
                A[i][j] = LagrangeInterpolation(xk, yk, n, static_cast<double>(j));
            }
        }
    }
    else {
        for (int i = 1; i <= 36; ++i) {
            for (int j = 91; j <= 3029; ++j) {
                int k = j / 30;
                int s_min = k - 4;
                int s_max = k + 5;
                if (s_min < 0) { s_min = 0; s_max = s_min + 9; }
                if (s_max > 105) { s_max = 105; s_min = s_max - 9; }

                double xk[10], yk[10];
                int n = 0;
                for (int s = s_min; s <= s_max; ++s) {
                    int x_s = 30 * s;
                    xk[n] = static_cast<double>(x_s);
                    yk[n] = A[i][x_s];
                    n++;
                }
                A[i][j] = LagrangeInterpolation(xk, yk, n, static_cast<double>(j));
            }
        }
    }
}

void R_fillArray(double A[27][3288]) {
    if (A[1][20] != 0) {
        for (int i = 1; i <= 24; ++i) {
            for (int j = 51; j <= 2929; ++j) {
                int k = j / 10;
                int s_min = k - 4;
                int s_max = k + 5;
                if (s_min < 1) { s_min = 1; s_max = s_min + 9; }
                if (s_max > 297) { s_max = 297; s_min = s_max - 9; }

                double xk[10], yk[10];
                int n = 0;
                for (int s = s_min; s <= s_max; ++s) {
                    int x_s = 10 * s;
                    xk[n] = static_cast<double>(x_s);
                    yk[n] = A[i][x_s];
                    n++;
                }
                A[i][j] = LagrangeInterpolation(xk, yk, n, static_cast<double>(j));
            }
        }
    }
    else {
        for (int i = 1; i <= 24; ++i) {
            for (int j = 91; j <= 3029; ++j) {
                int k = j / 30;
                int s_min = k - 4;
                int s_max = k + 5;
                if (s_min < 0) { s_min = 0; s_max = s_min + 9; }
                if (s_max > 105) { s_max = 105; s_min = s_max - 9; }

                double xk[10], yk[10];
                int n = 0;
                for (int s = s_min; s <= s_max; ++s) {
                    int x_s = 30 * s;
                    xk[n] = static_cast<double>(x_s);
                    yk[n] = A[i][x_s];
                    n++;
                }
                A[i][j] = LagrangeInterpolation(xk, yk, n, static_cast<double>(j));
            }
        }
    }
}

// ========== Clean redundant data filled at head and tail ==========
void cleanSp3(sp3& SP3) {

    if (SP3.X[1][20] != 0) {
        for (int i = 1; i <= 2880; i++) {
            for (int j = 1; j <= 32; j++) {
                SP3.X[j][i] = SP3.X[j][i + 49];
                SP3.Y[j][i] = SP3.Y[j][i + 49];
                SP3.Z[j][i] = SP3.Z[j][i + 49];

            }
        }
        for (int i = 1; i <= 2880; i++) {
            for (int j = 1; j <= 46; j++) {
                SP3.CX[j][i] = SP3.CX[j][i + 49];
                SP3.CY[j][i] = SP3.CY[j][i + 49];
                SP3.CZ[j][i] = SP3.CZ[j][i + 49];

            }
        }
        for (int i = 1; i <= 2880; i++) {
            for (int j = 1; j <= 36; j++) {
                SP3.EX[j][i] = SP3.EX[j][i + 49];
                SP3.EY[j][i] = SP3.EY[j][i + 49];
                SP3.EZ[j][i] = SP3.EZ[j][i + 49];

            }
        }
        for (int i = 1; i <= 2880; i++) {
            for (int j = 1; j <= 24; j++) {
                SP3.RX[j][i] = SP3.RX[j][i + 49];
                SP3.RY[j][i] = SP3.RY[j][i + 49];
                SP3.RZ[j][i] = SP3.RZ[j][i + 49];

            }
        }
    }
    else {
        for (int i = 1; i <= 2880; i++) {
            for (int j = 1; j <= 32; j++) {
                SP3.X[j][i] = SP3.X[j][i + 149];
                SP3.Y[j][i] = SP3.Y[j][i + 149];
                SP3.Z[j][i] = SP3.Z[j][i + 149];

            }
        }
        for (int i = 1; i <= 2880; i++) {
            for (int j = 1; j <= 24; j++) {
                SP3.RX[j][i] = SP3.RX[j][i + 149];
                SP3.RY[j][i] = SP3.RY[j][i + 149];
                SP3.RZ[j][i] = SP3.RZ[j][i + 149];

            }
        }
        for (int i = 1; i <= 2880; i++) {
            for (int j = 1; j <= 46; j++) {
                SP3.CX[j][i] = SP3.CX[j][i + 149];
                SP3.CY[j][i] = SP3.CY[j][i + 149];
                SP3.CZ[j][i] = SP3.CZ[j][i + 149];

            }
        }
        for (int i = 1; i <= 2880; i++) {
            for (int j = 1; j <= 36; j++) {
                SP3.EX[j][i] = SP3.EX[j][i + 149];
                SP3.EY[j][i] = SP3.EY[j][i + 149];
                SP3.EZ[j][i] = SP3.EZ[j][i + 149];

            }
        }
    }
}

void processSP3(sp3 SP3[]) {
    mergeSP3Data(SP3);

    G_fillArray(SP3[1].X);
    G_fillArray(SP3[1].Y);
    G_fillArray(SP3[1].Z);

    C_fillArray(SP3[1].CX);
    C_fillArray(SP3[1].CY);
    C_fillArray(SP3[1].CZ);

    E_fillArray(SP3[1].EX);
    E_fillArray(SP3[1].EY);
    E_fillArray(SP3[1].EZ);

    R_fillArray(SP3[1].RX);
    R_fillArray(SP3[1].RY);
    R_fillArray(SP3[1].RZ);

    cleanSp3(SP3[1]);
}

