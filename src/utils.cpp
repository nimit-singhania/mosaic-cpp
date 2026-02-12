#include "utils.hpp"

#include <iostream>
#include <fstream>
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

boost::json::object loadModelJSON(const std::string& model_path)
{
    std::fstream fs;
    fs.open(model_path);
    if (!fs.is_open())
        return boost::json::object();

    std::string serialized;
    char c;
    while ((c = fs.get()) != EOF)
        serialized.push_back(c);

    auto model_jv = boost::json::parse(serialized);

    fs.close();
    return model_jv.as_object();
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

boost::json::object outputAffineFunctionJSON(const affineFunction& f)
{
    boost::json::object func;
    boost::json::array coeffs;
    for (auto c : f.coeff)
        coeffs.push_back(c);
    func["coeff"] = coeffs;
    return func;
}

boost::json::object outputPredicateJSON(const predicate& g)
{
    boost::json::object func;
    boost::json::array coeffs;
    for (auto c : g.coeff)
        coeffs.push_back(c);
    func["coeff"] = coeffs;
    return func;
}

boost::json::object outputPredicateJSON(const guardPredicate::orPredicate& g)
{
    boost::json::object func;
    boost::json::array terms;
    for (auto t : g.terms)
        terms.push_back(outputPredicateJSON(t));
    func["terms"] = terms;
    return func;
}

boost::json::object outputPredicateJSON(const guardPredicate& g)
{
    boost::json::object func;
    boost::json::array clauses;
    for (auto c : g.clauses)
        clauses.push_back(outputPredicateJSON(c));
    func["clauses"] = clauses;
    return func;
}

boost::json::object outputRegionJSON(const piecewiseAffineModel::region& r)
{
    boost::json::object region_json;
    boost::json::object affine_function_json = outputAffineFunctionJSON(r.f);
    boost::json::object guard_json = outputPredicateJSON(r.g);
    region_json["f"] = affine_function_json;
    region_json["g"] = guard_json;
    return region_json;
}

boost::json::object outputModelJSON(const piecewiseAffineModel& model)
{
    boost::json::object model_json;
    boost::json::array regions;
    boost::json::array scale_vec;
    for (auto& r : model.regions)
    {
        regions.push_back(outputRegionJSON(r));
    }
    model_json["regions"] = regions;
    for (auto &c : model.scale_vec)
    {
        scale_vec.push_back(c);
    }
    model_json["scale_vec"] = scale_vec;
    return model_json;
}

affineFunction parseAffineFunctionJSON(const boost::json::object& func)
{
    affineFunction f;
    auto coeffs = func.at("coeff").as_array();
    for (auto &c : coeffs)
    {
        f.coeff.push_back((float)c.as_double());
    }
    return f;
}

predicate parsePredicateJSON(const boost::json::object& func)
{
    predicate g;
    auto coeffs = func.at("coeff").as_array();
    for (auto &c : coeffs)
    {
        g.coeff.push_back((float)c.as_double());
    }
    return g;
}

guardPredicate::orPredicate parseOrPredicateJSON(const boost::json::object& func)
{
    guardPredicate::orPredicate o;
    auto terms = func.at("terms").as_array();
    for (auto &t : terms)
    {
        o.terms.push_back(parsePredicateJSON(t.as_object()));
    }
    return o;
}

guardPredicate parseGuardPredicateJSON(const boost::json::object& func)
{
    guardPredicate g;
    auto clauses = func.at("clauses").as_array();
    for (auto &c : clauses)
    {
        g.clauses.push_back(parseOrPredicateJSON(c.as_object()));
    }
    return g;
}

piecewiseAffineModel::region parseRegionJSON(const boost::json::object& region)
{
    piecewiseAffineModel::region r;
    r.f = parseAffineFunctionJSON(region.at("f").as_object());
    r.g = parseGuardPredicateJSON(region.at("g").as_object());
    return r;
}

piecewiseAffineModel parseModelJSON(const boost::json::object& model_json)
{
    piecewiseAffineModel m;

    // Set scale vector.
    auto scale_vec = model_json.at("scale_vec").as_array();
    for (auto &s : scale_vec)
    {
        m.scale_vec.push_back((float)s.as_double());
    }

    auto regions = model_json.at("regions").as_array();
    for (auto &r : regions)
    {
        m.regions.push_back(parseRegionJSON(r.as_object()));
    }
    return m;
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

bool contains_value(std::string config)
{
    return config.find_first_of("=") != std::string::npos;
}

std::map<std::string, std::string> read_configuration(int argc, char** argv)
{
    // Format: (--config | -c | --config=value | --config value)*
    std::map<std::string, std::string> config_map;
    for (int i = 1; i < argc; i++)
    {
        std::string config = argv[i];
        if (config.find_first_of("-") == std::string::npos)
            continue;
    
        if (contains_value(argv[i]))
        {
            // format --config=value.
            std::string confg_and_value = argv[i];
            int start_pos = confg_and_value.find_last_of("-");
            int pos = confg_and_value.find_first_of("=");
            config_map.emplace(confg_and_value.substr(start_pos + 1, pos - start_pos),
                               confg_and_value.substr(pos+1));
        }
        else
        {
            // -c v | -c
            int pos = config.find_last_of("-");
            if (i < argc - 1 && argv[i+1][0] != '-')
                config_map.emplace(config.substr(pos+1), argv[i+1]);
            else
                config_map.emplace(config.substr(pos+1), "1");
        }
    }
    return config_map;
}
