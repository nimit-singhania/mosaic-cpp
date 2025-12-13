#pragma once

#include "PieceWiseAffineModel.hpp"
#include <string>
#include <map>
#include <vector>

#include "boost/json.hpp"

// Utilities for loading data and dumping the model.

std::map<std::vector<float>, float> loadData(const std::string& path);

std::string vectorString(const std::vector<float>& v);

void outputPredicate(const predicate& g,
                     const std::vector<float>& scale_vec = std::vector<float>());
void outputGuardPredicate(const guardPredicate& g,
                          const std::vector<float>& scale_vec = std::vector<float>());
void outputAffineFunction(const affineFunction& f,
                          const std::vector<float>& scale_vec = std::vector<float>());
void outputModel(const piecewiseAffineModel& model);

// JSON utilities.
boost::json::object outputModelJSON(const piecewiseAffineModel& model);
piecewiseAffineModel loadModelJSON(const std::string& model_path);
piecewiseAffineModel parseModelJSON(const boost::json::object& model_json);

// Utilities for simple predicates.
guardPredicate true_predicate(int n);
guardPredicate false_predicate(int n);

// Utilities for distance computation in vector space.
float distance(const std::vector<float>& p1, const std::vector<float>& p2);

// Utilities for reading input configuration.
std::map<std::string, std::string> read_configuration(int argc, char** argv);
