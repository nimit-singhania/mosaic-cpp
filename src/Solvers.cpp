#include "Solvers.hpp"
#include "AlgLibUtils.hpp"
#include "utils.hpp"

#include <iostream>

// #define DEBUG

using namespace std;

guardPredicate genPredicate(const set<vector<float>>& p,
                            const set<vector<float>>& n,
                            int num_vars)
{
    predicate pred = genPredicateUsingAlgLib(p, n, num_vars);
    if (pred.coeff.empty()) return guardPredicate();

    // Check predicate.
#ifdef CHECK
    for (auto & x : p)
    {
        if (pred.evaluate(x) == false)
        {
            std::cerr << "Error generating predicate! Please check predicate.";
            std::cerr << "Point: (";
            for (int i = 0; i < x.size(); i++) std::cerr << x[i] << ", ";
            std::cerr << ")";
            std::cerr << ", Predicate: (";
            for (auto c : pred.coeff) std::cerr << c << ", ";
            std::cerr << ")" << std::endl;
        }
    }
    for (auto & x : n)
    {
        if (pred.evaluate(x) == true)
        {
            std::cerr << "Error generating predicate! Please check predicate." << std::endl;
            std::cerr << "Point: (";
            for (int i = 0; i < x.size(); i++) std::cerr << x[i] << ", ";
            std::cerr << ")";
            std::cerr << ", Predicate: (";
            for (auto c : pred.coeff) std::cerr << c << ", ";
            std::cerr << ")" << std::endl;
        }
    }
#endif

    guardPredicate g;
    guardPredicate::orPredicate o;
    o.terms.push_back(pred);
    g.clauses.push_back(o);
    return g;
}

guardPredicate genPredicate(vector<set<vector<float>>>& pos_groups,
                            set<vector<float>>& n, int num_vars)
{
    guardPredicate g;
    guardPredicate::orPredicate o;
    for (auto& p : pos_groups)
    {
        guardPredicate g_p = genPredicate(p, n, num_vars);
        if (g_p.clauses.empty()) return guardPredicate();
        o.terms.push_back(g_p.clauses[0].terms[0]);
    }
    g.clauses.push_back(o);
    return g;
}

guardPredicate genPredicate(vector<set<vector<float>>>& pos_groups,
                            vector<set<vector<float>>>& neg_groups,
                            int num_vars)
{
    guardPredicate g;
    for (auto& n: neg_groups)
    {
        guardPredicate g_n = genPredicate(pos_groups, n, num_vars);
        if (g_n.clauses.empty()) return guardPredicate();
        g.clauses.push_back(g_n.clauses[0]);
    }
    return g;
}

void split_group(const set<vector<float>>& g, const vector<float>& ce, vector<set<vector<float>>>& groups,
                 vector<set<vector<float>>>& new_groups)
{
    // Splits group g into two groups, such that the counterexample ce can be accomodated.
    // Updates groups with the new groups while erasing old group g.

    set<vector<float>> g_less, g_more;
    bool found = false;
    for (int i = 0; i < NUM_ITERATIONS; i++)
    {
        // Find an affine function f, so that f(ce) = 0, f(p) != 0 for all p in g.
        affineFunction f = findAffineFunctionPassingThroughCEOnlyAlternate(g, ce);

        g_less.clear();
        g_more.clear();
        for (auto p: g)
        {
            if (f.evaluate(p) > 0)
                g_more.emplace(p);
            else
                g_less.emplace(p);
        }
        if (g_more.size() > 0 && g_less.size() > 0 &&
            !genPredicate(g_less, {ce}, ce.size()).clauses.empty() &&
            !genPredicate(g_more, {ce}, ce.size()).clauses.empty())
        {
            found = true;
            break;
        }
    }
    if (!found)
    {
        // Manual heuristics to split the set.
        g_less.clear();
        g_more = g;
        for (int i = 0; i < g.size() - 1; i++)
        {
            auto p = *g_more.begin();
            g_less.insert(p);
            g_more.erase(g_more.begin());
            if (!genPredicate(g_less, {ce}, ce.size()).clauses.empty() &&
                !genPredicate(g_more, {ce}, ce.size()).clauses.empty())
            {
                found = true;
                break;
            }
        }
    }

#ifdef DEBUG
    std::cerr << "Split Group Result: " << std::endl;
    for (auto &p : g_less)
    {
        std::cerr << "(";
        for (auto &c : p)
            std::cerr << c << ", ";
        std::cerr << "),";
    }
    std::cerr << std::endl;
    for (auto &p : g_more)
    {
        std::cerr << "(";
        for (auto &c : p)
            std::cerr << c << ", ";
        std::cerr << "),";
    }
#endif
    // auto it = std::find(groups.begin(), groups.end(), g);
    // if (it != groups.end())
    //    groups.erase(it);

    new_groups.push_back(g_more);
    new_groups.push_back(g_less);
}

