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
        if (coeff.empty()) return result;
        for (int i = 0; i < input.size(); i++)
        {
            result += coeff[i]*input[i];
        }
        result += coeff[input.size()];
        return result;
    }

    bool operator==(const affineFunction& f) const
    {
        return this->coeff == f.coeff;
    }

    bool operator!=(const affineFunction& f) const
    {
        return this->coeff != f.coeff;
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

    bool operator==(const predicate& p) const
    {
        return this->coeff == p.coeff;
    }

    bool operator!=(const predicate& p) const
    {
        return this->coeff != p.coeff;
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
       bool operator==(const orPredicate& p) const
       {
           return this->terms == p.terms;
       }
       bool operator!=(const orPredicate& p) const
       {
           return this->terms != p.terms;
       }
    };

    std::vector<orPredicate> clauses;
    bool evaluate(const std::vector<float>& input)
    {
        if (clauses.empty()) return false;
        for (auto& c: clauses)
        {
            if (!c.evaluate(input))
                return false;
        }
        return true;
    }
    bool operator==(const guardPredicate& p) const
    {
        return this->clauses == p.clauses;
    }
    bool operator!=(const guardPredicate& p) const
    {
        return this->clauses != p.clauses;
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

    // Scale vector to normalize the input for the normalization that is applied during
    // training.
    std::vector<float> scale_vec;

    float evaluate(const std::vector<float>& input)
    {
        if (regions.size() == 0 || scale_vec.size() != input.size()) return 0.0;

        std::vector<float> normalized_input;
        for (int i = 0; i < input.size(); i++)
            normalized_input.push_back(input.at(i)*scale_vec.at(i));

        for (auto& r : regions)
        {
            if (r.g.evaluate(normalized_input))
            {
                return r.f.evaluate(normalized_input);
            }
        }
        return 0.0;
    }
    bool operator==(const piecewiseAffineModel& m) const
    {
        if (scale_vec != m.scale_vec) return false;

        if (m.regions.size() != this->regions.size()) return false;
        for (int i = 0; i < this->regions.size(); i++)
        {
            if (this->regions.at(i).f != m.regions.at(i).f) return false;
            if (this->regions.at(i).g != m.regions.at(i).g) return false;
        }
        return true;
    }
    bool operator!=(const piecewiseAffineModel& m) const
    {
        return !(*this == m);
    }
};
