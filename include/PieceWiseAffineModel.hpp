#pragma once

#include <vector>

/* Affine function: An affine function represents a linear combination of input
 * values to generate the output value. Let the coefficients of linear
 * combination be `coeff`. Given an input vector `x`, the output of the function
 * is:
 *     `f(x) = coeff[0]*x[0] + coeff[1]*x[1] + ... + coeff[n]*x[n] + coeff[n+1]`
 * Note, the vector `coeff` has one additional value that is the constant.
 */
struct affineFunction
{
    std::vector<float> coeff;

    float evaluate(const std::vector<float>& input)
    {
        float result = 0;
        for (int i = 0; i < input.size(); i++)
        {
            result += coeff[i]*input[i];
        }
        result += coeff[input.size()];
        return result;
    }
};

/* Predicate: A predicate is an affine inequality. For simplicity we represent
 * an affine inequality as:
 *     p(x) = f(x) >= 0, where f(x) is an affine function.
 * Note that this only represents strict inequality.
 */
struct predicate
{
    std::vector<float> coeff;
    bool evaluate(const std::vector<float>& input)
    {
        float result = 0;
        for (int i = 0; i < input.size(); i++)
        {
            result+= coeff[i]*input[i];
        }
        result += coeff[input.size()];
        return result >=  0.0;
    }
};

/* Guard Predicate: A Boolean predicate is a boolean combination of simple
 * affine predicates. It consists of multiple clauses that are conjuncted
 * together:
 *   p(x) = c1(x) AND c2(x) AND ... AND cM(x).
 * Each clause is a disjunction of simple affine predicates:
 *  ci(x) = pi1(x) OR pi2(x) OR ... OR piN(x).
 */
struct guardPredicate
{
    struct orPredicate
    {
        std::vector<predicate> terms;
        bool evaluate(const std::vector<float>& input)
        {
            for (auto& t : terms)
            {
                if(t.evaluate(input))
                    return true;
            }
            return false;
       }
    };

    std::vector<orPredicate> clauses;
    bool evaluate(const std::vector<float>& input)
    {
        for (auto& c: clauses)
        {
            if (!c.evaluate(input))
                return false;
        }
        return true;
    }
};

/* Piecewise affine model: A piecewise affine model consists of regions, where
 * each region is associated with an affine function and a guard predicate. The
 * regions are also called modes which correspond to modes of operations which
 * are switched to define multiple modes. Within each mode, the dynamics of the
 * model are represented by an affine function while modes are separated by
 * guards associated with each region. Only one of the guard predicates is true
 * for any input point in the input space.
 */
struct piecewiseAffineModel
{
    struct region
    {
        affineFunction f;
        guardPredicate g;
    };
    std::vector<region> regions;

    float evaluate(const std::vector<float>& input)
    {
        for (auto& r : regions)
        {
            if (r.g.evaluate(input))
            {
                return r.f.evaluate(input);
            }
        }
        return 0.0; // Not reached.
    }
};
