#ifndef FILTERUTILS_H
#define FILTERUTILS_H

#include <vector>
#include "rgba.h"

namespace FilterUtils {
    std::uint8_t floatToUint8(float x);
    std::vector<RGBA> vectorToRGBA(const std::vector<RGBAFloat>& floatVec);
    std::vector<RGBAFloat> vectorToRGBAFloat(const std::vector<RGBA>& rgbaVec);
    RGBAFloat getPixelRepeated1D(std::vector<RGBAFloat> &data1D, int x);
    std::vector<RGBAFloat> convolve2D(const std::vector<RGBA>& data, int width, int height, const std::vector<float>& rowKernel, const std::vector<float>& colKernel);
};

#endif // FILTERUTILS_H