guardPredicate genGuard(set<vector<float>>& pos_points,
                        set<vector<float>>& neg_points,
                        int num_vars)
{
    // We collect groups of positive and negative points.
    // Each group forms a cluster that is separated simultaneously
    // from other clusters. By grouping points, we are able to
    // learn a single separator for all points in the group, thereby
    // improving the model learnt.
    vector<set<vector<float>>> pos_groups, neg_groups;

    if (pos_points.size() == 0) return false_predicate(num_vars);
    if (neg_points.size() == 0) return true_predicate(num_vars);

    pos_groups.push_back({*pos_points.begin()});
    neg_groups.push_back({*neg_points.begin()});

    int iter_count = 0;
    while (true)
    {
#ifdef DEBUG
        // std::cerr << "Iteration " << iter_count++ << std::endl;
#endif
        guardPredicate g = genPredicate(pos_groups, neg_groups, num_vars);
        vector<vector<float>> pos_counterexamples, neg_counterexamples;
        for (auto& p : pos_points)
        {
            if (g.evaluate(p) == false)
            {
                pos_counterexamples.emplace_back(p);
            }
        }
        for (auto& p : neg_points)
        {
            if (g.evaluate(p) == true)
                neg_counterexamples.emplace_back(p);
        }
        if (pos_counterexamples.empty() && neg_counterexamples.empty()) return g;

        // Process positive counterexample.
        if (pos_counterexamples.size() > 0)
        {
            // auto idx = alglib::randominteger(pos_counterexamples.size());
            // auto pos_ce = pos_counterexamples[idx];
            auto pos_ce = *pos_counterexamples.begin();
            vector<set<vector<float>>> new_groups;
            for (auto& n : neg_groups)
            {
                if (genPredicate({pos_ce}, n, num_vars).clauses.empty())
                {
                    // pos_ce conflicts with n.
                    // n needs to be split.
#ifdef DEBUG
                    std::cerr << "Split Groups call: " << iter_count++ << std::endl;
#endif
                    split_group(n, pos_ce, neg_groups, new_groups);
                }
                else
                    new_groups.push_back(n);
            }
            neg_groups = new_groups;
            bool merged = false;
            for (auto& p : pos_groups)
            {
                p.insert(pos_ce);
                if (genPredicate(neg_groups, p, num_vars).clauses.empty() == false)
                {
                    merged = true;
                    break;
                }
                else
                {
                    p.erase(p.find(pos_ce));
                }
            }

            if (!merged)
            {
                pos_groups.push_back({pos_ce});
            }
        }
        if (neg_counterexamples.size() > 0)
        {
            // auto idx = alglib::randominteger(neg_counterexamples.size());
            // auto neg_ce = neg_counterexamples[idx];
            auto neg_ce = *neg_counterexamples.begin();
            vector<set<vector<float>>> new_groups;
            for (auto& p : pos_groups)
            {
                if (genPredicate({neg_ce}, p, num_vars).clauses.empty())
                {
#ifdef DEBUG
                    std::cerr << "Split Groups call: " << iter_count++ << std::endl;
#endif
                    // neg_ce conflicts with p.
                    // p needs to be split.
                    split_group(p, neg_ce, pos_groups, new_groups);
                }
                else
                    new_groups.push_back(p);
            }
            pos_groups = new_groups;
            bool merged = false;
            for (auto& n : neg_groups)
            {
                n.insert(neg_ce);
                if (genPredicate(pos_groups, n, num_vars).clauses.empty() == false)
                {
                    merged = true;
                    break;
                }
                else
                {
                    n.erase(n.find(neg_ce));
                }
            }
            if (!merged)
            {
                neg_groups.push_back({neg_ce});
            }
        }
    }
    return guardPredicate();
}

float distance(const vector<float>& p1, const vector<float>& p2)
{
    // returns distance between p1 and p2 in the vector space using L2 norm.
    float dist = 0.0;
    for (int i = 0; i < p1.size(); i++)
    {
        dist += (p1[i]-p2[i])*(p1[i]-p2[i]);
    }
    return std::sqrt(dist);
}

