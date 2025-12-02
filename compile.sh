g++ -std=c++11 -O3 -I include/ -I alglib-cpp/src/ src/* alglib-cpp/src/*.cpp -o main -DAE_CPU=AE_INTEL -mavx2 -mfma -DAE_OS=AE_POSIX -pthread
