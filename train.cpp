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

    if (argc < 3)
    {
        std::cout << "Usage: ./main <path_to_data> <threshold> [<path_to_output_model]" << std::endl;
        return 0;
    }

    std::cout << "Loading data ... " << std::endl;
    auto data = loadData(std::string(argv[1]));
    std::cout << "Training piecewise affine model." << std::endl;
    auto m = learnModelFromData(data, std::stod(argv[2]));
    if (argc < 4)
    {
        std::cout << "Model Output: " << std::endl;
        outputModel(m);
    }
    if (argc == 4)
    {
        auto model_path = argv[3];
        auto model_json = outputModelJSON(m);
        //boost::json::serialize(model_json, model_path);
        std::fstream fs;
        fs.open(model_path, std::ios::out);
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
