#include "Solvers.hpp"
#include "utils.hpp"

#include <set>
#include <functional>
#include <iostream>
#include "dataanalysis.h"
#include "optimization.h"

// #define DEBUG

using namespace std;

predicate genPredicateUsingAlgLib(const set<vector<float>>& p, const set<vector<float>>& n,
                                  int num_vars)
{
    // Set up an min LP solver.

#ifdef DEBUG
    std::cerr << "Solving predicate for points." << std::endl;

#endif
    int num_constraints = p.size() + n.size();
    // Convert problem to standard form.
    float max = 1.0;
    for (auto &x: p)
        for (auto c : x)
            if (max < abs(c)) max = abs(c);
    for (auto &x: n)
        for (auto c : x)
            if (max < abs(c)) max = abs(c);

    // cost is to maximize the distance from all points (which is quadratic generally), we set it to 0, to find
    // a feasible solution for now.
    alglib::real_2d_array a;
    a.setlength(num_constraints, num_vars + 1);

    // Initialize constraints array.
    int i = 0;
    for (auto& x : p)
    {
        for (int j = 0; j < x.size(); j++)
        {
            a[i][j] = x[j];
        }
        a[i][x.size()] = 1;
        i++;
    }
    for (auto& x : n)
    {
        for (int j = 0; j < x.size(); j++)
        {
            a[i][j] = x[j];
        }
        a[i][x.size()] = 1;
        i++;
    }

    // Initialize constraint bounds
    alglib::real_1d_array au, al;
    au.setlength(num_constraints);
    al.setlength(num_constraints);
    i = 0;
    for (int j = 0; j < p.size(); j++)
    {
        al[i] = 0.001;
        au[i] = alglib::fp_posinf;
        i++;
    }
    for (int j = 0; j < n.size(); j++)
    {
        al[i] = alglib::fp_neginf;
        au[i] = -0.001;
        i++;
    }

    // Initialize variable bounds.
    alglib::real_1d_array bu, bl;
    bu.setlength(num_vars + 1);
    bl.setlength(num_vars + 1);
    for (int j = 0; j < num_vars; j++)
    {
        bl[j] = -1.0;
        bu[j] = 1.0;
    }
    bl[num_vars] = -num_vars*max;
    bu[num_vars] = num_vars*max; // This bound is on the constant which is scaled to shift the equilibrium.

    alglib::real_1d_array s;
    s.setlength(num_vars + 1);
    for (int j = 0; j < num_vars + 1; j++)
    {
        s[j] = 1.0;
    }
    alglib::real_1d_array c;
    c.setlength(num_vars + 1);
    for (int j = 0; j < num_vars + 1; j++)
    {
        c[j] = 0.0;
    }
    alglib::minlpstate state;
    alglib::real_1d_array x;
    alglib::minlpreport rep;

    predicate pred;
    
    try {
        alglib::minlpcreate(num_vars + 1, state);
        // alglib::minlpsetlc(state, a, ct);
        alglib::minlpsetcost(state, c);
        alglib::minlpsetscale(state, s);
        alglib::minlpsetbc(state, bl, bu);
        alglib::minlpsetlc2dense(state, a, al, au, num_constraints);
        // alglib::minlpsetalgoipm(state);
        alglib::minlpsetalgodss(state, 0.00001);
        // alglib::trace_file("DSS,PREC.F6", "trace.log");
        alglib::minlpoptimize(state);
        alglib::minlpresults(state, x, rep);
        if (rep.terminationtype < 0)
        {
            // Infeasible constraint system.
#ifdef DEBUG
            std::cerr << "Error! Could not construct the predicate." << std::endl;
            std::cerr << "Pos Points: ";
            for (auto &x : p)
            {
                std::cerr << "(";
                for (auto j : x)
                    std::cerr << j << ",";
                std::cerr << "),";
            }
            std::cerr << std::endl;
            std::cerr << "Neg Points: ";
            for (auto &x : n)
            {
                std::cerr << "(";
                for (auto j : x)
                    std::cerr << j << ",";
                std::cerr << "),";
            }
            std::cerr << std::endl;
            std::cerr << "Result type: " << rep.terminationtype << std::endl;
#endif
            return pred;
        }
    }
    catch(alglib::ap_error alglib_exception)
    {
        printf("ALGLIB exception with message '%s'\n", alglib_exception.msg.c_str());
        return pred;
    }

    for (int i = 0; i < num_vars; i++)
    {
        pred.coeff.push_back(x[i]);
    }
    pred.coeff.push_back(x[num_vars]);
    return pred;
}

affineFunction findAffineFunctionPassingThroughCEOnlyAlternate(const set<vector<float>>& g, const vector<float>& ce)
{
    // Using AlgLib.
    while (true)
    {
        affineFunction f;
        float ce_val = 0.0;
        for (int j = 0; j < ce.size(); j++)
        {
            float c = alglib::randomreal();
            ce_val = ce[j]*c;
            f.coeff.push_back(c);
        }
        f.coeff.push_back(-ce_val);
        bool function_found = true;
        for (auto &p : g)
        {
            if (abs(f.evaluate(p)) < 0.001)
            {
                function_found = false;
                break;
            }
        }
        if (function_found)
            return f;
    }
    return affineFunction();
}

