#include "HungarianAlgo.h"
#include <limits>   // For std::numeric_limits
#include <algorithm> // For std::max
#include <vector>    // Explicitly include for std::vector

// Removed: #include <queue> // Was unused

std::vector<int> HungarianAlgo::solveHungarian(const std::vector<std::vector<float>>& costMatrix) {
    if (costMatrix.empty() || costMatrix[0].empty()) {
        return {};
    }

    const int original_rows = costMatrix.size();
    const int original_cols = costMatrix[0].size();
    // n is the size of the square matrix used for the algorithm
    const int n = std::max(original_rows, original_cols);

    // Padded cost matrix (n x n)
    // Initialize with 0.0f. If costs can be extremely large, or if padded
    // assignments should be heavily penalized, a large constant (infinity)
    // might be used for padding instead of 0 for specific scenarios.
    // For standard minimization, 0 is fine for cells outside original_rows/original_cols.
    std::vector<std::vector<float>> cost(n, std::vector<float>(n, 0.0f));
    for (int i = 0; i < original_rows; ++i) {
        for (int j = 0; j < original_cols; ++j) {
            cost[i][j] = costMatrix[i][j];
        }
    }

    // Potentials for rows (u) and columns (v) - 0-indexed
    std::vector<float> u(n, 0.0f);
    std::vector<float> v(n, 0.0f);

    // p[j_1_based] stores the 1-based row index matched to column j_1_based.
    // p[0] is used as a temporary variable to store the current row being processed.
    std::vector<int> p(n + 1, 0); // 1-indexed, p[j]=0 means column j is unmatched

    // way[j_1_based] stores the previous column in the augmenting path exploration.
    std::vector<int> way(n + 1, 0); // 1-indexed

    const float infinity = std::numeric_limits<float>::max();

    // Main loop: iterate for each row (1 to n) of the square matrix
    for (int i = 1; i <= n; ++i) { // i is the 1-based current row to find a match for
        p[0] = i; // The row i is the starting point of the augmenting path search
                  // It's conceptually linked to a dummy "column 0"

        // minv[j_1_based] stores the minimum slack to column j_1_based from rows in the tree
        std::vector<float> minv(n + 1, infinity);
        std::vector<bool> used(n + 1, false); // Marks visited 1-based columns in current path search

        int j0 = 0; // j0 is the 1-based column index from which we extend the path.
                    // Starts with dummy column 0.

        do {
            used[j0] = true; // Mark column j0 as visited (part of the alternating tree)
            int i0 = p[j0];  // i0 is the 1-based row matched to column j0 (or current row i if j0=0)
            float delta = infinity;
            int j1_next_col = -1; // The next 1-based column to add to the tree

            // Find the unvisited column j with the minimum slack
            for (int j = 1; j <= n; ++j) { // Iterate over all 1-based columns
                if (!used[j]) {
                    // Calculate slack: cost[i0-1][j-1] - u[i0-1] - v[j-1]
                    // (i0-1, j-1 convert to 0-based indices for cost, u, v)
                    float current_slack = cost[i0 - 1][j - 1] - u[i0 - 1] - v[j - 1];
                    if (current_slack < minv[j]) {
                        minv[j] = current_slack;
                        way[j] = j0; // j0 is the parent of j in the alternating tree
                    }
                    if (minv[j] < delta) {
                        delta = minv[j];
                        j1_next_col = j;
                    }
                }
            }

            // Update potentials u, v and slacks minv
            for (int j = 0; j <= n; ++j) { // Iterate 0 to n (0 for dummy, 1..n for real columns)
                if (used[j]) {
                    // For rows/columns in the current alternating tree:
                    // p[j]-1 gives 0-based row index
                    u[p[j] - 1] += delta;
                    // v[j-1] gives 0-based column index
                    // *** CRITICAL FIX HERE: Only update v if j > 0 (i.e., it's a real column) ***
                    if (j > 0) {
                        v[j - 1] -= delta;
                    }
                } else {
                    // For columns not in the tree, reduce their slacks
                    minv[j] -= delta;
                }
            }
            j0 = j1_next_col; // Move to the next column in the path
        } while (p[j0] != 0); // Continue until an unmatched column j0 is found (p[j0]==0)

        // Augment the path:
        // Backtrack from j0 using 'way' array and flip assignments
        do {
            int j1_prev_in_path = way[j0];
            p[j0] = p[j1_prev_in_path]; // Assign row p[j1_prev_in_path] to column j0
            j0 = j1_prev_in_path;
        } while (j0 != 0); // Stop when we reach the dummy column 0
    }

    // Construct the result vector for the original matrix dimensions
    std::vector<int> result(original_rows, -1); // result[orig_row_idx] = orig_col_idx, or -1 if unassigned
    for (int j_1_based = 1; j_1_based <= n; ++j_1_based) {
        // p[j_1_based] is the 1-based row matched to 1-based column j_1_based
        // Convert to 0-based indices for original matrix context:
        int assigned_row_0_based = p[j_1_based] - 1;
        int col_0_based = j_1_based - 1;

        // Only include assignments that are within the bounds of the original matrix
        if (assigned_row_0_based < original_rows && col_0_based < original_cols) {
            // It's possible p[j_1_based] was 0 if it was never properly assigned a row > 0
            // However, after the algorithm, p[j] for j=1..n should be a permutation of 1..n.
            // So assigned_row_0_based should be >= 0.
            // The check `assigned_row_0_based < original_rows` implies assigned_row_0_based is valid.
            result[assigned_row_0_based] = col_0_based;
        }
    }
    return result;
}
