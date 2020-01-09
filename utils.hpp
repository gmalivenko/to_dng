#include <vector>
#include <tuple>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <tiffio.h>

namespace utils {
    template<typename T>
    std::vector<T> pixelsToBayer(std::vector<T> const &srcData,
        unsigned const int height, unsigned const int width, unsigned const int channels, unsigned const int stride
    ) {
        if (width % 2 || height % 2) {
            throw std::invalid_argument("Input shapes should be divisible by 2");
        }
        // Bayer pixel values:
        std::vector<T> bayer(height * width, static_cast<T>(0));

        for (unsigned int i = 0; i < height; i += 2) {
            for (unsigned int j = 0; j < width; j += 2) {
                bayer[i * width + j] = srcData[channels * (i * stride + j)];
                bayer[i * width + j + 1] = srcData[channels * (i * stride + j) + 1];
                bayer[(i + 1) * width + j] = srcData[channels * ((i + 1) * stride + j) + 1];
                bayer[(i + 1) * width + j + 1] = srcData[channels * ((i + 1) * stride + j + 1) + 2];
            }
        }

        return bayer;
    }

    auto readImage(const std::string &filename, unsigned const int channels) {
        int width;
        int height;
        int bitdepth;
        void* data = stbi_load(filename.c_str(), &width, &height, &bitdepth, channels);
        std::vector<unsigned char> srcData((unsigned char*)data, ((unsigned char*)data + width * height * channels));

        return std::make_tuple(srcData, width, height);
    }

    template<typename T>
    auto writeTiff(const std::string &filename, std::vector<T> const &bayer,
        unsigned const int height, unsigned const int width
    ) {
        const int max_white = 255;
        const short bayerPatternDimensions[] = { 2, 2 };
        const float ColorMatrix1[] =
        {
            1.0, 0.0, 0.0,
            0.0, 1.0, 0.0,
            0.0, 0.0, 1.0,
        };

        const float AsShotNeutral[] =
        {
            1.0, 1.0, 1.0,
        };

        auto tif = TIFFOpen(filename.c_str(), "w");

        TIFFSetField (tif, TIFFTAG_DNGVERSION, "\01\01\00\00");
        TIFFSetField (tif, TIFFTAG_SUBFILETYPE, 0);
        TIFFSetField (tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
        TIFFSetField (tif, TIFFTAG_IMAGEWIDTH, width);
        TIFFSetField (tif, TIFFTAG_IMAGELENGTH, height);
        TIFFSetField (tif, TIFFTAG_BITSPERSAMPLE, sizeof(T) * 8);
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

        for (unsigned int row = 0; row < height; row++) {
            TIFFWriteScanline (tif, (T*)(bayer.data()) + row * width, row, 0);
        }

        TIFFClose(tif);

        return 0;
    }
}