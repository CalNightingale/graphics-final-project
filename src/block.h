#pragma once

#include "utils/scenedata.h"
#include <glm/glm.hpp>

enum blockType { Grass, Dirt, Stone, Water, Snow };

const SceneColor blockSpecular = SceneColor{1,1,1,1};
const float blockShininess = 30;

struct Block {
    glm::vec3 pos;
    enum blockType type;
};
