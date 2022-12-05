#pragma once

#include "utils/sceneparser.h"
#include <vector>
#include <glm/glm.hpp>

class Cube
{
public:
    void updateParams(int param1, RenderShapeData shape);
    std::vector<float> generateShape() { return m_vertexData; }
    static glm::mat4 getTranslationMatrix(float dx, float dy, float dz);

private:
    void insertVec3(std::vector<float> &data, glm::vec3 v, bool isPos);
    void setVertexData();
    void makeTile(glm::vec3 topLeft,
                  glm::vec3 topRight,
                  glm::vec3 bottomLeft,
                  glm::vec3 bottomRight);
    void makeFace(glm::vec3 topLeft,
                  glm::vec3 topRight,
                  glm::vec3 bottomLeft,
                  glm::vec3 bottomRight);

    std::vector<float> m_vertexData;
    int m_param1;
    RenderShapeData m_shape;
};
