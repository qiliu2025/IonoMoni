#include "obs.h"

obs::obs() {
    for (int i = 0; i < 60; ++i) {
        std::fill(C1[i], C1[i] + 2888, 0.0);
        std::fill(C2[i], C2[i] + 2888, 0.0);
        std::fill(L1[i], L1[i] + 2888, 0.0);
        std::fill(L2[i], L2[i] + 2888, 0.0);
        std::fill(S1[i], S1[i] + 3688, 0.0); 
        std::fill(S2[i], S2[i] + 3688, 0.0); 
    }
}

void obs::reset() {
    for (int i = 0; i < 60; ++i) {
        std::fill(C1[i], C1[i] + 2888, 0.0);
        std::fill(C2[i], C2[i] + 2888, 0.0);
        std::fill(L1[i], L1[i] + 2888, 0.0);
        std::fill(L2[i], L2[i] + 2888, 0.0);
        std::fill(S1[i], S1[i] + 3688, 0.0); 
        std::fill(S2[i], S2[i] + 3688, 0.0); 
    }
}
