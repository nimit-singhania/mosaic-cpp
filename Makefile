.PHONY: all
all: main infer model_stats

main: train.cpp src/*.cpp alglib-cpp/src/*.cpp include/*.hpp
	g++ -std=c++11 -O3 -I include/ -I alglib-cpp/src/ src/*.cpp alglib-cpp/src/*.cpp train.cpp -o main -DAE_CPU=AE_INTEL -mavx2 -mfma -DAE_OS=AE_POSIX -pthread -DCHECK -lboost_json
infer: infer.cpp src/utils.cpp include/*.hpp
	g++ -std=c++11 -O3 -I include/ src/utils.cpp infer.cpp -o infer -lboost_json
model_stats: model_stats.cpp src/utils.cpp include/*.hpp
	g++ -std=c++11 -O3 -I include/ src/utils.cpp model_stats.cpp -o model_stats -lboost_json
