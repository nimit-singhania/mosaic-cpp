#include "utils.hpp"

#include <iostream>
#include <fstream>

#include "boost/json.hpp"

piecewiseAffineModel load_model(char* model_path)
{
    std::fstream fs;
    fs.open(model_path);
    if (!fs.is_open())
        return piecewiseAffineModel();

    std::string serialized;
    char c;
    while ((c = fs.get()) != EOF)
        serialized.push_back(c);

    auto model_jv = boost::json::parse(serialized);
    auto m = parseModelJSON(model_jv.as_object());

    fs.close();
    return m;
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: ./infer <model_file> <test_file>\n";
        return 0;
    }

    auto model = load_model(argv[1]);
    auto test_data = loadData(argv[2]);

    for (auto & r : test_data)
    {
        std::cout << "Expected: " << r.second << ", Inferred: " << model.evaluate(r.first) << std::endl;
    }
    return 0;
}
