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

    for (auto & r : test_data)
    {
        std::cout << "Expected: " << r.second << ", Inferred: " << model.evaluate(r.first) << std::endl;
    }
    return 0;
}
