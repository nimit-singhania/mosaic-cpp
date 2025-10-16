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
    vector<float> coeff;

    float compute(vector<float>& input)
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
 *     p(x) = f(x) <= 0, where f(x) is an affine function.
 * Note that this only represents strict inequality.
 */
struct guardPredicate
{
    vector<float> coeff;
    bool evaluate(vector<float>& input)
    {
        float result = 0;
        for (int i = 0; i < input.size(); i++)
        {
            result+= coeff[i]*input[i];
        }
        result += coeff[input.size()];
        return result <  0.0;
    }
};
