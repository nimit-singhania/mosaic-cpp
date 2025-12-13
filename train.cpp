#include "Solvers.hpp"
#include "utils.hpp"

#include <iostream>
#include <fstream>
#include <string>

#include "boost/json.hpp"

int sample()
{
    // load data
    std::map<std::vector<float>, float> data;
    for (float i = 1; i <= 100; i+=1)
    {
        for (float j = 1; j <= 100; j+=1)
        {
            if (i <= 50 && j <= 50 || i > 50 && j > 50)
            {
                data.emplace(std::vector<float>({(float)i, (float)j}), 1.0);
            }
            else
            {
                data.emplace(std::vector<float>({(float)i, (float)j}), -1.0);
            }
        }
    }

    piecewiseAffineModel m = learnModelFromData(data, 0.1);

    std::cout << "Number of regions: " << m.regions.size() << std::endl;
    for (int i = 0; i < m.regions.size(); i++)
    {
        std::cout << "Region " << i << std::endl;
        std::cout << "Affine function: ";
        auto f = m.regions[i].f;
        for (int j = 0; j < f.coeff.size() - 1; j++)
        {
            std::cout << f.coeff[j] << ".x" << j << " + ";
        }
        std::cout << f.coeff[f.coeff.size() - 1] << std::endl;
    }

    std::cout << "Evaluate function: " << std::endl;
    std::cout << "Point (50, 50): (expected 1.0) " << m.evaluate({50, 50}) << std::endl;
    std::cout << "Point (51, 51): (expected 1.0) " << m.evaluate({51, 51}) << std::endl;
    std::cout << "Point (51, 49): (expected -1.0) " << m.evaluate({51, 49}) << std::endl;
    std::cout << "Point (49, 51): (expected -1.0)" << m.evaluate({49, 51}) << std::endl;
    std::cout << "Point (49.99, 50.01): (expected -1.0)" << m.evaluate({49.999, 50.001}) << std::endl;

    return 0;
}

int main(int argc, char** argv)
{
    // Run a sample program and evaluate correctness.
    // sample();

    auto config_map = read_configuration(argc, argv);

    if (config_map.find("h") != config_map.end() ||
        config_map.find("help") != config_map.end())
    {
        std::cout << "Usage: ./main -t <threshold> [-o <path_to_output_model>] <path_to_train_data>" << std::endl;
        std::cout << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << " -t <value> | --threshold <value>: "
                  << " The error threshold for training on input-data." << std::endl;
        std::cout << " -o <path> | --output <path>: "
                  << " The file path to output learnt model." << std::endl;
        std::cout << " -s <value> | --num_splits <value>: "
                  << "Number of split iterations during guard predicate training." << std::endl;
        std::cout << " -h | --help: "
                  << "Usage and options for the model training." << std::endl;
        return 0;
    }

    float threshold = 0.5;
    std::string path_to_train_data, path_to_output_model;
    if (config_map.find("t") != config_map.end())
    {
        threshold = std::stof(config_map["t"]);
    }
    if (config_map.find("threshold") != config_map.end())
    {
        threshold = std::stof(config_map["threshold"]);
    }
    if (config_map.find("o") != config_map.end())
    {
        path_to_output_model = config_map["o"];
    }
    if (config_map.find("output") != config_map.end())
    {
        path_to_output_model = config_map["output"];
    }
    if (config_map.find("s") != config_map.end())
    {
        num_splits = std::stoi(config_map["s"]);
    }
    if (config_map.find("num_splits") != config_map.end())
    {
        num_splits = std::stoi(config_map["num_splits"]);
    }
    path_to_train_data = argv[argc - 1];

    std::cout << "Loading data ... " << std::endl;
    auto data = loadData(path_to_train_data);
    std::cout << "Training piecewise affine model." << std::endl;
    auto m = learnModelFromData(data, threshold);
    if (path_to_output_model.empty())
    {
        std::cout << "Model Output: " << std::endl;
        outputModel(m);
    }
    else
    {
        auto model_json = outputModelJSON(m);
        //boost::json::serialize(model_json, path_to_output_model);
        std::fstream fs;
        fs.open(path_to_output_model, std::ios::out);
        if (!fs.is_open()) return 0;

        boost::json::serializer sr;
        sr.reset( &model_json );
        while( ! sr.done() )
        {
            char buf[ 4000 ];
            fs << sr.read( buf );
        }

        fs.close();
    }

    return 0;
}