affineFunction genAffineFunction(const map<vector<float>, float>& data, set<vector<float>>& covered,
                                 float threshold, int num_vars)
{
    // Find a point that is not covered.
    // Seed point.
    vector<float> x_p;
    float y_p;
    for (auto p : data)
    {
        if (covered.find(p.first) != covered.end()) continue;
        x_p = p.first;
        y_p = p.second;
        break;
    }

    // Find atleast N + 1 points around the seed point to learn a model.
    set<vector<float>> seed_points; 
    seed_points.emplace(x_p);
    for (int i = 0; i < num_vars + 2; i++)
    {
        // Find next point.
        vector<float> min_point = x_p;
        float min_dist = 0.0;
        for (auto& p : data)
        {
            if (seed_points.find(p.first) != seed_points.end()) continue;
            if (covered.find(p.first) != covered.end()) continue;
            float dist = distance(x_p, p.first);
            if (min_point == x_p || min_dist > dist)
            {
                min_point = p.first; min_dist = dist;
            }
        }
        seed_points.emplace(min_point);
    }

#ifdef CHECK
    // No function found!
    if (seed_points.size() < num_vars + 2)
        return affineFunction();
#endif

    set<vector<float>> points;
    vector<float> new_point;
    for (auto& p : seed_points)
    {
        new_point = p;
        new_point.emplace_back(data.at(p));
        points.emplace(new_point);
    }
    affineFunction l = trainModelUsingAlgLib(points, num_vars);

    while (true)
    {
        set<vector<float>> l_covered;
        for (auto p : data)
        {
            if (covered.find(p.first) != covered.end()) continue;
            if (abs(l.evaluate(p.first) - p.second) < threshold)
                l_covered.emplace(p.first);
        }
        if (points.size() >= l_covered.size())
           break;
        points.clear();
        for (auto& p : l_covered)
        {
            vector<float> new_point = p;
            new_point.push_back(data.at(p));
            points.emplace(new_point);
        }
        l = trainModelUsingAlgLib(points, num_vars);
    }
    return l;
}

piecewiseAffineModel learnModelFromData(const map<vector<float>, float>& data, float threshold)
{
    if (data.size() == 0) return piecewiseAffineModel();

    int num_vars = data.begin()->first.size();

    // learn affine functions.
    vector<affineFunction> affineFunctions;
    set<vector<float>> covered;

    while (covered.size() < data.size())
    {
        affineFunction l = genAffineFunction(data, covered, threshold, num_vars);
        if (l.coeff.empty()) break;
        for (auto p : data)
        {
            auto x = p.first;
            if (abs(l.evaluate(x) - p.second) < threshold)
                covered.insert(x);
        }
        affineFunctions.push_back(l);
    }

#ifdef DEBUG
    std::cerr << "Found " << affineFunctions.size() << " regions!" << std::endl;
#endif

    vector<int> cover_size;
    for (int i = 0; i < affineFunctions.size(); i++)
        cover_size.push_back(0);
    for (auto p : data)
    {
        for (int i = 0; i < affineFunctions.size(); i++)
        {
            if (abs(affineFunctions[i].evaluate(p.first) - p.second) < threshold)
                cover_size[i]++;
        }
    }

    piecewiseAffineModel model;

    for (int i = 0; i < affineFunctions.size() - 1; i++)
    {
        int j = 0;
        for (int k = 0; k < affineFunctions.size(); k++)
        {
            if (cover_size[k] == -1) continue;
            if (cover_size[j] == -1 || cover_size[j] > cover_size[k])
            {
                j = k;
            }
        }
        // Select j as the next region.
        set<vector<float>> positive_points;
        set<vector<float>> neg_points;
        for (auto& p : data)
        {
            bool pos_label = false, neg_label = false;
            bool already_labeled = false;
            for (int k = 0; k < affineFunctions.size(); k++)
            {
                if (cover_size[k] == -1 &&
                    abs(affineFunctions[k].evaluate(p.first) - p.second) < threshold)
                    already_labeled = true;
                    break;
            }
            if (already_labeled) break;

            if (abs(affineFunctions[j].evaluate(p.first) - p.second) < threshold)
            {
                pos_label = true;
            }
            for (int k = 0; k < affineFunctions.size(); k++)
            {
                if (cover_size[k] == -1 || k == j) continue;
                if (abs(affineFunctions[k].evaluate(p.first) - p.second) < threshold)
                {
                    neg_label = true;
                    break;
                }
            }
            if (pos_label && !neg_label) 
            {
                positive_points.emplace(p.first);
            }
            if (neg_label && !pos_label)
            {
                neg_points.emplace(p.first);
            }
        }
#ifdef DEBUG
        std::cerr << "Generating afffine guard for region" << j << std::endl;
        std::cerr << "Number of positive points: " << positive_points.size()
                  << ", number of negative points: " << neg_points.size() << std::endl;
#endif
        guardPredicate g = genGuard(positive_points, neg_points,
                                    num_vars);
        piecewiseAffineModel::region r;
        r.f = affineFunctions[i];
        r.g = g;
        model.regions.push_back(r);

        // set cover_size to -1, so it is not selected in the future.
        cover_size[j] = -1;
    }
    // Add the remaining region.
    int j = 0;
    for (; cover_size[j] == -1; j++);
    piecewiseAffineModel::region r;
    r.f = affineFunctions[j];
    r.g = true_predicate(data.begin()->first.size());
    model.regions.push_back(r); 

    return model;
}
