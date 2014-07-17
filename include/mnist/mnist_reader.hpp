//=======================================================================
// Copyright (c) 2014 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef MNIST_READER_HPP
#define MNIST_READER_HPP

#include <fstream>
#include <iostream>
#include <vector>
#include <cstdint>
#include <memory>

namespace mnist {

template<template<typename...> class  Container = std::vector, template<typename...> class  Sub = std::vector, typename Pixel = uint8_t, typename Label = uint8_t>
struct MNIST_dataset {
    Container<Sub<Pixel>> training_images;
    Container<Sub<Pixel>> test_images;
    Container<Label> training_labels;
    Container<Label> test_labels;
};

inline uint32_t read_header(const std::unique_ptr<char[]>& buffer, size_t position){
    auto header = reinterpret_cast<uint32_t*>(buffer.get());

    auto value = *(header + position);
    return (value << 24) | ((value << 8) & 0x00FF0000) | ((value >> 8) & 0X0000FF00) | (value >> 24);
}

template<template<typename...> class Container = std::vector, template<typename...> class Sub = std::vector, typename Pixel = uint8_t>
Container<Sub<Pixel>> read_mnist_image_file(const std::string& path){
    std::ifstream file;
    file.open(path, std::ios::in | std::ios::binary | std::ios::ate);

    if(!file){
        std::cout << "Error opening file" << std::endl;
    } else {
        auto size = file.tellg();
        std::unique_ptr<char[]> buffer(new char[size]);

        //Read the entire file at once
        file.seekg(0, std::ios::beg);
        file.read(buffer.get(), size);
        file.close();

        auto magic = read_header(buffer, 0);

        if(magic != 0x803){
            std::cout << "Invalid magic number, probably not a MNIST file" << std::endl;
        } else {
            auto count = read_header(buffer, 1);
            auto rows = read_header(buffer, 2);
            auto columns = read_header(buffer, 3);

            if(size < count * rows * columns + 16){
                std::cout << "The file is not large enough to hold all the data, probably corrupted" << std::endl;
            } else {
                //Skip the header
                //Cast to unsigned char is necessary cause signedness of char is
                //platform-specific
                auto image_buffer = reinterpret_cast<unsigned char*>(buffer.get() + 16);

                Container<Sub<Pixel>> images;
                images.reserve(count);

                for(size_t i = 0; i < count; ++i){
                    images.emplace_back(rows * columns);

                    for(size_t j = 0; j < rows * columns; ++j){
                        auto pixel = *image_buffer++;
                        images[i][j] = static_cast<Pixel>(pixel);
                    }
                }

                return std::move(images);
            }
        }
    }

    return {};
}

template<template<typename...> class  Container = std::vector, typename Label = uint8_t>
Container<Label> read_mnist_label_file(const std::string& path){
    std::ifstream file;
    file.open(path, std::ios::in | std::ios::binary | std::ios::ate);

    if(!file){
        std::cout << "Error opening file" << std::endl;
    } else {
        auto size = file.tellg();
        std::unique_ptr<char[]> buffer(new char[size]);

        //Read the entire file at once
        file.seekg(0, std::ios::beg);
        file.read(buffer.get(), size);
        file.close();

        auto magic = read_header(buffer, 0);

        if(magic != 0x801){
            std::cout << "Invalid magic number, probably not a MNIST file" << std::endl;
        } else {
            auto count = read_header(buffer, 1);

            if(size < count + 8){
                std::cout << "The file is not large enough to hold all the data, probably corrupted" << std::endl;
            } else {
                //Skip the header
                //Cast to unsigned char is necessary cause signedness of char is
                //platform-specific
                auto label_buffer = reinterpret_cast<unsigned char*>(buffer.get() + 8);

                Container<Label> labels(count);

                for(size_t i = 0; i < count; ++i){
                    auto label = *label_buffer++;
                    labels[i] = static_cast<Label>(label);
                }

                return std::move(labels);
            }
        }
    }

    return {};
}

template<template<typename...> class Container = std::vector, template<typename...> class Sub = std::vector, typename Pixel = uint8_t>
Container<Sub<Pixel>> read_training_images(){
    return read_mnist_image_file<Container,Sub,Pixel>("mnist/train-images-idx3-ubyte");
}

template<template<typename...> class Container = std::vector, template<typename...> class Sub = std::vector, typename Pixel = uint8_t>
Container<Sub<Pixel>> read_test_images(){
    return read_mnist_image_file<Container,Sub,Pixel>("mnist/t10k-images-idx3-ubyte");
}

template<template<typename...> class Container = std::vector, typename Label = uint8_t>
Container<Label> read_training_labels(){
    return read_mnist_label_file<Container, Label>("mnist/train-labels-idx1-ubyte");
}

template<template<typename...> class Container = std::vector, typename Label = uint8_t>
Container<Label> read_test_labels(){
    return read_mnist_label_file<Container, Label>("mnist/t10k-labels-idx1-ubyte");
}

template<template<typename...> class Container = std::vector, template<typename...> class Sub = std::vector, typename Pixel = uint8_t, typename Label = uint8_t>
MNIST_dataset<Container, Sub, Pixel, Label> read_dataset(){
    MNIST_dataset<Container, Sub, Pixel, Label> dataset;

    dataset.training_images = read_training_images<Container, Sub, Pixel>();
    dataset.training_labels = read_training_labels<Container, Label>();

    dataset.test_images = read_test_images<Container, Sub, Pixel>();
    dataset.test_labels = read_test_labels<Container, Label>();

    return std::move(dataset);
}

} //end of namespace mnist

#endif
