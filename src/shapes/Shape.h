#pragma once

#include "utils/sceneparser.h"
#include <glm/glm.hpp>


struct Shape {
    RenderShapeData data;
    glm::mat3 inverseCTM;
};
