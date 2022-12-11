#pragma once

#include "utils/scenedata.h"
#include <glm/glm.hpp>

// Define block material properties that remain consistent
const SceneColor blockAmbient = SceneColor{0.5,0.5,0.5,1};
const SceneColor blockSpecular = SceneColor{1,1,1,1};
const float blockShininess = 30;

struct Block {
    glm::vec3 pos;
    SceneColor color;
};
