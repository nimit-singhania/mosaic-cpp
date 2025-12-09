#include "Solvers.hpp"
#include "AlgLibUtils.hpp"
#include "utils.hpp"

#include <iostream>

// #define DEBUG
#define SIMPLIFY
#define UNIGRAM_PREDICATE
// #define NORMALIZE

#define MAX_ITERATIONS 10

using namespace std;

void genPredicateError(const vector<float>& x, const set<vector<float>>& p,
                       const set<vector<float>>& n, const predicate& pred)
{
    std::cerr << "Predicate not satisfied for point: " << vectorString(x) << "!" << std::endl;
    std::cerr << "Set of input points: " << std::endl;
    for (auto & e : p)
    {
        std::cerr << vectorString(e) << ", ";
    }
    std::cerr <<std::endl;
    for (auto & e : n)
    {
        std::cerr << vectorString(e) << ", ";
    }
    std::cerr <<std::endl;
    std::cerr << "Predicate: ";
    outputPredicate(pred);
    std::cerr <<std::endl;
}

guardPredicate genPredicate(const set<vector<float>>& p,
                            const set<vector<float>>& n,
                            int num_vars)
{
    bool found = false;
    predicate pred;

    if (p.size() == 0) return false_predicate(num_vars);
    if (n.size() == 0) return true_predicate(num_vars);

#ifdef UNIGRAM_PREDICATE
    // Try simple heuristics: xi >= n for satisfiability.
    for (int i = 0; i < num_vars; i++)
    {
        float max_p = p.begin()->at(i), min_p = p.begin()->at(i);
        float max_n = n.begin()->at(i), min_n = n.begin()->at(i);
        for (auto & x: p)
        {
            if (max_p < x[i]) max_p = x[i];
            if (min_p > x[i]) min_p = x[i];
        }

        for (auto & x: n)
        {
            if (max_n < x[i]) max_n = x[i];
            if (min_n > x[i]) min_n = x[i];
        }

        if (max_p >= min_n && max_n >= min_p) continue;

        for (int j = 0; j < i; j++) pred.coeff.push_back(0.0);
        if (min_p >= max_n)
        {
            pred.coeff.push_back(1.0);
            for (int j = i+1; j < num_vars; j++) pred.coeff.push_back(0.0);
            pred.coeff.push_back(-(min_p + max_n)/2);
        }
        else
        {
            pred.coeff.push_back(-1.0);
            for (int j = i+1; j < num_vars; j++) pred.coeff.push_back(0.0);
            pred.coeff.push_back((min_n + max_p)/2);
        }
        found = true;
        break;
    }
#endif

#ifdef NORMALIZE
    // Another heuristic: xi + xj >= n.
    if (!found)
        for (int i = 0; i < num_vars; i++)
        {
            for (int j = i+1; j < num_vars; j++)
            {
                float max_p = p.begin()->at(i) + p.begin()->at(j);
                float min_p = p.begin()->at(i) + p.begin()->at(j);
                float max_n = n.begin()->at(i) + n.begin()->at(j);
                float min_n = n.begin()->at(i) + n.begin()->at(j);

                for (auto & x : p)
                {
                    if (max_p < x[i] + x[j]) max_p = x[i] + x[j];
                    if (min_p > x[i] + x[j]) min_p = x[i] + x[j];
                }
                for (auto & x : n)
                {
                    if (max_n < x[i] + x[j]) max_n = x[i] + x[j];
                    if (min_n > x[i] + x[j]) min_n = x[i] + x[j];
                }
                if (max_p >= min_n && max_n >= min_p) continue;

                for (int k = 0; k < num_vars; k++) pred.coeff.push_back(0.0);
                if (min_p >= max_n)
                {
                    pred.coeff[i] = 1.0;
                    pred.coeff[j] = 1.0;
                    pred.coeff.push_back(-(min_p + max_n)/2);
                }
                else
                {
                    pred.coeff[i] = -1.0;
                    pred.coeff[j] = -1.0;
                    pred.coeff.push_back((min_n + max_p)/2);
                }
                found = true;
                break;
            }
            if (found) break;
        }

#endif

    if (!found)
        pred = genPredicateUsingAlgLib(p, n, num_vars);

    if (pred.coeff.empty()) return guardPredicate();

    // Check predicate.
#ifdef CHECK
    for (auto & x : p)
    {
        if (pred.evaluate(x) == false)
        {
            genPredicateError(x, p, n, pred);
            return guardPredicate();
        }
    }
    for (auto & x : n)
    {
        if (pred.evaluate(x) == true)
        {
            genPredicateError(x, p, n, pred);
            return guardPredicate();
        }
    }
#endif

    guardPredicate g;
    guardPredicate::orPredicate o;
    o.terms.push_back(pred);
    g.clauses.push_back(o);
    return g;
}

