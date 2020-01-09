#include <tiffio.h>
#include <cmath>
#include <cstring>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


int main(int argc, char **argv) {
    std::string output_filename = "output.dng";

    if (argc <= 1) {
        std::cout << "Specify at least input filename" << std::endl;
        return -1;
    }

    std::string input_filename = std::string(argv[1]);

    if (argc > 2) {
        output_filename = std::string(argv[2]);
    }

    int max_white = 255;
    int width;
    int height;
    int bitdepth;
    void* data = stbi_load(input_filename.c_str(), &width, &height, &bitdepth, 3);

    int src_width = width;

    std::cout << "Image information: " << std::endl
              << "Width: " << width << std::endl
              << "Height: " << height << std::endl
              << "Bitdepth: " << bitdepth << std::endl
              << std::endl;

    // Round target shapes
    width = (width / 2) * 2;
    height = (height / 2) * 2;


    std::cout << "Target Bayer matrix: " << std::endl
              << "Width: " << width << std::endl
              << "Height: " << height << std::endl
              << std::endl;

    static const short bayerPatternDimensions[] = { 2, 2 };
    static const float ColorMatrix1[] =
    {
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0,
    };

    static const float AsShotNeutral[] =
    {
        1.0, 1.0, 1.0,
    };

    // Bayer pixel values:
    unsigned char image[height * width];
    memset(image, 0, height * width);

    for (int i = 0; i < height; i += 2) {
        for (int j = 0; j < width; j+= 2) {
            image[i * width + j] = ((char*)data)[3 * (i * src_width + j)];
            image[i * width + j + 1] = ((char*)data)[3 * (i * src_width + j) + 1];
            image[(i + 1) * width + j] = ((char*)data)[3 * ((i + 1) * src_width + j) + 1];
            image[(i + 1) * width + j + 1] = ((char*)data)[3 * ((i + 1) * src_width + j + 1) + 2];
        }
    }

    TIFF* tif = TIFFOpen (output_filename.c_str(), "w");

    TIFFSetField (tif, TIFFTAG_DNGVERSION, "\01\01\00\00");
    TIFFSetField (tif, TIFFTAG_SUBFILETYPE, 0);
    TIFFSetField (tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
    TIFFSetField (tif, TIFFTAG_IMAGEWIDTH, width);
    TIFFSetField (tif, TIFFTAG_IMAGELENGTH, height);
    TIFFSetField (tif, TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField (tif, TIFFTAG_ROWSPERSTRIP, 1);
    TIFFSetField (tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField (tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_CFA);
    TIFFSetField (tif, TIFFTAG_SAMPLESPERPIXEL, 1);
    TIFFSetField (tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField (tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
    TIFFSetField (tif, TIFFTAG_CFAREPEATPATTERNDIM, bayerPatternDimensions);
    TIFFSetField (tif, TIFFTAG_CFAPATTERN, "\00\01\01\02");
    TIFFSetField (tif, TIFFTAG_MAKE, "DNG");
    TIFFSetField (tif, TIFFTAG_UNIQUECAMERAMODEL, "DNG");
    TIFFSetField (tif, TIFFTAG_COLORMATRIX1, 9, ColorMatrix1);
    TIFFSetField (tif, TIFFTAG_ASSHOTNEUTRAL, 3, AsShotNeutral);
    TIFFSetField (tif, TIFFTAG_CFALAYOUT, 1);
    TIFFSetField (tif, TIFFTAG_CFAPLANECOLOR, 3, "\00\01\02");
    TIFFSetField(tif, TIFFTAG_WHITELEVEL, 1, &max_white);

    for (int row = 0; row < height; row++) {
        TIFFWriteScanline (tif, image + row * width, row, 0);
    }

    TIFFClose (tif);

    return 0;
}