#include "utils.hpp"

#include <iostream>

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: ./model_stats <model_file>" << std::endl;
    }
    auto model_json = loadModelJSON(argv[1]);
    auto m = parseModelJSON(model_json);

    std::cout << "Number of Regions: " << m.regions.size();

    for (int i = 0; i < m.regions.size(); i++)
    {
        std::cout << "Region: " << i << std::endl;

        std::cout << "-- Affine Function: ";
        outputAffineFunction(m.regions[i].f);
        std::cout << std::endl;

        std::cout << "-- Predicate Complexity: " << m.regions[i].g.clauses.size();
        if (m.regions[i].g.clauses.size() > 0)
        {
            auto clauses = m.regions[i].g.clauses;
            std::cout << " x " << clauses[0].terms.size();
        }
        std::cout << std::endl;
    }
    return 0;
}
