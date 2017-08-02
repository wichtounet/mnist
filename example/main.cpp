//=======================================================================
// Copyright (c) 2017 Adrian Schneider
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include "mnist/mnist_reader.hpp"

int main(int argc, char* argv[]) {
    // MNIST_DATA_LOCATION set by MNIST cmake config
    std::cout << "MNIST data directory: " << MNIST_DATA_LOCATION << std::endl;

    // Load MNIST data
    mnist::MNIST_dataset<std::vector, std::vector<uint8_t>, uint8_t> dataSet =
        mnist::read_dataset<std::vector, std::vector, uint8_t, uint8_t>(MNIST_DATA_LOCATION);

    std::cout << "Nbr of training images = " << dataSet.training_images.size() << std::endl;
    std::cout << "Nbr of training labels = " << dataSet.training_labels.size() << std::endl;
    std::cout << "Nbr of test images = " << dataSet.test_images.size() << std::endl;
    std::cout << "Nbr of test labels = " << dataSet.test_labels.size() << std::endl;

    return 0;
}
