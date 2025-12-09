#include "utils.hpp"

#include <iostream>
#include <fstream>

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: ./infer <model_file> <test_file>\n";
        return 0;
    }

    auto model = loadModelJSON(argv[1]);
    auto test_data = loadData(argv[2]);

    float squared_error = 0.0;
    for (auto & r : test_data)
    {
        float val = model.evaluate(r.first);
        std::cout << "Expected: " << r.second << ", Inferred: " << val << std::endl;
        squared_error += (r.second - val)*(r.second - val); 
    }
    squared_error = squared_error/test_data.size();
    std::cout << "RMSE: " << std::sqrt(squared_error) << std::endl;
    return 0;
}
