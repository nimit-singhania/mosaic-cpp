#include "utils.hpp"

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>

// #define DEBUG
std::map<std::vector<float>, float> loadData(const std::string& path)
{
    std::map<std::vector<float>, float> m;
    FILE* fp = std::fopen(path.c_str(), "r");
    if (!fp)
    {
        std::cerr << "Could not open file for loading data: " << path << std::endl;
        return m;
    }

    // Format of the data:
    // Each line consists of one input output pair.
    // The values are comma separated with the last value as the output.
    std::string buf;
    char c;
    while (!std::feof(fp))
    {
        c = std::fgetc(fp);
        while (c != '\n' && c != EOF)
        {
            buf.push_back(c);
            c = std::fgetc(fp);
        }

        // Read float values from buf.
        std::vector<float> input;
        int pos = 0;
        while (pos < buf.size())
        {
            int end_pos = buf.find_first_of(",", pos);
            if (end_pos == std::string::npos)
            {
                m.emplace(input, std::stof(buf.substr(pos).c_str()));
                input.clear();
                break;
            }
            input.push_back(std::stof(buf.substr(pos, end_pos - pos).c_str()));;
            pos = end_pos + 1;
        }
        buf.clear();
    }
    std::fclose(fp);
    return m;
}

std::string vectorString(const std::vector<float>& v)
{
    std::string s;
    s.append("(");
    for (auto x: v)
    {
        s.append(std::to_string(x));
        s.append(", ");
    }
    s.append(")");
    return s;
}

void outputAffineFunction(const affineFunction& f,
                          const std::vector<float>& scale_vec)
{
    if (scale_vec.empty())
        for (int i = 0; i < f.coeff.size() - 1; i++)
        {
            std::cout << f.coeff[i] << ".x" << i << " + ";
        }
    else
        for (int i = 0; i < f.coeff.size() - 1; i++)
        {
            std::cout << f.coeff[i]/scale_vec[i] << ".x" << i << " + ";
        }
    std::cout << f.coeff[f.coeff.size() - 1];
}

void outputPredicate(const predicate& p,
                     const std::vector<float>& scale_vec)
{
    if (scale_vec.empty())
        for (int i = 0; i < p.coeff.size() - 1; i++)
        {
            std::cout << p.coeff[i] << ".x" << i << " + ";
        }
    else
        for (int i = 0; i < p.coeff.size() - 1; i++)
        {
            std::cout << p.coeff[i]/scale_vec[i] << ".x" << i << " + ";
        }
    std::cout << p.coeff[p.coeff.size() - 1] << " >= 0";
}

void outputOrPredicate(const guardPredicate::orPredicate& o,
                       const std::vector<float>& scale_vec)
{
    if (o.terms.size() == 1)
        outputPredicate(o.terms[0], scale_vec);
    else
    {
        std::cout << "OR(";
        for (auto& t : o.terms)
        {
            outputPredicate(t, scale_vec);
            std::cout << ", ";
        }
        std::cout << ")";
    }
}

void outputGuardPredicate(const guardPredicate& g,
                          const std::vector<float>& scale_vec)
{
    if (g.clauses.size() == 1)
        outputOrPredicate(g.clauses[0], scale_vec);
    else
    {
        std::cout << "AND(";
        for (auto& c : g.clauses)
        {
            outputOrPredicate(c, scale_vec);
            std::cout << ", ";
        }
        std::cout << ")";
    }
}


void outputModel(const piecewiseAffineModel& model)
{
    int i = 0;
    for (auto& r : model.regions)
    {
        std::cout << "Region " << i++ << std::endl;
        std::cout << "-- affine function: ";
        outputAffineFunction(r.f, model.scale_vec);
        std::cout << std::endl;

        std::cout << "-- guard: ";
        outputGuardPredicate(r.g, model.scale_vec);
        std::cout << std::endl;
    }
}

float distance(const std::vector<float>& p1, const std::vector<float>& p2)
{
    // returns distance between p1 and p2 in the vector space using L2 norm.
    float dist = 0.0;
    for (int i = 0; i < p1.size(); i++)
    {
        dist += (p1[i]-p2[i])*(p1[i]-p2[i]);
    }
    return std::sqrt(dist);
}

guardPredicate true_predicate(int n)
{
    // True predicate: 1 >= 0.
    guardPredicate g;
    guardPredicate::orPredicate o;
    predicate pred;
    for (int i = 0; i < n; i++)
    {
        pred.coeff.push_back(0.0);
    }
    pred.coeff.push_back(1.0);
    o.terms.push_back(pred);
    g.clauses.push_back(o);
    return g;
}

guardPredicate false_predicate(int n)
{
    // True predicate: 1 >= 0.
    guardPredicate g;
    guardPredicate::orPredicate o;
    predicate pred;
    for (int i = 0; i < n; i++)
    {
        pred.coeff.push_back(0.0);
    }
    pred.coeff.push_back(-1.0);
    o.terms.push_back(pred);
    g.clauses.push_back(o);
    return g;
} 
