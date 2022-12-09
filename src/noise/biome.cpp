#include "biome.h"
#include <algorithm>
#include <cmath>

std::map<int, int> Biome::createHistogram(std::vector<std::vector<float>>& noiseMap, int nBins) {
    std::vector<float> noiseMapFlattened;
    std::map<int, int> histogram;

    for (auto& row: noiseMap) {
        for (float entry: row) {
            noiseMapFlattened.push_back(entry);
        }
    }


    float binSize = (noiseMax - noiseMin) / nBins;

    std::sort(noiseMapFlattened.begin(), noiseMapFlattened.end());

    int currentIndex = 0;
    for (int i = 0; i < nBins; i++) {
        if (currentIndex >= noiseMapFlattened.size()) break;

        float binStart = noiseMin + binSize * i;
        float binEnd = noiseMin + binSize * (i + 1);
        int binCount = 0;

        // Skip to the first element within the bin
        while (noiseMapFlattened[currentIndex] < binStart) {
            currentIndex += 1;
        }

        // Add elements to the bin unitl we exceed
        while(currentIndex < noiseMapFlattened.size() && noiseMapFlattened[currentIndex] <= binEnd)  {
            binCount += 1;
            currentIndex += 1;
        }

        histogram[i] = binCount;

    }

    return histogram;
}

void Biome::createCDF(std::vector<std::vector<float>>& noiseMap, int nBins) {
    std::map<int, int> histogram = createHistogram(noiseMap, nBins);
    m_nBins = nBins;
    cdfTable.clear();

    int cdfAcc = 0;
    bool cdfMinSet = false;
    for (int i = 0; i < nBins; i++) {
        cdfAcc += histogram[i];
        if (!cdfMinSet && cdfAcc > 0) {
            cdfMin = cdfAcc;
            cdfMinSet = true;
        }

        cdfTable[i] = cdfAcc;
    }
}

int Biome::cdf(float value) {
    if (value < noiseMin || value > noiseMax) return 0;
    float binSize = (noiseMax - noiseMin) / m_nBins;
    int binIndex = round((value - noiseMin) / binSize);
    return cdfTable[binIndex];
}