guardPredicate genPredicate(const vector<set<vector<float>>>& pos_groups,
                            const set<vector<float>>& n, int num_vars)
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

guardPredicate genPredicate(const vector<set<vector<float>>>& pos_groups,
                            const vector<set<vector<float>>>& neg_groups,
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

    // Manual heuristics to split the set.
    // Split by axis.
    for (int i = 0; i < ce.size(); i++)
    {
        g_less.clear();
        g_more.clear();
        bool infeasible = false;
        for (auto &p : g)
        {
            if (p[i] < ce[i]) g_less.emplace(p);
            else if (p[i] > ce[i]) g_more.emplace(p);
            else
            {
                infeasible = true;
                break;
            }
        }
        if (infeasible) continue;

        // Splitting found along axis i.
        found = true;
        break;
    }
#ifdef NORMALIZE
    if (!found)
        for (int i = 0; i < ce.size(); i++)
        {
            for (int j = i + 1; j < ce.size(); j++)
            {
                g_less.clear(); g_more.clear();
                bool infeasible = false;
                for (auto & x : g)
                {
                    if (x[i] + x[j] < ce[i] + ce[j]) g_less.emplace(x);
                    else if (x[i] + x[j] > ce[i] + ce[j]) g_more.emplace(x);
                    else
                    {
                        infeasible = true;
                        break;
                    }
                }
                if (infeasible) continue;
                // Splitting found along axis i + j.
                found = true;
                break;
            }
            if (found) break;
        }
#endif

    // Try global splitting heuristic.
    if (!found)
    {
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

    if (!found)
    {
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
    }

#ifdef DEBUG
    std::cerr << "Split Group Result: " << std::endl;
    for (auto &p : g_less)
    {
        std::cerr << vectorString(p) << ", ";
    }
    std::cerr << std::endl;
    for (auto &p : g_more)
    {
        std::cerr << vectorString(p) << ", ";
    }
#endif
    // auto it = std::find(groups.begin(), groups.end(), g);
    // if (it != groups.end())
    //    groups.erase(it);

    if (found)
    {
        new_groups.push_back(g_more);
        new_groups.push_back(g_less);
    }
}

guardPredicate simplify(const vector<set<vector<float>>>& pos_groups,
                        const vector<set<vector<float>>>& neg_groups,
                        int num_vars)
{
    // Try merging pos_groups, if extraneous groups are formed.
    vector<set<vector<float>>> simplified_pos_groups, simplified_neg_groups;
    simplified_pos_groups.push_back(pos_groups[0]);
    for (int i = 1; i < pos_groups.size(); i++)
    {
        bool merged = false;
        // Check if pos_group[i] can be merged with any of the simplified groups.
        for (int j = 0; j < simplified_pos_groups.size(); j++)
        {
            auto merged_group = pos_groups[i];
            for (auto & x : simplified_pos_groups[j])
                merged_group.emplace(x);
            if (genPredicate(neg_groups, merged_group, num_vars).clauses.empty())
                continue;
            // Merging is feasible.
            merged = true;
            for (auto & x: pos_groups[i])
                simplified_pos_groups[j].emplace(x);
            break;
        }
        if (!merged)
            simplified_pos_groups.push_back(pos_groups[i]);
    }
    simplified_neg_groups.push_back(neg_groups[0]);
    for (int i = 1; i < neg_groups.size(); i++)
    {
        bool merged = false;
        // Check if neg_group[i] can be merged with any of the simplified groups.
        for (int j = 0; j < simplified_neg_groups.size(); j++)
        {
            auto merged_group = neg_groups[i];
            for (auto & x : simplified_neg_groups[j])
                merged_group.emplace(x);
            if (genPredicate(pos_groups, merged_group, num_vars).clauses.empty())
                continue;
            // Merging is feasible.
            merged = true;
            for (auto & x: neg_groups[i])
                simplified_neg_groups[j].emplace(x);
            break;
        }
        if (!merged)
            simplified_neg_groups.push_back(neg_groups[i]);
    }
    return genPredicate(simplified_pos_groups, simplified_neg_groups, num_vars);
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
    while (iter_count <= MAX_ITERATIONS)
    {
#ifdef DEBUG
        // std::cerr << "Iteration " << iter_count++ << std::endl;
#endif
        guardPredicate g = genPredicate(pos_groups, neg_groups, num_vars);
        vector<vector<float>> counterexamples;
        for (auto& p : pos_points)
        {
            if (g.evaluate(p) == false)
            {
                counterexamples.emplace_back(p);
            }
        }
        for (auto& p : neg_points)
        {
            if (g.evaluate(p) == true)
                counterexamples.emplace_back(p);
        }
        if (counterexamples.empty() || iter_count == MAX_ITERATIONS)
        {
#ifdef SIMPLIFY
            g = simplify(pos_groups, neg_groups, num_vars);
#endif
            return g;
        }

        auto ce = *counterexamples.begin();
        if (g.evaluate(ce) == false)
        {
            // Process positive counterexample.
            vector<set<vector<float>>> new_groups;
            for (auto& n : neg_groups)
            {
                if (genPredicate({ce}, n, num_vars).clauses.empty())
                {
                    // ce conflicts with n.
                    // n needs to be split.
#ifdef DEBUG
                    std::cerr << "Split Groups call: " << iter_count << std::endl;
#endif
                    split_group(n, ce, neg_groups, new_groups);
                    iter_count++;
                }
                else
                    new_groups.push_back(n);
            }
            neg_groups = new_groups;
            bool merged = false;
            for (auto& p : pos_groups)
            {
                p.insert(ce);
                if (genPredicate(neg_groups, p, num_vars).clauses.empty() == false)
                {
                    merged = true;
                    break;
                }
                else
                {
                    p.erase(p.find(ce));
                }
            }

            if (!merged)
            {
                pos_groups.push_back({ce});
            }
        }
        else
        {
            // Process negative counterexample.
            vector<set<vector<float>>> new_groups;
            for (auto& p : pos_groups)
            {
                if (genPredicate({ce}, p, num_vars).clauses.empty())
                {
#ifdef DEBUG
                    std::cerr << "Split Groups call: " << iter_count << std::endl;
#endif
                    // ce conflicts with p.
                    // p needs to be split.
                    split_group(p, ce, pos_groups, new_groups);
                    iter_count++;
                }
                else
                    new_groups.push_back(p);
            }
            pos_groups = new_groups;
            bool merged = false;
            for (auto& n : neg_groups)
            {
                n.insert(ce);
                if (genPredicate(pos_groups, n, num_vars).clauses.empty() == false)
                {
                    merged = true;
                    break;
                }
                else
                {
                    n.erase(n.find(ce));
                }
            }
            if (!merged)
            {
                neg_groups.push_back({ce});
            }
        }
    }
    return guardPredicate();
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
    for (int i = 0; i < num_vars + 1; i++)
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

std::vector<float> normalizeInput(const map<vector<float>, float>& data,
                                  map<vector<float>, float>& normalized_data,
                                  int num_vars)
{
    std::vector<float> scale_vec;
    for (int i = 0; i < num_vars; i++)
    {
        scale_vec.push_back(1.0);
    }
    if (data.size() == 0) return scale_vec;

#ifdef NORMALIZE
    for (int i = 0; i < num_vars; i++)
    {
        // Normalize ith feature.
        float feature_avg = 0.0;
        for (auto& p : data)
        {
            feature_avg += p.first[i]/data.size();
        }
        scale_vec[i] = feature_avg;
    }
#endif
    for (auto &p : data)
    {
        std::vector<float> normalized;
        for (int i = 0; i < num_vars; i++)
            normalized.push_back(p.first[i]/scale_vec[i]);

        normalized_data.emplace(normalized, p.second);
    }
    return scale_vec;
}

piecewiseAffineModel learnModelFromData(const map<vector<float>, float>& data, float threshold)
{
    piecewiseAffineModel model;

    if (data.size() == 0) return model;

    int num_vars = data.begin()->first.size();

    // Normalize input.
    map<vector<float>, float> normalized_data;
    auto scale_vec = normalizeInput(data, normalized_data, num_vars);
    model.scale_vec = scale_vec;

    // learn affine functions.
    vector<affineFunction> affineFunctions;
    set<vector<float>> covered;

    while (covered.size() < normalized_data.size())
    {
        affineFunction l = genAffineFunction(normalized_data, covered, threshold, num_vars);
        if (l.coeff.empty()) break;
        for (auto p : normalized_data)
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
    for (auto p : normalized_data)
    {
        for (int i = 0; i < affineFunctions.size(); i++)
        {
            if (abs(affineFunctions[i].evaluate(p.first) - p.second) < threshold)
                cover_size[i]++;
        }
    }

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
        for (auto& p : normalized_data)
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
            if (already_labeled) continue;

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
    r.g = true_predicate(num_vars);
    model.regions.push_back(r); 

    return model;
}
