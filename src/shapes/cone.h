#pragma once

#include "utils/sceneparser.h"
#include <vector>
#include <glm/glm.hpp>

class Cone
{
public:
    void updateParams(int param1, int param2, RenderShapeData shape);
    std::vector<float> generateShape() { return m_vertexData; }

private:
    void insertVec3(std::vector<float> &data, glm::vec3 v, bool isPos);
    void makeFace(float y);
    void setVertexData();
    glm::vec3 calcNorm(glm::vec3 pos);
    glm::vec3 cylToCart(glm::vec3 v);
    void makeTile(glm::vec3 topLeft,
                            glm::vec3 topRight,
                            glm::vec3 bottomLeft,
                            glm::vec3 bottomRight,
                            bool onFace);

    std::vector<float> m_vertexData;
    int m_param1;
    int m_param2;
    RenderShapeData m_shape;
    float m_radius = 0.5;
};
