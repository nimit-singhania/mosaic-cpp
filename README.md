# mosaic-cpp

Mosaic is a tool to learn piecewise affine models from input-output data. A piecewise affine model consists of multiple regions each region guarded by a guarded predicate and an affine function that defines the dynamics of the model in the region. To compute the model output for an input, the region for the input is located, followed by computation of the affine function on the input. The piecewise nature enables the model to represent non-linear behavior while affine dynamics provide explanability and ease of model checking.

## Compilation
To compile the project, we rely on ALGLIB (http://alglib.net). The free edition can be downloaded from the site, and used for the project.
The source `alglib-cpp.tar.gz` can be obtained from the site, which can be compiled with the project to generate the desired runnable version.
ALGLIB provides necessary solvers for the tool to solve various regression and constraints within the problem domain.

To compile:
```
    git clone http://github.com/nimit-singhania/mosaic-cpp
    cd mosaic-cpp
    tar -xvf alglib-cpp.tar.gz
    ./compile.sh
```

To run:
```
    ./main
```

The main provides a sample program that is used to assess the tool and its capabilities. The work is currently in progress and subsequent improvements will be made to the tool.
