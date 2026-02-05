# Preliminary Results with ISTELLA22 Dataset
To assess the efficacy of the tool, we trained multiple piecewise affine models and evaluated their efficacy on test data-sets.

We report the RMSE (on test and train data-set) and precision for each of the versions created for ISTELLA22 data-set.


| Data-set         | Train RMSE       | Test RMSE    | 
|---------------|----------------|------------|
| `istell22_v1.txt`| 0.640165          | 0.500014      |
| `istell22_v2.txt`| **0.61914**          | **0.330402**     |
| `istell22_v3.txt`| 5.33884          | 3.3611       |
| `istell22_v4.txt`| 1.32875          | 1.99878      |
|`istella22_v5.txt`| 730.407          | 1083.97      |

We note that we ran each model training with the following command:
```
    ./main -t 0.5 -o <model_output_path> -s 60 <train_set>
```
We measure the precision of the model by computing the fraction of points on which the error is less than the threshold 0.5.

| Data-set         | Train Precision       | Test Precision    | 
|---------------|----------------|------------|
| `istell22_v1.txt`| 0.915276          | 0.954638      |
| `istell22_v2.txt`| **0.918204**          | **0.976903**      |
| `istell22_v3.txt`| 0.0217212          | 0.0074451       |
| `istell22_v4.txt`| 0.879656          | 0.53271      |
| `istell22_v5.txt`| 0.0281924          | 0.0127742      |

Further, we observe that the train RMSE is quite huge as compared to the expected error of 0.5. This is because of an optimization in the training algorithm, that limits the number of iterations that the guard learning method runs for (to optimize the training time). We set it to 60 for the current training (via option `-s 60`). Increasing the iterations both increases the training time and the complexity of the model learnt. Improved methods for training the model are currently being considered.

We also observe that the scaling of the model reduces with additional features added to the data-set. The v1 and V4 data-sets have only 2 input features, while the others have 4 or more features. While this should not impact the training, we do observe this to have impact on teh training time. We further apply normalization of the features, which is used to scale the features to a uniform average value. This is useful when using bivariate features (as a heuristic) to identify predicates or split functions. We further analyze the performance of the tool when normalization is turned off.

| Data-set         | Train RMSE       | Test RMSE    | 
|--------------|----------------|------------|
|`istella22_v5.txt` | 29.2449          | 37.4807      |


| Data-set         | Train Precision       | Test Precision    | 
|---------------|----------------|------------|
| `istell22_v5.txt`| **0.677003**         | **0.676903**      |

Clearly, the precision is highest for the unnormalized version here, however the difference in data-sets could also be responsible for the differences. We could not compile the model for other data-sets for the unnormalized version of the features. With unnormalized features, fewer rounds of iteration suffice, while providing high coverage.

