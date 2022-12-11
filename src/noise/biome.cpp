#include "biome.h"
#include "filters/filterutils.h"
#include "rgba.h"
#include <algorithm>
#include <cmath>
#include <iostream>

Biome::Biome(){
    simplex = Simplex();
}

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
//    int sum = 0;
//    for (int i = 0; i < nBins; i++) {
//        std::cout << histogram[i] << " ";
//        sum += histogram[i];
//    }
//    std::cout << std::endl;
//    std::cout << sum << std::endl;
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

//    for (int i = 0; i < nBins; i++) {
//        std::cout << cdfTable[i] << " ";
//    }
//    std::cout << std::endl;

//    std::cout << cdfMin << std::endl;
}

int Biome::cdf(float value) {
    if (value < noiseMin || value > noiseMax) return 0;
    float binSize = (noiseMax - noiseMin) / m_nBins;
    int binIndex = round((value - noiseMin) / binSize);
    return cdfTable[binIndex];
}

float Biome::equalizeValue(float value, int size) {
    return (((float)cdf(value) - (float)cdfMin) / ((float)m_size * (float)m_size - (float)cdfMin)) * 2.0 - 1.0;
}

void Biome::histogramEqualize(std::vector<std::vector<float>>& noiseMap) {
    createCDF(noiseMap, 64);
    m_size = noiseMap[0].size();
    for (auto& row: noiseMap) {
        for (auto& value: row) {
            value = equalizeValue(value, m_size);
//            std::cout << equalizeValue(value, noiseMap[0].size()) << " ";
        }
//        std::cout << std::endl;
    }

//    std::map<int, int> newHist = createHistogram(noiseMap, 64);
//    for (int i = 0; i < 64; i++) {
//        std::cout << newHist[i] << " ";
//    }
//    std::cout << std::endl;
}

std::vector<std::vector<float>> Biome::createTemperatureMap(int size, int seed) {
    std::vector<std::vector<float>> tempMap = simplex.noiseMap(size, 2, seed*2);
    histogramEqualize(tempMap);
    std::vector<std::vector<float>> outputMap;
    for (auto& row: tempMap) {
        std::vector<float> outputRow;
        for (auto& value: row) {
            outputRow.push_back((value * 0.5 + 0.5));
        }
        outputMap.push_back(outputRow);
    }
    return outputMap;
}

std::vector<std::vector<float>> Biome::createPreciptiationMap(int size, int seed) {
    return createTemperatureMap(size, seed + 1);
}

std::vector<std::vector<float>> Biome::createHeightMap(int size, int seed) {
    std::vector<std::vector<float>> heightMap = simplex.noiseMap(size, 4, seed, 6, 0.5, 2);
    return heightMap;
}

