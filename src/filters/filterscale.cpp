#include <vector>
#include <math.h>
#include "filterutils.h"
#include "rgba.h"
#include "settings.h"
#include "canvas2d.h"

struct Scaler {
    float radius;
    float scaleFactor;
    virtual float triangleFilter(float x) {return 0;}
};

// Up and downscale not unified because a unified triangle filter
// would have to check for scale each time it is called
struct Upscale : Scaler {
    float triangleFilter(float x) {
        return 1 - fabs(x);
    }
    Upscale(float _scaleFactor) {
        scaleFactor = _scaleFactor;
        radius = 1.0;
    }
};

struct Downscale : Scaler {
    float triangleFilter(float x) {
        return (1 - fabs(x) / radius) / radius;
    }
    Downscale(float _scaleFactor) {
        scaleFactor = _scaleFactor;
        radius = 1/scaleFactor;
    }
};

/**
 * @brief Reconstruct one pixel in the output image by sampling from the input scanline
 * @param outputIndex: the index in the output image scanline
 * @param inputData1D: the corresponding input image scanline
 * @param scaler: the object containing information on how to scale the image
 * @return the reconstructed pixel
 */
RGBAFloat scaleOnePixel(int outputIndex, std::vector<RGBAFloat>& inputData1D, Scaler* scaler) {
    float redAcc = 0, greenAcc = 0, blueAcc = 0, weightsSum = 0;
    float scaleFactor = scaler->scaleFactor;
    float radius = scaler->radius;
    float center = outputIndex/scaleFactor + (1 - scaleFactor)/(2 * scaleFactor);
    int left = ceil(center - radius);
    int right = floor(center + radius);

    for (int i = left; i <= right; i++) {
        float weight = scaler->triangleFilter(i - center);
        weightsSum += weight;
        RGBAFloat pixel = FilterUtils::getPixelRepeated1D(inputData1D, i);
        redAcc += weight * pixel.r;
        greenAcc += weight * pixel.g;
        blueAcc += weight * pixel.b;
    }

    redAcc /= weightsSum;
    greenAcc /= weightsSum;
    blueAcc /= weightsSum;

    return RGBAFloat{redAcc, greenAcc, blueAcc};
}

/**
 * @brief Scale up a scanline from an image
 * @param inputData1D: the input scanline to be scaled
 * @param outputData1D: the vector to store the output scanline
 * @param scaler: the object containing information on how to scale the image
 */
void scale1D(std::vector<RGBAFloat>& inputData1D, std::vector<RGBAFloat>& outputData1D, Scaler* scaler) {
    for (int outputIndex = 0; outputIndex < outputData1D.size(); outputIndex++) {
        outputData1D[outputIndex] = scaleOnePixel(outputIndex, inputData1D, scaler);
    }
}

/**
 * @brief Apply the scaling filter to the canvas image according to the scale factors in the GUI settings
 */
void Canvas2D::filterScale() {
    float xScaleFactor = settings.scaleX;
    float yScaleFactor = settings.scaleY;

    int outputWidth = floor(m_width * xScaleFactor);
    int outputHeight = floor(m_height * yScaleFactor);

    std::vector<RGBAFloat> xScaledOutputData; // intermediate result of scaling original image in x direction
    xScaledOutputData.resize(outputWidth * m_height); // still maintains the height of the original image

    std::vector<RGBAFloat> finalOutputData; // final result after both x and y scaled
    finalOutputData.resize(outputWidth * outputHeight);

    Scaler* xScaler;
    if (xScaleFactor > 1) {
        xScaler = new Upscale(xScaleFactor);
    } else {
        xScaler = new Downscale(xScaleFactor);
    }

    Scaler* yScaler;
    if (yScaleFactor > 1) {
        yScaler = new Upscale(yScaleFactor);
    } else {
        yScaler = new Downscale(yScaleFactor);
    }

    // Scale X
    for (int rowIndex = 0; rowIndex < m_height; rowIndex++) { // convolve over all rows in original image
        std::vector<RGBA> inputRow; // fill out input row data from original image
        for (int colIndex = 0; colIndex < m_width; colIndex++) {
            inputRow.push_back(m_data[rowIndex * m_width + colIndex]);
        }
        std::vector<RGBAFloat> inputRowFloat = FilterUtils::vectorToRGBAFloat((inputRow));

        std::vector<RGBAFloat> outputRow;
        outputRow.resize(outputWidth);
        scale1D(inputRowFloat, outputRow, xScaler); // perform convolution over row

        std::copy(outputRow.begin(), outputRow.end(), xScaledOutputData.begin() + rowIndex * outputWidth); // insert row into x-scaled output image
    }

    // Scale Y
    for (int colIndex = 0; colIndex < outputWidth; colIndex++) { // convolve over all columns -- this time from our image that has been x-scaled
        std::vector<RGBAFloat> inputCol; // fill out input column data from x-scaled image
        for (int rowIndex = 0; rowIndex < m_height; rowIndex++) {
            inputCol.push_back(xScaledOutputData[rowIndex * outputWidth + colIndex]);
        }

        std::vector<RGBAFloat> outputCol;
        outputCol.resize(outputHeight);
        scale1D(inputCol, outputCol, yScaler); // perform convolution over column

        for (int rowIndex = 0; rowIndex < outputHeight; rowIndex++) { // insert column into final output image
            finalOutputData[rowIndex * outputWidth + colIndex] = outputCol[rowIndex];
        }
    }

    // Copy result to canvas
    std::vector<RGBA> finalOutputDataRGBA = FilterUtils::vectorToRGBA(finalOutputData);
    m_data = finalOutputDataRGBA;
    m_width = outputWidth;
    m_height = outputHeight;

    delete xScaler;
    delete yScaler;

}
