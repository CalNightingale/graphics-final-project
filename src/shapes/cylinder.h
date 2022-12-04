#pragma once

#include "utils/sceneparser.h"
#include <vector>
#include <glm/glm.hpp>

class Cylinder
{
public:
    void updateParams(int param1, int param2, RenderShapeData shape);
    std::vector<float> generateShape() { return m_vertexData; }

private:
    void insertVec3(std::vector<float> &data, glm::vec3 v, bool isPos);
    void setVertexData();
    void makeTile(glm::vec3 topLeft,
                  glm::vec3 topRight,
                  glm::vec3 bottomLeft,
                  glm::vec3 bottomRight,
                  bool onFace);
    glm::vec3 calcNorm(glm::vec3 pos);
    void makeFace(float z);
    std::vector<float> m_vertexData;
    int m_param1;
    int m_param2;
    RenderShapeData m_shape;
    float m_radius = 0.5;
};