affineFunction findAffineFunctionPassingThroughCEOnly(const set<vector<float>>& g, const vector<float>& ce)
{
    // Using AlgLib.
    // We pose this as a linear programming problem where the function output on ce is 0, while
    // we add non-linear constraints specifying the squared distance of points in g is positive.
    // The constraints are as follows:
    //     x1.ce1 + x2.ce2 + ... xn.cen + x0 = 0.
    //     for each point p in g,
    //         (x1.p1 + x2.p2 + ... + xn.pn + x0)^2 > 1.
    // We note that the squared distance is used to ensure that the sign of the distance is
    // ignored while computing the function.

    affineFunction f;
    alglib::real_1d_array x1;
    try
    {
        double epsx  = 0.000001;
        alglib::ae_int_t maxits = 0;
        alglib::minnlcstate state;
        alglib::real_1d_array s;
        s.setlength(ce.size() + 1);
        for (int j = 0; j < ce.size() + 1; j++)
        {
            s[j] = 1;
        }

        alglib::real_1d_array x0;
        x0.setlength(ce.size() + 1);
        float ce_val = 0.0;
        for (int j = 0; j < ce.size(); j++)
        {
            x0[j] = 1;
            ce_val -= ce[j];
        }
        x0[ce.size()] = ce_val;

        alglib::minnlccreate(ce.size() + 1, x0, state);
        alglib::minnlcsetcond(state, epsx, maxits);
        alglib::minnlcsetscale(state, s);

        alglib::minnlcsetalgoorbit(state, 0, 0);
        alglib::real_1d_array nl;
        nl.setlength(g.size());
        for (int j = 0; j < g.size(); j++)
        {
            nl[j] = 1;
        }
        alglib::real_1d_array nu;
        nu.setlength(g.size());
        for (int j = 0; j < g.size(); j++)
        {
            nu[j] = alglib::fp_posinf;
        }
        alglib::minnlcsetnlc2(state, nl, nu);

        alglib::real_2d_array a;
        a.setlength(1, ce.size() + 1);
        for (int j = 0; j < ce.size(); j++)
        {
            a[0][j] = ce[j];
        }
        a[0][ce.size()] = 1;
        alglib::real_1d_array al = "[0]";
        alglib::real_1d_array au = "[0]";
        alglib::minnlcsetlc2dense(state, a, al, au, 1);

        alglib::minnlcreport rep;
        auto func = [&g](const alglib::real_1d_array &x, alglib::real_1d_array& fi, void* ptr)
            {
                int i = 0;
                fi[i++] = 0.0; // Target function.
                for (auto& p : g)
                {
                    // (p.x)^2
                    float val = 0.0;
                    for (int k = 0; k < p.size(); k++)
                    {
                        val += p[k]*x[k];
                    }
                    val += x[p.size()];
                    fi[i++] = val*val;
                }
            };
        std::function<void (const alglib::real_1d_array &, alglib::real_1d_array &, void *)> funcObject = func;
        std::cerr << "Starting non-linear optimization...." << std::endl;
        alglib::minnlcoptimize(state, (void (*)(const alglib::real_1d_array &, alglib::real_1d_array &, void *))&funcObject);
        std::cerr << "Non-linear optimization completed...." << std::endl;
        alglib::minnlcresults(state, x1, rep);
    }
    catch(alglib::ap_error alglib_exception)
    {
        printf("ALGLIB exception with message '%s'\n", alglib_exception.msg.c_str());
        return f;
    }
    for (int i = 0; i < ce.size() + 1; i++)
    {
        f.coeff.push_back(x1[i]);
    }

    return f;
}

void split_group(const set<vector<float>>& g, const vector<float>& ce, vector<set<vector<float>>>& groups,
                 vector<set<vector<float>>>& new_groups)
{
    // Splits group g into two groups, such that the counterexample ce can be accomodated.
    // Updates groups with the new groups while erasing old group g.

    set<vector<float>> g_less, g_more;
    while (true)
    {
        // Find an affine function f, so that f(ce) = 0, f(p) != 0 for all p in g.
        affineFunction f = findAffineFunctionPassingThroughCEOnlyAlternate(g, ce);

        g_less.clear();
        g_more.clear();
        bool found = true;
        for (auto p: g)
        {
            if (f.evaluate(p) > 0)
                g_more.emplace(p);
            else if (f.evaluate(p) < 0)
                g_less.emplace(p);
            else
            {
                found = false;
                break;
            }
        }
        // Splitting not found.
        if (!found) continue;
        if (g_more.size() > 0 && g_less.size() > 0)
            break;
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

affineFunction trainModelUsingAlgLib(set<vector<float>>& points)
{
    alglib::real_2d_array xy;
    xy.setlength(points.size(), points.begin()->size());
    auto it = points.begin();
    for (int i = 0; it != points.end(); i++)
    {
        auto p = *it;
        for (int j = 0; j < p.size(); j++)
        {
            xy[i][j] = p[j];
        }
        it++;
    }
    alglib::ae_int_t nvars;
    alglib::linearmodel model;
    alglib::lrreport rep;
    alglib::real_1d_array c;

    affineFunction f;
    try
    {
        alglib::lrbuild(xy, points.size(), points.begin()->size() - 1, model, rep);
        alglib::lrunpack(model, c, nvars);
    }
    catch(alglib::ap_error alglib_exception)
    {
        printf("ALGLIB exception with message '%s'\n", alglib_exception.msg.c_str());
        return f;
    }
    for (int i = 0; i < points.begin()->size(); i++)
    {
        f.coeff.push_back(c[i]);
    }
    return f;
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
    affineFunction l = trainModelUsingAlgLib(points);

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
        l = trainModelUsingAlgLib(points);
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