std::vector<float> fiveByFiveBlur() {
    std::vector<float> blurKernel;
    blurKernel.resize(2 * 2 + 1);
    float kernelSum = 0; // for normalization
    for (int kerIndex = 0; kerIndex < blurKernel.size(); kerIndex++) {
        float xSquared =  pow(2.0 - kerIndex, 2);
        float sigmaSquared = pow(16.0, 2);
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

std::vector<std::vector<float>> Biome::createLandMask(int size, int seed) {
    std::vector<std::vector<float>> heightMap = simplex.noiseMap(size, 4, seed, 6, 0.5, 2);
    for (auto& row: heightMap) {
        for (auto& value: row) {
            if (value < 0) {
                value = 0.0; // set all negative values to 0. All 0 values are water
            } else {
                value = 1.0;
            }
        }
    }
    return heightMap;
}

void Biome::blurMask(std::vector<std::vector<float>>& mask) {
    int n = mask.size();
    std::vector<RGBA> maskFlattened;
    for (auto& row: mask) {
        for (auto& value: row) {
            std::uint8_t color = (int)value*255;
            maskFlattened.push_back(RGBA{color, color, color});
        }
    }
    std::vector<std::vector<float>> blurredMask;
    std::vector<float> blurKernel = fiveByFiveBlur(); // no need to flip the gaussian kernel since it is vertically symmetric
    std::vector<RGBAFloat> dataFloat = FilterUtils::convolve2D(maskFlattened, n, n, blurKernel, blurKernel);
    for (int r = 0; r < n; r++) {
        std::vector<float> row;
        for (int c = 0; c < n; c++) {
            row.push_back(dataFloat[r*n+c].r/255.0);
        }
        blurredMask.push_back(row);
    }

    mask = blurredMask;
}


std::vector<std::vector<float>> Biome::createSmoothHeightMap(int size, int seed) {
    std::vector<std::vector<float>> heightMap = simplex.noiseMap(size, 4, seed, 1, 0.5, 2);
    return heightMap;
}

std::function<glm::vec2 (float)> Biome::bezier(float x1, float y1, float x2, float y2, float a) {
    return[=](float t) {
        glm::vec2 p1 = glm::vec2(0.0, 0.0);
        glm::vec2 p2 = glm::vec2(x1, y1);
        glm::vec2 p3 = glm::vec2(x2, y2);
        glm::vec2 p4 = glm::vec2(1.0, a);
        return (1-t)*(1-t)*(1-t) * p1 + 3*(1-t)*(1-t)*t * p2 + 3*(1-t)*t*t * p3 + t*t*t * p4;
    };
}

std::function<std::vector<float> (std::vector<float>&)> Biome::interpolate1D(std::vector<float>& xs, std::vector<float>& ys) {
    int n = xs.size();
    return[=](std::vector<float>& xsToInterpolate) {
        std::vector<float> output;

        for (float& x: xsToInterpolate) {
            int i = 0;
            if (x >= xs[n - 2]) {
                i = n - 2;
            } else {
                while (x > xs[i+1]) i++;
            }

            float xL = xs[i], yL = ys[i], xR = xs[i+1], yR = ys[i+1];
            if ( x < xL ) yR = yL;
            if ( x > xR ) yL = yR;
            float dydx = ( yR - yL ) / ( xR - xL );
            output.push_back(yL + dydx * (x - xL));
        }
        return output;
    };
}

std::function<std::vector<float> (std::vector<float>&)> Biome::bezierInterpolate(float x1, float y1, float x2, float y2, float a) {
    std::vector<float> ts;
    for (int i = 0; i < 256; i++) {
        ts.push_back((float)i * 1.0/255.0);
    }
    auto f = bezier(x1, y1, x2, y2, a);
    std::vector<float> bezierXs;
    std::vector<float> bezierYs;
    for (int i = 0; i < ts.size(); i++) {
//        std::cout << ts[i] << std::endl;
        glm::vec2 point = f(ts[i]);
        bezierXs.push_back(point.x);
        bezierYs.push_back(point.y);
//        std::cout << point.x << " " << point.y << std::endl;
    }


    return interpolate1D(bezierXs, bezierYs);
}

std::vector<std::vector<float>> Biome::filterMap(std::vector<std::vector<float>>& hMap, std::vector<std::vector<float>>& smoothMap, float x1, float y1, float x2, float y2, float a, float b) {
    std::vector<std::vector<float>> outputMap;
    for (int i = 0; i < hMap.size(); i++) {
        std::vector<float> outputRow;
        for (int j = 0; j < hMap[0].size(); j++) {
            float outputValue = b * hMap[i][j] + (1-b) * smoothMap[i][j];
            outputValue = std::min(1.0f, std::max(0.0f, outputValue));
            outputRow.push_back(outputValue);
        }
        outputMap.push_back(outputRow);
    }

    auto f = bezierInterpolate(x1, y1, x2, y2, a);

    std::vector<std::vector<float>> finalOutputMap;
    for (auto& row: outputMap) {
        finalOutputMap.push_back(f(row));
    }

    return finalOutputMap;
}

std::vector<std::vector<float>> Biome::createBiomeHeightMap(int size, int seed, int biome) {
    float x1 = 0;
    float x2 = 0;
    float y1 = 0;
    float y2 = 0;
    float a = 0;
    float b = 0;

    switch(biome) {
        case 0: // Desert
        x1 = 0.75; y1 = 0.2; x2 = 0.95; y2 = 0.2; a = 0.2; b = 0.5;
        break;
        case 1: // Savanna
        x1 = 0.5; y1 = 0.1; x2 = 0.95; y2 = 0.1; a = 0.1; b = 0.2;
        break;
        case 2: // Tropical Woodland
        x1 = 0.33; y1 = 0.33; x2 = 0.95; y2 = 0.1; a = 0.1; b = 0.75;
        break;
        case 3: // Tundra
        x1 = 0.5; y1 = 1.0; x2 = 0.25; y2 = 1.0; a = 1.0; b = 1.0;
        break;
        case 4: // Seasonal Forest
        x1 = 0.75; y1 = 0.5; x2 = 0.4; y2 = 0.4; a = 0.33; b = 0.2;
        break;
        case 5: // Rainforest
        x1 = 0.5; y1 = 0.25; x2 = 0.66; y2 = 1.0; a = 1.0; b = 0.5;
        break;
        case 6: // Temperate Forest
        x1 = 0.75; y1 = 0.5; x2 = 0.4; y2 = 0.4; a = 0.33; b = 0.33;
        break;
        case 7: // Temperate Rainforest
        x1 = 0.75; y1 = 0.5; x2 = 0.4; y2 = 0.4; a = 0.33; b = 0.33;
        break;
        case 8: // Boreal
        x1 = 0.8; y1 = 0.1; x2 = 0.9; y2 = 0.05; a = 0.05; b = 0.1;
        break;
    }

//    std::cout << biome << " " << x1 << std::endl;

    std::vector<std::vector<float>> heightMap = createHeightMap(size, seed);
    std::vector<std::vector<float>> smoothHeightMap = createSmoothHeightMap(size, seed);
    return filterMap(heightMap, smoothHeightMap, x1, y1, x2, y2, a, b);
}

std::vector<std::vector<float>> Biome::multiplyMasks(std::vector<std::vector<float>>& mask1, std::vector<std::vector<float>>& mask2) {
    std::vector<std::vector<float>> outputMask;
    for (int r = 0; r < mask1.size(); r++) {
        std::vector<float> row;
        for (int c = 0; c < mask1[0].size(); c++) {
            row.push_back(mask1[r][c] * mask2[r][c]);
        }
        outputMask.push_back(row);
    }
    return outputMask;
}

std::vector<std::vector<float>> Biome::applyMask(std::vector<std::vector<float>>& originalMap, std::vector<std::vector<float>>& newMap, std::vector<std::vector<float>> mask) {
    std::vector<std::vector<float>> outputMask;
    for (int r = 0; r < originalMap.size(); r++) {
        std::vector<float> row;
        for (int c = 0; c < originalMap[0].size(); c++) {
            row.push_back(((1 - mask[r][c]) * originalMap[r][c]) + (mask[r][c] * newMap[r][c]));
        }
        outputMask.push_back(row);
    }
    return outputMask;
}


