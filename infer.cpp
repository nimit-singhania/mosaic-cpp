#include "utils.hpp"

#include <iostream>
#include <fstream>

int main(int argc, char** argv)
{
    if (argc < 4)
    {
        std::cout << "Usage: ./infer <model_file> <test_file> <threshold>\n";
        return 0;
    }

    auto model = loadModelJSON(argv[1]);
    auto test_data = loadData(argv[2]);
    float threshold = std::stof(argv[3]);

    float squared_error = 0.0;
    int error_count = 0;
    for (auto & r : test_data)
    {
        float val = model.evaluate(r.first);
        // std::cout << "Expected: " << r.second << ", Inferred: " << val << std::endl;
        squared_error += (r.second - val)*(r.second - val);
        if (abs(val - r.second) > threshold) error_count++;
    }
    squared_error = squared_error/test_data.size();
    std::cout << "RMSE: " << std::sqrt(squared_error) << std::endl;
    std::cout << "Precision: " << 1 - ((float)error_count/test_data.size()) << std::endl;
    return 0;
}
