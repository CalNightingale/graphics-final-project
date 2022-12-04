#pragma once

#include <glm/glm.hpp>

enum blockType { Sunday, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday };

struct Block {
    glm::vec3 pos;
    enum blockType type;
};
