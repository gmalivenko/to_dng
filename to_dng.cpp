#include "utils.hpp"
#include <iostream>


int main(int argc, char *argv[]) {
    std::string output_filename = "output.dng";

    if (argc <= 1) {
        std::cout << "Specify at least input filename" << std::endl;
        return -1;
    }

    std::string input_filename = std::string(argv[1]);

    if (argc > 2) {
        output_filename = std::string(argv[2]);
    }

    int channels = 3;
    auto [srcData, width, height] = utils::readImage(input_filename, channels);
    int stride = width;

    std::cout << "Image information: " << std::endl
              << "Width: " << width << std::endl
              << "Height: " << height << std::endl
              << std::endl;

    // Round target shapes
    width = (width / 2) * 2;
    height = (height / 2) * 2;

    std::cout << "Target Bayer matrix: " << std::endl
              << "Width: " << width << std::endl
              << "Height: " << height << std::endl
              << std::endl;

    auto bayer = utils::pixelsToBayer<unsigned char>(srcData, height, width, channels, stride);
    return utils::writeTiff(output_filename, bayer, height, width);
}
