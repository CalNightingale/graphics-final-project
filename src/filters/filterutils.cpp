#include <cmath>
#include <vector>
#include "filterutils.h"

namespace FilterUtils {

/**
 * @brief Convert a float value to a 0-255 pixel value
 * @param x: the input float value
 * @return the corresponding pixel value, rounded and clamped to fit within 0-255
 */
std::uint8_t floatToUint8(float x) {
    return (std::uint8_t) fmax(0, fmin(round(x), 255));
}

/**
 * @brief Convert a vector of float pixels to RGB pixels
 * @param floatVec: the vector of pixels, where each pixel contains floats
 * @return the converted vector of pixels, each pixel now contains RGBAs
 */
std::vector<RGBA> vectorToRGBA(const std::vector<RGBAFloat>& floatVec) {
    std::vector<RGBA> outputVec;
    for (int i = 0; i < floatVec.size(); i++) {
        RGBAFloat inputPixel = floatVec[i];
        RGBA outputPixel = RGBA{floatToUint8(inputPixel.r), floatToUint8(inputPixel.g), floatToUint8(inputPixel.b)};
        outputVec.push_back(outputPixel);
    }
    return outputVec;
}

/**
 * @brief Convert a vector of RGB pixels to contain float values
 * @param rgbaVec: the vector of RGBA pixels
 * @return the converted vector of pixels, each pixel now contains floats
 */
std::vector<RGBAFloat> vectorToRGBAFloat(const std::vector<RGBA>& rgbaVec) {
    std::vector<RGBAFloat> outputVec;
    for (int i = 0; i < rgbaVec.size(); i++) {
        RGBA inputPixel = rgbaVec[i];
        RGBAFloat outputPixel = RGBAFloat{(float) inputPixel.r, (float) inputPixel.g, (float) inputPixel.b};
        outputVec.push_back(outputPixel);
    }
    return outputVec;
}

// Adapted from lab 3
/**
 * @brief Handles out-of-bounds pixels by repeating the pixel on the edge of the image such that A,B,C,D looks like ...A,A,A,B,C,D,D,D...
 * @param data1D: an input image scanline
 * @param x: the index to access from the scanline; can be within or out of bounds
 * @return the corresponding pixel from the scanline, if x is within bounds; otherwise the pixel obtained by repeating
 */
RGBAFloat getPixelRepeated1D(std::vector<RGBAFloat> &data1D, int x) {
    int newX = (x < 0) ? 0 : std::min(x, (int)data1D.size()  - 1);
    return data1D[newX];
}

/**
 * @brief Apply convolution in 1D
 * @param data1D: the input image scanline to convolve
 * @param kernel: the kernel to convolve with
 * @return the result of convolving the kernel on the scanline
 */
std::vector<RGBAFloat> convolve1D(std::vector<RGBAFloat>& data1D, const std::vector<float>& kernel) {
    std::vector<RGBAFloat> outputData;
    outputData.resize(data1D.size());
    int kerRadius = (kernel.size() - 1) / 2;
    for (int dataIndex = 0; dataIndex < data1D.size(); dataIndex++) {
        float redAcc = 0;
        float greenAcc = 0;
        float blueAcc = 0;
        for (int kerIndex = 0; kerIndex < kernel.size(); kerIndex++) {
            RGBAFloat pixel = getPixelRepeated1D(data1D, dataIndex - kerRadius + kerIndex);
            float weight = kernel[kerIndex];
            redAcc += weight * pixel.r;
            greenAcc += weight * pixel.g;
            blueAcc += weight * pixel.b;
        }
        outputData[dataIndex] = RGBAFloat{redAcc, greenAcc, blueAcc};
    }
    return outputData;
}

/**
 * @brief Convolve a single row of a 2D image
 * @param rowIndex: the index of the row to convolve
 * @param data: the 2D image whose row to convolve
 * @param width: the width of the image
 * @param height: the height of the image
 * @param kernel: the kernel to convolve with
 */
void convolveRow(int rowIndex, std::vector<RGBAFloat>& data, int width, int height, const std::vector<float>& kernel) {
    std::vector<RGBAFloat> inputRow;
    for (int colIndex = 0; colIndex < width; colIndex++) {
        inputRow.push_back(data[rowIndex * width + colIndex]);
    }
    std::vector<RGBAFloat> outputRow = convolve1D(inputRow, kernel);
    std::copy(outputRow.begin(), outputRow.end(), data.begin() + rowIndex * width);
}

/**
 * @brief Convolve a single column of a 2D image
 * @param rowIndex: the index of the column to convolve
 * @param data: the 2D image whose column to convolve
 * @param width: the width of the image
 * @param height: the height of the image
 * @param kernel: the kernel to convolve with
 */
void convolveCol(int colIndex, std::vector<RGBAFloat>& data, int width, int height, const std::vector<float>& kernel) {
    std::vector<RGBAFloat> inputCol;
    for (int rowIndex = 0; rowIndex < height; rowIndex++) {
        inputCol.push_back(data[rowIndex * width + colIndex]);
    }
    std::vector<RGBAFloat> outputCol = convolve1D(inputCol, kernel);
    for (int rowIndex = 0; rowIndex < height; rowIndex++) {
        data[rowIndex * width + colIndex] = outputCol[rowIndex];
    }
}

/**
 * @brief Convolve all pixels in a 2D image row-by-row and then column-by-column
 * @param data: the 2D image to convolve
 * @param width: the width of the image
 * @param height: the height of the image
 * @param rowKernel: the 1D kernel to convolve rows with
 * @param colKernel: the 1D kernel to convolve columns with
 * @return
 */
std::vector<RGBAFloat> convolve2D(const std::vector<RGBA>& data, int width, int height, const std::vector<float>& rowKernel, const std::vector<float>& colKernel) {
    std::vector<RGBAFloat> dataFloat = vectorToRGBAFloat(data);

    for (int rowIndex = 0; rowIndex < height; rowIndex++) {
        convolveRow(rowIndex, dataFloat, width, height, rowKernel);
    }

    for (int colIndex = 0; colIndex < width; colIndex++) {
        convolveCol(colIndex, dataFloat, width, height, colKernel);
    }

    return dataFloat;
}

}
