#pragma once

#include "PieceWiseAffineModel.hpp"
#include <string>
#include <map>
#include <vector>

// Utilities for loading data and dumping the model.

std::map<std::vector<float>, float> loadData(const std::string& path);

std::string vectorString(const std::vector<float>& v);

void outputPredicate(const predicate& g);
void outputGuardPredicate(const guardPredicate& g);
void outputAffineFunction(const affineFunction& f);
void outputModel(const piecewiseAffineModel& model);

// Utilities for simple predicates.
guardPredicate true_predicate(int n);
guardPredicate false_predicate(int n);

// Utilities for distance computation in vector space.
float distance(const std::vector<float>& p1, const std::vector<float>& p2);
