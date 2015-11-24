//=======================================================================
// Copyright (c) 2014-2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*!
 * \file
 * \brief Contains functions to read the MNIST dataset
 */

#ifndef MNIST_READER_HPP
#define MNIST_READER_HPP

#include <fstream>
#include <iostream>
#include <vector>
#include <cstdint>
#include <memory>

namespace mnist {

/*!
 * \brief Represents a complete mnist dataset
 * \tparam Container The container to use
 * \tparam Image The type of image
 * \tparam Label The type of label
 */
template<template<typename...> class Container, typename Image, typename Label>
struct MNIST_dataset {
    Container<Image> training_images;
    Container<Image> test_images;
    Container<Label> training_labels;
    Container<Label> test_labels;

    /*!
     * \brief Resize the training set to new_size
     *
     * If new_size is less than the current size, this function has no effect.
     *
     * \param new_size The size to resize the training sets to.
     */
    void resize_training(std::size_t new_size){
        if(training_images.size() > new_size){
            training_images.resize(new_size);
            training_labels.resize(new_size);
        }
    }

    /*!
     * \brief Resize the test set to new_size
     *
     * If new_size is less than the current size, this function has no effect.
     *
     * \param new_size The size to resize the test sets to.
     */
    void resize_test(std::size_t new_size){
        if(test_images.size() > new_size){
            test_images.resize(new_size);
            test_labels.resize(new_size);
        }
    }
};

/*!
 * \brief Extract the MNIST header from the given buffer
 * \param buffer The current buffer
 * \param position The current reading positoin
 * \return The value of the mnist header
 */
inline uint32_t read_header(const std::unique_ptr<char[]>& buffer, size_t position){
    auto header = reinterpret_cast<uint32_t*>(buffer.get());

    auto value = *(header + position);
    return (value << 24) | ((value << 8) & 0x00FF0000) | ((value >> 8) & 0X0000FF00) | (value >> 24);
}

/*!
 * \brief Read a MNIST image file inside the given container
 * \param images The container to fill with the images
 * \param path The path to the image file
 * \param limit The maximum number of elements to read (0: no limit)
 * \param func The functor to create the image object
 */
template<template<typename...> class Container = std::vector, typename Image, typename Functor>
void read_mnist_image_file(Container<Image>& images, const std::string& path, std::size_t limit, Functor func){
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

                if(limit > 0 && count > limit){
                    count = limit;
                }

                images.reserve(count);

                for(size_t i = 0; i < count; ++i){
                    images.push_back(func());

                    for(size_t j = 0; j < rows * columns; ++j){
                        auto pixel = *image_buffer++;
                        images[i][j] = static_cast<typename Image::value_type>(pixel);
                    }
                }
            }
        }
    }
}

/*!
 * \brief Read a MNIST label file inside the given container
 * \param images The container to fill with the labels
 * \param path The path to the label file
 * \param limit The maximum number of elements to read (0: no limit)
 */
template<template<typename...> class  Container = std::vector, typename Label = uint8_t>
void read_mnist_label_file(Container<Label>& labels, const std::string& path, std::size_t limit = 0){
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

                if(limit > 0 && count > limit){
                    count = limit;
                }

                labels.resize(count);

                for(size_t i = 0; i < count; ++i){
                    auto label = *label_buffer++;
                    labels[i] = static_cast<Label>(label);
                }
            }
        }
    }
}

/*!
 * \brief Read all training images and return a container filled with the images.
 *
 * The dataset is assumed to be in a mnist subfolder
 *
 * \param limit The maximum number of elements to read (0: no limit)
 * \param func The functor to create the image objects.
 * \return Container filled with the images
 */
template<template<typename...> class Container = std::vector, typename Image, typename Functor>
Container<Image> read_training_images(std::size_t limit, Functor func){
    Container<Image> images;
    read_mnist_image_file<Container, Image>(images, "mnist/train-images-idx3-ubyte", limit, func);
    return images;
}

