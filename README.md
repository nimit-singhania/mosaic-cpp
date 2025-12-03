# mosaic-cpp

Mosaic is a tool to learn piecewise affine models from input-output data. A piecewise affine model consists of multiple regions each region guarded by a guarded predicate and an affine function that defines the dynamics of the model in the region. To compute the model output for an input, the region for the input is located, followed by computation of the affine function on the input. The piecewise nature enables the model to represent non-linear behavior while affine dynamics provide explanability and ease of model checking.

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
    ./main <path_to_data> <threshold>
```

The tool accepts a csv file with rows corresponding to data vectors and first N-1 columns for the input vector while the last column for the output floating point value. Sample data is available in `test_data.txt`. This corresponds to a data-set where the input is a 2-dimensional floating vector, and the output is a single floating point value. The threshold corresponds to the error threshold acceptable on the model output. For more details, please refer to [1].

[1] Rajeev Alur, Nimit Singhania. Precise Piecewise Affine Models from Input Output Data. EMSOFT 2013.
