#pragma once

#include "PieceWiseAffineModel.hpp"
#include <set>

// Learn a predicate (x*c + c0 >= 0) that separates points in p from points in n,
// i.e. predicate is true on points in p, and false on points in n.
predicate genPredicateUsingAlgLib(const std::set<std::vector<float>>& p,
                                  const std::set<std::vector<float>>& n,
                                  int num_vars);

// Find an affine function, such that the point ce evaluates to value 0, while points in
// g evaluate to non-zero value.
affineFunction findAffineFunctionPassingThroughCEOnly(const std::set<std::vector<float>>& g,
                                                      const std::vector<float>& ce);

// Find an affine function, such that the point ce evaluates to value 0, while points in
// g evaluate to non-zero value.
// Alternate implementation using simple heuristics to find the affine function.
affineFunction findAffineFunctionPassingThroughCEOnlyAlternate(const std::set<std::vector<float>>& g,
                                                               const std::vector<float>& ce);

// Trains a model that fits a linear regression linear function on points given.
// Note, the points also include the output as the last dimension.
// Hence, the points are of dimensions (num_vars + 1).
affineFunction trainModelUsingAlgLib(std::set<std::vector<float>>& points, int num_vars);
