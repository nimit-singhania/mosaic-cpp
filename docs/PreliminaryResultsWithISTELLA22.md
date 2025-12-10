# Preliminary Results with ISTELLA22 Dataset
To assess the efficacy of the tool, we trained multiple piecewise affine models and evaluated their efficacy on test data-sets.

We report the RMSE (on test and train data-set) for each of the versions created for ISTELLA22 data-set.

| Data-set         | Train RMSE       | Test RMSE    | 
|:-------------- -:|:----------------:|:------------:|
| `istell22_v1.txt`| 2.01595          | 2.09373      |
| `istell22_v3.txt`| 5.33884          | 3.3611       |
| `istell22_v4.txt`| 2.07402          | 2.93968      |

We note that we ran each model training with the following command:
```
    ./main <train_set> 0.5 <model_output_path>
```

Further, we observe that the train RMSE is quite huge as compared to the expected error of 0.5. This is because of an optimization in the training algorithm, that limits the number of iterations that the guard learning method runs for (to optimize the training time). We set it to 60 for the current training. Increasing the iterations both increases the training time and the complexity of the model learnt. Improved methods for training the model are currently being considered.

We also observe that the scaling of the model reduces with additional features added to the data-set. The v1 and V4 data-sets have only 2 input features, while the others have 4 or more features. While this should not impact the training, we do observe this to have impact on teh training time. We further apply normalization of the features, which is used to scale the features to a uniform average value. This is useful when using bivariate features (as a heuristic) to identify predicates or split functions. We further analyze the performance of the tool when normalization is turned off.

| Data-set         | Train RMSE       | Test RMSE    | 
|:-------------- -:|:----------------:|:------------:|
|`istella22_v5.txt | 29.2449          | 37.4807      |

The RMSE is much higher compared to the other data-sets, however note that we set the number of iterations to only 30 (as compared to 60 for the normalized version). We will increasing training time for better performance.
