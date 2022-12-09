#pragma once

#include "utils/sceneparser.h"
#include <glm/glm.hpp>


struct Shape {
    RenderShapeData data;
    std::vector<float> vertexData;
};