/*!
 * \brief Read all test images and return a container filled with the images.
 *
 * The dataset is assumed to be in a mnist subfolder
 *
 * \param limit The maximum number of elements to read (0: no limit)
 * \param func The functor to create the image objects.
 * \return Container filled with the images
 */
template<template<typename...> class Container = std::vector, typename Image, typename Functor>
Container<Image> read_test_images(std::size_t limit, Functor func){
    Container<Image> images;
    read_mnist_image_file<Container, Image>(images, "mnist/t10k-images-idx3-ubyte", limit, func);
    return images;
}

/*!
 * \brief Read all training label and return a container filled with the labels.
 *
 * The dataset is assumed to be in a mnist subfolder
 *
 * \param limit The maximum number of elements to read (0: no limit)
 * \return Container filled with the labels
 */
template<template<typename...> class Container = std::vector, typename Label = uint8_t>
Container<Label> read_training_labels(std::size_t limit){
    Container<Label> labels;
    read_mnist_label_file<Container, Label>(labels, "mnist/train-labels-idx1-ubyte", limit);
    return labels;
}

/*!
 * \brief Read all test label and return a container filled with the labels.
 *
 * The dataset is assumed to be in a mnist subfolder
 *
 * \param limit The maximum number of elements to read (0: no limit)
 * \return Container filled with the labels
 */
template<template<typename...> class Container = std::vector, typename Label = uint8_t>
Container<Label> read_test_labels(std::size_t limit){
    Container<Label> labels;
    read_mnist_label_file<Container, Label>(labels, "mnist/t10k-labels-idx1-ubyte", limit);
    return labels;
}

/*!
 * \brief Read dataset and assume images in 3D (1x28x28)
 *
 * The dataset is assumed to be in a mnist subfolder
 *
 * \param training_limit The maximum number of elements to read from training set (0: no limit)
 * \param test_limit The maximum number of elements to read from test set (0: no limit)
 * \return The dataset
 */
template<template<typename...> class Container, typename Image, typename Label = uint8_t>
MNIST_dataset<Container, Image, Label> read_dataset_3d(std::size_t training_limit = 0, std::size_t test_limit = 0){
    MNIST_dataset<Container, Image, Label> dataset;

    dataset.training_images = read_training_images<Container, Image>(training_limit, []{return Image(1, 28, 28);});
    dataset.training_labels = read_training_labels<Container, Label>(training_limit);

    dataset.test_images = read_test_images<Container, Image>(test_limit, []{return Image(1, 28, 28);});
    dataset.test_labels = read_test_labels<Container, Label>(test_limit);

    return dataset;
}

/*!
 * \brief Read dataset.
 *
 * The dataset is assumed to be in a mnist subfolder
 *
 * \param training_limit The maximum number of elements to read from training set (0: no limit)
 * \param test_limit The maximum number of elements to read from test set (0: no limit)
 * \return The dataset
 */
template<template<typename...> class Container, typename Image, typename Label = uint8_t>
MNIST_dataset<Container, Image, Label> read_dataset_direct(std::size_t training_limit = 0, std::size_t test_limit = 0){
    MNIST_dataset<Container, Image, Label> dataset;

    dataset.training_images = read_training_images<Container, Image>(training_limit, []{ return Image(1 * 28 * 28); });
    dataset.training_labels = read_training_labels<Container, Label>(training_limit);

    dataset.test_images = read_test_images<Container, Image>(test_limit, []{ return Image(1 * 28 * 28); });
    dataset.test_labels = read_test_labels<Container, Label>(test_limit);

    return dataset;
}

template<template<typename...> class Container = std::vector, template<typename...> class Sub = std::vector, typename Pixel = uint8_t, typename Label = uint8_t>
MNIST_dataset<Container, Sub<Pixel>, Label> read_dataset(std::size_t training_limit = 0, std::size_t test_limit = 0){
    return read_dataset_direct<Container, Sub<Pixel>>(training_limit, test_limit);
}

} //end of namespace mnist

#endif
