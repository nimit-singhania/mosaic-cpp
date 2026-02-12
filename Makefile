.PHONY: all clean intel
all: train infer model_stats

train: train.cpp src/*.cpp alglib-cpp/src/*.cpp include/*.hpp
	g++ -std=c++11 -O3 -I include/ -I alglib-cpp/src/ src/*.cpp alglib-cpp/src/*.cpp train.cpp -o main -DCHECK -DEBUG -lboost_json 
train_naive_bayes: train_naive_bayes.cpp  src/utils.cpp include/utils.hpp
	g++ -std=c++11 -O3 -I include/ src/utils.cpp train_naive_bayes.cpp -o train_naive_bayes -DCHECK -DEBUG -lboost_json 
infer: infer.cpp src/utils.cpp include/*.hpp
	g++ -std=c++11 -O3 -I include/ src/utils.cpp infer.cpp -o infer -lboost_json
infer_naive_bayes: infer_naive_bayes.cpp src/utils.cpp include/*.hpp
	g++ -std=c++11 -O3 -I include/ src/utils.cpp infer_naive_bayes.cpp -o infer_naive_bayes -lboost_json
model_stats: model_stats.cpp src/utils.cpp include/*.hpp
	g++ -std=c++11 -O3 -I include/ src/utils.cpp model_stats.cpp -o model_stats -lboost_json

intel: train.cpp src/*.cpp alglib-cpp/src/*.cpp include/*.hpp
	rm main
	g++ -std=c++11 -O3 -I include/ -I alglib-cpp/src/ src/*.cpp alglib-cpp/src/*.cpp train.cpp -o main -DCHECK -DEBUG -lboost_json -DAE_CPU=AE_INTEL -mavx2 -mfma -DAE_OS=AE_POSIX 

clean:
	rm main infer model_stats
