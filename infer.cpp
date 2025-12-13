#include "utils.hpp"

#include <iostream>
#include <fstream>

// #define DEBUG

int main(int argc, char** argv)
{
#ifdef DEBUG
    std::cerr << "Reading configuration: " << std::endl;
    for (int i = 0; i < argc - 1; i++)
    {
        std::cerr << argv[i+1] << ", " << std::endl;
    }
#endif
    auto config_map = read_configuration(argc, argv);
#ifdef DEBUG
    std::cerr << "Read Configuration: " << std::endl;
    for (auto& p: config_map)
    {
        std::cerr << "(" << p.first << ": " << p.second << "), " << std::endl;
    }
#endif
    if (config_map.find("h") != config_map.end() ||
        config_map.find("help") != config_map.end())
    {
        std::cout << "Usage: ./infer -i <model_file> -t <threshold> <test_file> \n";

        std::cout << std::endl;
        std::cout << "Options: " << std::endl;
        std::cout << "-i: " << "Input model for which inference is run." << std::endl;
        std::cout << "-t: " << "Error threshold to evaluate precision of the model inference." << std::endl;
        return 0;
    }

    std::string model_path, test_data_path;
    float threshold = 0.5;

    if (config_map.find("i") != config_map.end())
    {
        model_path = config_map["i"];
    }
    if (config_map.find("t") != config_map.end())
    {
        threshold = std::stof(config_map["t"].c_str());
    }
    test_data_path = argv[argc - 1];

    auto model = loadModelJSON(model_path);
    auto test_data = loadData(test_data_path);

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
