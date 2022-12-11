#include <vector>
#include <math.h>
#include "filterutils.h"
#include "rgba.h"
#include "canvas2d.h"
#include "settings.h"

/**
 * @brief Apply a sobel filter to the canvas image
 */
void Canvas2D::filterEdge() {
    filterGray(); // convert m_data to grayscale
    std::vector<float> sobelXRow = {1, 0, -1}; // These kernels are flipped 180 degrees so that we are actually doing convolution
    std::vector<float> sobelXCol = {1, 2, 1};
    std::vector<RGBAFloat> sobelXOutput = FilterUtils::convolve2D(m_data, m_width, m_height, sobelXRow, sobelXCol);

    std::vector<float> sobelYRow = {1, 2, 1};
    std::vector<float> sobelYCol = {1, 0, -1};
    std::vector<RGBAFloat> sobelYOutput = FilterUtils::convolve2D(m_data, m_width, m_height, sobelYRow, sobelYCol);

    for (int imageIndex = 0; imageIndex < m_data.size(); imageIndex++) {
        // Since the images are in grayscale, all channels have the same value; we just look at r to get the grayscale value
        float Gx = sobelXOutput[imageIndex].r;
        float Gy = sobelYOutput[imageIndex].r;
        float G = sqrt(pow(Gx, 2) + pow(Gy, 2));
        std::uint8_t outputGrayValue = FilterUtils::floatToUint8(G * settings.edgeDetectSensitivity);
        m_data[imageIndex] = RGBA{outputGrayValue, outputGrayValue, outputGrayValue};
    }
}

