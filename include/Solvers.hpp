#pragma once

#include "PieceWiseAffineModel.hpp"
#include <algorithm>
#include <map>

/* We would like to learn a piecewise affine model that can represent the dynamics
 * of a system. The piecewise affine model is amenable to analysis via formal methods
 * and thereby is a convenient representation of real-world complex systems. The model
 * is flexible and can represent non-linearity in the system dynamics via different
 * modes of operations. The modes or regions represent more coherent dynamics that are
 * easy to analyze, interpolate and build controllers for. These characteristics make
 * piecewise affine models desirable as the model for representing system dynamics.
 *
 * This file consists of methods to construct piecewise affine models from system
 * data. System data can consist of multiple inputs:
 * (1) A collection of input-output data points (this is akin to machine learning systems
 *     and ML methods can be used to train the models).
 * (2) Trajectory data from multiple different trajectories within the system. This data
 *     is inherently sequential, but requires careful modelling via Piecewise affine
 *     models.
 */

extern int num_splits;

piecewiseAffineModel learnModelFromData(const std::map<std::vector<float>, float>& data, float threshold);

piecewiseAffineModel learnModelFromTrajectories(std::vector<std::vector<std::pair<float,float>>>& trajectories, float threshold);
