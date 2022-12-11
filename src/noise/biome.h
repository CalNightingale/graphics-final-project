#pragma once

#include "noise/simplex.h"
#include <map>
#include <vector>
#include <functional>
#include <glm/glm.hpp>



class Biome
{
public:
    Biome();
    void histogramEqualize(std::vector<std::vector<float>>& noiseMap);
    std::vector<std::vector<float>> createTemperatureMap(int size, int seed);
    std::vector<std::vector<float>> createPreciptiationMap(int size, int seed);
    std::vector<std::vector<float>> createHeightMap(int size, int seed);
    std::vector<std::vector<float>> createSmoothHeightMap(int size, int seed);
    std::vector<std::vector<float>> createLandMask(int size, int seed);
    std::vector<std::vector<float>> createBiomeHeightMap(int size, int seed, int biome);
    void blurMask(std::vector<std::vector<float>>& mask);
    std::vector<std::vector<float>> multiplyMasks(std::vector<std::vector<float>>& mask1, std::vector<std::vector<float>>& mask2);
    std::vector<std::vector<float>> applyMask(std::vector<std::vector<float>>& originalMap, std::vector<std::vector<float>>& newMap, std::vector<std::vector<float>> mask);

private:
    Simplex simplex;

    std::map<int, int> createHistogram(std::vector<std::vector<float>>& noiseMap, int nBins);
    void createCDF(std::vector<std::vector<float>>& noiseMap, int nBins);
    int cdf(float value);
    float equalizeValue(float value, int size);

    std::map<int, int> cdfTable;
    int cdfMin;
    int m_nBins;
    int m_size;

    float noiseMin = -1.0;
    float noiseMax = 1.0;

    std::function<glm::vec2 (float)> bezier(float x1, float y1, float x2, float y2, float a);
    std::function<std::vector<float> (std::vector<float>&)> interpolate1D(std::vector<float>& xs, std::vector<float>& ys);
    std::function<std::vector<float> (std::vector<float>&)> bezierInterpolate(float x1, float y1, float x2, float y2, float a);
    std::vector<std::vector<float>> filterMap(std::vector<std::vector<float>>& hMap, std::vector<std::vector<float>>& smoothMap, float x1, float y1, float x2, float y2, float a, float b);
};


