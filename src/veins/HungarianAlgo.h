#pragma once

#include <vector>

class HungarianAlgo {
public:
    HungarianAlgo(){};
    std::vector<int> solveHungarian(const std::vector<std::vector<float>>& costMatrix);
    bool findAugmentingPath(
    const std::vector<std::vector<float>>& cost,
    const std::vector<int>& matched_rows,
    const std::vector<int>& matched_cols,
    std::vector<bool>& covered_rows,
    std::vector<bool>& covered_cols,
    int start_row,
    std::vector<int>& path);
};
