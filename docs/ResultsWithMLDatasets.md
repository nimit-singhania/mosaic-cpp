# Results with ML Datasets

Mosaic implements an approach to learn piecewise affine models that can be used for predictive modelling for standard ML data-sets. We refer to a few data-sets and compare the performance with [Naive Bayes implementation](ComparisonWithNaiveBayes.md). These data-sets provide additional support for piecewise affine models and their training in general.

##ISIC 2024 - Skin Cancer Detection with 3D-TBP
We leverage the ISIC 2024 dataset ([link](https://www.kaggle.com/competitions/isic-2024-challenge/data)) to evaluate our approach. The data-sets consists of both images and metadata (characteristics extracted from the images) for malignant and benign specimens. We evaluate our methods on metadata to predict an incoming image is benign or malignant. To prepare the data-set, we first joined the metadata file with the ground-truth file to obtain a labelled data-set. Subsequently, we extracted features that were numeric and specific to the characteristics of the image. We then split the data-set into two sets: test and train by 80/20 split. We use the permissive dataset only which is a smaller data-set of about 217K images. We trained both Naive Bayes and Mosaic models to compare performance.

modelling approach | Test RMSE | Test Precision (test error < 0.1)
-----------------------------------------------------------------
Naive Bayes | 0.19317 | 0.883364
Mosaic | 0.0434287 | 0.998114

Note that we consider a label 0 for benign and 1 for malignant. We treat this as a numerical label (rather than a categorial label). This is because Mosaic learns a numerical output. We observe that Naive Bayes is not able to perform as well, which is probably due to a strong assumption that all the input features are independent. Mosaic does not rely on such an assumption.

## Breast Cancer Wisconsin (Diagnostic) Data Set
We leverage the diagnostic [data-set](https://www.kaggle.com/datasets/uciml/breast-cancer-wisconsin-data/data) to learn a binary classifier for benign and malignant cases. The data-set was prepared by converting the output to 0 for Benign and 1 for Malignant, while the features are used as is. The data-set consists of about 500 specimen. We used an 80/20 split to create a train and test data-set. We train both Naive Bayes and Mosaic piecewise affine models.

modelling approach | Test RMSE | Test Precision (test error < 0.1)
-----------------------------------------------------------------
Naive Bayes | 0.347572 | 0.790909
Mosaic | 0.165145 | 0.972727

Again Mosaic is able to scale better, and despite the low number of data-points, it is able to achieve a precision of 97% or higher. This reflects the power of Mosaic, and its ability to automatically learn models from input data.
