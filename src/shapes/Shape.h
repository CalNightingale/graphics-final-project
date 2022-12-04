#pragma once

#include "utils/sceneparser.h"
#include <glm/glm.hpp>
#include <GL/glew.h>


struct Shape {
    RenderShapeData data;
    std::vector<float> vertexData;
};
