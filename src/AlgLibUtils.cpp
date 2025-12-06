#include "AlgLibUtils.hpp"
#include "utils.hpp"

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

    // Convert problem to standard form.
    std::vector<float> scale_vec;
    for (int i = 0; i < num_vars; i++)
    {
        float avg_absval = 0.0;
        for (int j = 0; j < num_constraints; j++)
        {
            avg_absval += abs(a[j][i]);
        }
        avg_absval = avg_absval/num_constraints;
        if (avg_absval < 1.0) avg_absval = 1.0;

        scale_vec.push_back(avg_absval);
    }

    // Compute maximum for bounds setting.
    float max = 1.0;
    for (int i = 0; i < num_vars; i++)
        for (int j = 0; j < num_constraints; j++)
            if (max < abs(a[j][i])) max = abs(a[j][i]);

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
    for (int j = 0; j < num_vars; j++)
    {
        s[j] = 1.0/scale_vec[j];
    }
    s[num_vars] = 1.0;
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

affineFunction findAffineFunctionPassingThroughCEOnlyAlternate(const set<vector<float>>& g, const vector<float>& ce)
{
    // Using AlgLib.
    for (int i = 0; i < NUM_ITERATIONS; i++)
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

affineFunction trainModelUsingAlgLib(set<vector<float>>& points, int num_vars)
{
    alglib::real_2d_array xy;
    xy.setlength(points.size(), num_vars + 1);
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
        alglib::lrbuild(xy, points.size(), num_vars, model, rep);
        alglib::lrunpack(model, c, nvars);
    }
    catch(alglib::ap_error alglib_exception)
    {
        printf("ALGLIB exception with message '%s'\n", alglib_exception.msg.c_str());
        return f;
    }
    for (int i = 0; i < num_vars + 1; i++)
    {
        f.coeff.push_back(c[i]);
    }
    return f;
}
