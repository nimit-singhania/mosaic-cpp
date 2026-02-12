#include "utils.hpp"
#include <iostream>
#include <fstream>
#include <map>
#include <string>

#include "boost/json.hpp"


boost::json::object learnNaiveBayesModelFromData(
    const std::map<std::vector<float>, float>& data)
{
    // Naive bayes model assumes: P(X|y) = P(X1|y)*P(X2|,y)*...*P(Xn|y)
    // Therefore, P(y|X) = P(X,y)/P(X) = P(X|y)*P(y)/P(X) ~ P(X|y)*P(y)
    // Given the probabilities of different classes, we can then do an expectation
    // to get the desired output for a given point.

    // We need to store the following coefficients therefore:
    // P(y): probabilities of different output values.
    // P(Xi|y): probability of a specific value of Xi given y. This can be represented by a normal distribution.

    std::map<float, float> prob_y;
    // Map from xi, y to distribution params. Xi is captured by index number.
    std::map<std::pair<int, float>, float> mean_xy;
    std::map<std::pair<int, float>, float> var_xy;

    // Compute P(y)
    std::map<float, int> ycounts;
    for (auto& p : data)
    {
        auto y = p.second;
        if (ycounts.find(y) == ycounts.end())
        {
            ycounts.emplace(y, 1);
        }
        else
            ycounts[y]++;
    }
    for (auto ycount: ycounts)
    {
        prob_y.emplace(ycount.first, ((float)ycount.second)/data.size());
    }

    for (auto ycount: ycounts)
    {
        auto y = ycount.first;
        auto count = ycount.second;
        for (int i = 0; i < data.begin()->first.size(); i++)
        {
            float sum = 0, sq_sum = 0;
            for (auto p : data)
            {
                if (p.second != y) continue;

                sum += p.first[i];
                sq_sum += p.first[i]*p.first[i];
            }
            mean_xy.emplace(std::pair<int, float>(i, y), sum/count);
            var_xy.emplace(std::pair<int, float>(i, y), sq_sum/count - (sum/count)*(sum/count));
        }
    }

    // Output model.
    boost::json::object model;
    boost::json::array py;
    for (auto p : prob_y)
    {
        boost::json::object py_inst;
        py_inst["input"] = p.first;
        py_inst["output"] = p.second;
        py.push_back(py_inst);
    }
    boost::json::array pxy;
    for (auto p : mean_xy)
    {
        auto input = p.first;
        boost::json::object input_obj;
        input_obj["i"] = input.first;
        input_obj["y"] = input.second;
        boost::json::array output;
        output.push_back(mean_xy[input]);
        output.push_back(var_xy[input]);
        boost::json::object entry;
        entry["input"] = input_obj;
        entry["output"] = output;
        pxy.push_back(entry);
    }
    model["y_coeffs"] = py;
    model["xy_normal_coeffs"] = pxy;
    return model;
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
        std::cout << " -o <path> | --output <path>: "
                  << " The file path to output learnt model." << std::endl;
        std::cout << " -h | --help: "
                  << "Usage and options for the model training." << std::endl;
        return 0;
    }

    float threshold = 0.5;
    std::string path_to_train_data, path_to_output_model;
    if (config_map.find("o") != config_map.end())
    {
        path_to_output_model = config_map["o"];
    }
    if (config_map.find("output") != config_map.end())
    {
        path_to_output_model = config_map["output"];
    }
    path_to_train_data = argv[argc - 1];

    std::cout << "Loading data ... " << std::endl;
    auto data = loadData(path_to_train_data);
    std::cout << "Training naive Bayes model." << std::endl;
    auto m = learnNaiveBayesModelFromData(data);
    if (path_to_output_model.empty())
    {
        std::cout << "Model Output: " << m << std::endl;
    }
    else
    {
        //boost::json::serialize(model_json, path_to_output_model);
        std::fstream fs;
        fs.open(path_to_output_model, std::ios::out);
        if (!fs.is_open()) return 0;

        boost::json::serializer sr;
        sr.reset( &m );
        while( ! sr.done() )
        {
            char buf[ 4000 ];
            fs << sr.read( buf );
        }

        fs.close();
    }

    return 0;
}
