# mosaic-cpp

Mosaic is a tool to learn piecewise affine models from input-output data. A piecewise affine model consists of multiple regions each region guarded by a guarded predicate and an affine function that defines the dynamics of the model in the region. To compute the model output for an input, the region for the input is located, followed by computation of the affine function on the input. The piecewise nature enables the model to represent non-linear behavior while affine dynamics provide explanability and ease of model checking.

## Running the tool
To compile the project, we rely on ALGLIB (http://alglib.net). The free edition can be downloaded from the site, and used for the project.
The source `alglib-cpp.tar.gz` can be obtained from the site, which can be compiled with the project to generate the desired runnable version.
ALGLIB provides necessary solvers for the tool to solve various regression and constraints within the problem domain.

To compile:
```
    git clone https://github.com/nimit-singhania/mosaic-cpp.git
    cd mosaic-cpp
    tar -xvf alglib-cpp.tar.gz
    make
```

To run:
```
    ./main <path_to_data> <threshold> [<model_file_output_path>]
```

The tool accepts a CSV file with rows corresponding to data vectors and first N-1 columns for the input vector while the last column for the output floating point value.


## Sample evaluation
Sample data is available in `test_data.txt`. This corresponds to a data-set where the input is a 2-dimensional floating vector, and the output is a single floating point value. The threshold corresponds to the error threshold acceptable on the model output. To run the tool on this test data:

```
    ./main test_data.txt 0.1
```

## Evaluating the tool
To evaluate the efficacy of the tool, and to improve the robustness, we obtained the [ISTELLA22 dataset](https://istella.ai/datasets/istella22-dataset/) [2] that consists of query-document collection with 220 rich industrial features (based on query, document and query-document pair) learn-to-rank dataset. We identified a small set of features that we use to train the prediction model (using piecewise affine model). We have provided the following variants:
* `istella22_v1.txt`: this consists of two features (features 125 and 140).
* `istella22_v2.txt`: this consists of features 4, 29, 39, 43, 125, 126, 137, 139, 140, 151, and 180.
* `istella22_v3.txt`: this consists of features 125, 139, 140, and 180.
* `istella22_v4.txt`: this consists of features 139 and 140.
* `istella22_v5.txt`: this consists of features 125, 141, 142, 143, and 145.

The dataset `istella22.tar.gz` can be downloaded from the [data-set website](https://istella.ai/datasets/istella22-dataset/). We have provided a `istella22_data_generation.sh` file to help create train and test sets. A utility `infer` is available to assess the model performance on test data. To use the tool:

```
    sh istella_data_generation.sh
    make
    ./infer <model_file> istella22/<test_data.txt>
```
The inference will output the expected and inferred output and report the RMSE at the end of the report. The `model_file` can be optionally dumped from the `main` method. The test data is generated from the data generation script.

For more details about the implementation, please refer to [1].

[1] Rajeev Alur, Nimit Singhania. Precise Piecewise Affine Models from Input Output Data. EMSOFT 2013.

[2] Domenico Dato, Sean MacAvaney, Franco Maria Nardini, Raffaele Perego, and Nicola Tonollotto. The Istella22 Dataset: Bridging Traditional and Neural Learning to Rank Evalaution. SIGIR 2022.
