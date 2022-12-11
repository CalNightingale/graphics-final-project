#pragma once

#include <vector>
class Simplex
{
public:
    Simplex();
    std::vector<std::vector<float>> noiseMap(int size, int res, int seed, int octaves=1, float persistence=0.5, float lacunarity=2.0);


private:
    int fastFloor(float x);
    float dot(const int g[3], float x, float y);
    int p[256];
    static constexpr int grad3[12][3] = {{1,1,0},{-1,1,0},{1,-1,0},{-1,-1,0},{1,0,1},{-1,0,1},{1,0,-1},{-1,0,-1},{0,1,1},{0,-1,1},{0,1,-1},{0,-1,-1}};
    int perm[512];
    float baseNoise(float x, float y);
    float noise(float x, float y, float scale, float octaves, float persistence, float lacunarity);

};
