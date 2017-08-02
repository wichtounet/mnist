cmake_minimum_required(VERSION 3.0)

PROJECT(mnist_example)

# .. -> hint, that the mnist package is one directory level above.
# When using just "find_package(MNIST REQUIRED)", "MNIST_DIR"
#    cmake variable has to be set correctly.
find_package(MNIST PATHS ..)
if(NOT MNIST_FOUND)
    message(FATAL_ERROR "MNIST loader could not be found. It is available under https://github.com/wichtounet/mnist")
endif(NOT MNIST_FOUND)

include_directories(${MNIST_INCLUDE_DIR})
add_executable(mnist_example main.cpp)
target_compile_features(mnist_example PRIVATE cxx_range_for)

# Pass MNIST data directory to main.cpp
target_compile_definitions(mnist_example PRIVATE MNIST_DATA_LOCATION="${MNIST_DATA_DIR}")
