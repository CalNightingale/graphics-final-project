#pragma once

#include <map>
#include <vector>

class Biome
{
public:
    Biome();
private:
    std::map<int, int> createHistogram(std::vector<std::vector<float>>& noiseMap, int nBins);
    void createCDF(std::vector<std::vector<float>>& noiseMap, int nBins);
    int cdf(float value);

    std::map<int, int> cdfTable;
    int cdfMin;
    int m_nBins;

    float noiseMin = -1.0;
    float noiseMax = 1.0;

};


