#include <vector>
#include <math.h>
#include "filterutils.h"
#include "rgba.h"
#include "settings.h"
#include "canvas2d.h"

/**
 * @brief Create a gaussian kernel according to the radius set in the GUI settings
 * @return the created gaussian kernel that has been discretized and normalized
 */
std::vector<float> createBlurKernel() {
    std::vector<float> blurKernel;
    blurKernel.resize(2 * 2 + 1);
    float kernelSum = 0; // for normalization
    for (int kerIndex = 0; kerIndex < blurKernel.size(); kerIndex++) {
        float xSquared =  pow(2.0 - kerIndex, 2);
        float sigmaSquared = pow(2.0 / 3, 2);
        float fX = exp(-xSquared / (2 * sigmaSquared)) / sqrt(2 * M_PI * sigmaSquared);
        blurKernel[kerIndex] = fX;
        kernelSum += fX;
    }

    // normalize by dividing each element by sum, so that the sum goes from kernelSum to 1
    for (int kerIndex = 0; kerIndex < blurKernel.size(); kerIndex++) {
        blurKernel[kerIndex] /= kernelSum;
    }

    return blurKernel;
}

/**
 * @brief Apply a gaussian filter to the canvas image to blur it
 */
void Canvas2D::filterBlur() {
    std::vector<float> blurKernel = createBlurKernel(); // no need to flip the gaussian kernel since it is vertically symmetric
    std::vector<RGBAFloat> dataFloat = FilterUtils::convolve2D(m_data, m_width, m_height, blurKernel, blurKernel);
    m_data = FilterUtils::vectorToRGBA(dataFloat);
}

