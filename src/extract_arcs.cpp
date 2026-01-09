#include "extract_arcs.h"

// Identify all non-zero arcs in the input array
void extract_arcs(const double* array, int len, std::vector<std::vector<int>>& arcs, int& num_arcs) {

    std::vector<double> vec(array, array + len);
    vec.erase(vec.begin());      
    len = vec.size();

    arcs.clear();
    num_arcs = 0;

    int start = -1;

    // Traverse the array to detect arcs
    for (int i = 0; i < len; i++) {
        if (vec[i] != 0 && start == -1) {
            start = i;           // Start of a new arc
        }
        else if (vec[i] == 0 && start != -1) {
            arcs.push_back({ start + 1, i }); // End of arc
            num_arcs++;
            start = -1;
        }
    }

    // Handle the last arc if it reaches the end
    if (start != -1) {
        arcs.push_back({ start + 1, len });
        num_arcs++;
    }
}
