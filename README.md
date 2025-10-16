# mosaic-cpp

Mosaic is a tool to learn piecewise affine models from input-output data. A piecewise affine model consists of multiple regions each region guarded by a guarded predicate and an affine function that defines the dynamics of the model in the region. To compute the model output for an input, the region for the input is located, followed by computation of the affine function on the input. The piecewise nature enables the model to represent non-linear behavior while affine dynamics provide explanability and ease of model checking.


