#include "utils.hpp"

#include <iostream>
#include <fstream>

// #define DEBUG

struct NaiveBayesModel {
    std::map<double, double> y_coeffs;
    std::map<std::pair<int, double>, double> xy_normal_mean;
    std::map<std::pair<int, double>, double> xy_normal_var;

    double evaluate(const std::vector<float>& input)
    {
        // P(y|x) = P(y)*P(x|y)/Sum(P(y)*P(x|y))
        // Expected value: Sum(y*P(y|x)) (alternate: value for class with maximum probability).
        double prob_sum = 0.00000001;
        double value = 0;
        for (auto& y_coeff : y_coeffs)
        {
            double y = y_coeff.first;
            double prob = y_coeff.second;

            for (int i = 0; i < input.size(); i++)
            {
                double mean = xy_normal_mean.at(std::pair<int, double>(i, y));
                double var = xy_normal_var.at(std::pair<int, double>(i, y));
                prob *= std::exp(-0.5*(input.at(i) - mean)*(input.at(i) - mean)/var);
            }
            prob_sum += prob;
            value += y*prob;
        }
        return value/prob_sum;
    }
};

NaiveBayesModel loadModelNaiveBayes(const std::string& model_path)
{
    auto model_json = loadModelJSON(model_path);

    // Get coeffs.
    NaiveBayesModel m;
    auto y_coeffs = model_json.at("y_coeffs").as_array();
    for (auto y_coeff : y_coeffs)
    {
        auto y_coeff_obj = y_coeff.as_object();
        auto key = (double)y_coeff_obj.at("input").as_double();
        auto val = (double)y_coeff_obj.at("output").as_double();
        m.y_coeffs.emplace(key, val);
    }
    auto xy_coeffs = model_json.at("xy_normal_coeffs").as_array();
    for (auto xy_coeff : xy_coeffs)
    {
        auto xy_coeff_obj = xy_coeff.as_object();
        auto input = xy_coeff_obj.at("input").as_object();
        auto i = input.at("i").as_int64();
        auto y = (double)input.at("y").as_double();
        auto output = xy_coeff_obj.at("output").as_array();
        auto mean = (double)output[0].as_double();
        auto var = (double)output[1].as_double();
        m.xy_normal_mean.emplace(std::pair<int, double>(i, y), mean);
        m.xy_normal_var.emplace(std::pair<int, double>(i, y), var);
   }
   return m;
}

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
        std::cout << "Usage: ./infer_naive_bayes -i <model_file> -t <threshold> <test_file> \n";

        std::cout << std::endl;
        std::cout << "Options: " << std::endl;
        std::cout << "-i <model_file> | --input <model_file>: " << "Input model for which inference is run." << std::endl;
        std::cout << "-t <value> | --threshold <value>: " << "Error threshold to evaluate precision of the model inference." << std::endl;
        std::cout << "-h | --help: " << "Usage instructions for the tool." << std::endl;
        return 0;
    }

    std::string model_path, test_data_path;
    double threshold = 0.5;

    if (config_map.find("i") != config_map.end())
    {
        model_path = config_map["i"];
    }
    if (config_map.find("input") != config_map.end())
    {
        model_path = config_map["input"];
    }
    if (config_map.find("t") != config_map.end())
    {
        threshold = std::stof(config_map["t"].c_str());
    }
    if (config_map.find("threshold") != config_map.end())
    {
        threshold = std::stof(config_map["threshold"].c_str());
    }
    test_data_path = argv[argc - 1];

    auto model = loadModelNaiveBayes(model_path);
    auto test_data = loadData(test_data_path);

    double squared_error = 0.0;
    int error_count = 0;
    for (auto & r : test_data)
    {
        double val = model.evaluate(r.first);
        // std::cout << "Expected: " << r.second << ", Inferred: " << val << std::endl;
        squared_error += (r.second - val)*(r.second - val);
        if (abs(val - r.second) > threshold) error_count++;
    }
    squared_error = squared_error/test_data.size();
    std::cout << "RMSE: " << std::sqrt(squared_error) << std::endl;
    std::cout << "Precision: " << 1 - ((double)error_count/test_data.size()) << std::endl;
    return 0;
}
