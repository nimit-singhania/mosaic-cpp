# Comparison with Naive Bayes

A Naive Bayes model makes simplistic assumption that given the output label, the features are independent of each other and hence their contributions can computed and accumulated independently. We assume that given features have a Normal distribution. We assess the performance of the Naive Bayes model in comparison to the modelling approach presented in this work. As presented in the [Preliminary Results](docs/PreliminaryResultsWithISTELLA22.md), the best performing configuration is selected as representative. To build a Naive Bayes model, we perform the following commands:

```
make train_naive_bayes infer_naive_bayes
./train_naive_bayes -o model_naive_bayes istella22/istella22_v5.txt
./infer_naive_bayes -i model_naive_bayes -t 0.5 istella22/istella22_v5test.txt # for test RMSE and precision
./infer_naive_bayes -i model_naive_bayes -t 0.5 istella22/istella22_v5.txt # for train RMSE and precision
```

The observed performance characteristics are here:

modelling approach |	Train RMSE | 	Train Precision |	Test RMSE |	Test Precision |
------------------|------------|----------|---------|-------|
Piecewise affine model | 	0.519565 |	0.919753 |	0.205845 |	0.990627|
Naive Bayes	| 	0.551295| 	0.917752 | 	0.211423 | 	0.98554 |

As can be observed, the two models perform similarly with the piecewise affine model having a slight edge in performance over the Naive Bayes model. The Naive Bayes model is simpler however and provides reasonable performance with a simple interface. This definitely provides a baseline that can be used to improve the piecewise affine modelling approach and its performance. One possibility is to use higher precision values and coefficients (for example the Naive Bayes model leverages double-precision to avoid NAN values). 
