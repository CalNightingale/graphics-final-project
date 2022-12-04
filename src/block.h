#pragma once

#include <glm/glm.hpp>

enum blockType { Grass, Dirt, Stone, Water, Snow };

struct Block {
    glm::vec3 pos;
    enum blockType type;
};
