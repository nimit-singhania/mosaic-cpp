#!/bin/bash

tar -tf istella22.tar.gz

tar -xvf istella22.tar.gz istella22/train.svm.gz
tar -xvf istella22.tar.gz istella22/test.svm.gz
cd istella22

# Train data sets.
# The feature is offset by two records (since the first and the second record are the graded relevance score and the query id).
gunzip train.svm.gz
awk '{print $127 "," $142 "," $1}' train.svm | sed 's/[0-9]*://g' > istella22_v1.txt
awk '{print $6 "," $31 "," $41 "," $45 "," $127 "," $128 "," $139 "," $141 "," $142 "," $153 "," $182 "," $1}' train.svm | sed 's/[0-9]*://g' > istella22_v2.txt
awk '{print $127 "," $141 "," $142 "," $182 "," $1}' train.svm | sed 's/[0-9]*://g' > istella22_v3.txt
awk '{print $139 "," $142 "," $1}' train.svm | sed 's/[0-9]*://g' > istella22_v4.txt
awk '{print $127 "," $143 "," $144 "," $145 "," $147 "," $1}' train.svm | sed 's/[0-9]*://g' > istella22_v5.txt

gunzip test.svm.gz
awk '{print $127 "," $142 "," $1}' test.svm | sed 's/[0-9]*://g' > istella22_v1test.txt
awk '{print $6 "," $31 "," $41 "," $45 "," $127 "," $128 "," $139 "," $141 "," $142 "," $153 "," $182 "," $1}' test.svm | sed 's/[0-9]*://g' > istella22_v2test.txt
awk '{print $127 "," $141 "," $142 "," $182 "," $1}' test.svm | sed 's/[0-9]*://g' > istella22_v3test.txt
awk '{print $139 "," $142 "," $1}' test.svm | sed 's/[0-9]*://g' > istella22_v4test.txt
awk '{print $127 "," $143 "," $144 "," $145 "," $147 "," $1}' test.svm | sed 's/[0-9]*://g' > istella22_v5test.txt
