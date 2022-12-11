#include <vector>
#include <math.h>
#include "rgba.h"
#include "canvas2d.h"
#include "filterutils.h"

// Grayscale code taken from lab 3

/**
 * @brief Obtain the grayscale value corresponding to an RGB pixel
 * @param pixel: the input pixel
 * @return the corresponding grayscale value
 */
std::uint8_t rgbaToGray(const RGBA &pixel) {
    float lumaWeightR = 0.299;
    float lumaWeightG = 0.587;
    float lumaWeightB = 0.114;
    std::uint8_t grayValue = FilterUtils::floatToUint8(pixel.r * lumaWeightR + pixel.g * lumaWeightG + pixel.b * lumaWeightB);

    return grayValue;
}

/**
 * @brief Apply a grayscale filter to the canvas image
 */
void Canvas2D::filterGray() {
    for (int row = 0; row < m_height; ++row) {
        for (int col = 0; col < m_width; ++col) {
            size_t currentIndex = m_width * row + col;
            RGBA &currentPixel = m_data[currentIndex];

            std::uint8_t grayValue = rgbaToGray(currentPixel);

            RGBA grayColor{grayValue, grayValue, grayValue};
            currentPixel = grayColor;
        }
    }
}
