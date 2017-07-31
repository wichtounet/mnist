cmake_minimum_required(VERSION 3.0)

PROJECT(mnistexample)

# .. -> hint, that the mnist package is one directory level above.
# When using just "find_package(MNIST REQUIRED)", "MNIST_DIR"
#    cmake variable has to be set correctly.
find_package(MNIST REQUIRED PATHS ..)

include_directories(${MNIST_INCLUDE_DIR})
add_executable(mnistexample main.cpp)

# Pass MNIST data directory to main.cpp
target_compile_definitions(mnistexample PRIVATE MNIST_DATA_LOCATION="${MNIST_DATA_DIR}")

target_compile_features(mnistexample PRIVATE cxx_range_for)







