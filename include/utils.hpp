#pragma once

#include "PieceWiseAffineModel.hpp"
#include <string>
#include <map>
#include <vector>

// Utilities for loading data and dumping the model.

std::map<std::vector<float>, float> loadData(const std::string& path);

void outputAffineFunction(affineFunction& f);
void outputModel(piecewiseAffineModel& model);
