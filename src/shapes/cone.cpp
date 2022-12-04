#include "./src/shapes/cone.h"
#include "utils/sceneparser.h"

void Cone::updateParams(int param1, int param2, RenderShapeData shape) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    m_shape = shape;
    setVertexData();
}

// takes vector in spherical coordinates, returns cartesian
glm::vec3 Cone::cylToCart(glm::vec3 v) {
    return glm::vec3(v[0]*cos(v[1]), v[2], v[0]*sin(v[1]));
}

glm::vec3 Cone::calcNorm(glm::vec3 pos) {
    float yx = pos.x / sqrt(pos.x*pos.x + pos.z*pos.z);
    float yz = pos.z / sqrt(pos.x*pos.x + pos.z*pos.z);
    return normalize(glm::vec3(yx, 1, yz));
}

void Cone::makeTile(glm::vec3 topLeft,
                        glm::vec3 topRight,
                        glm::vec3 bottomLeft,
                        glm::vec3 bottomRight,
                        bool onFace) {
    // Task 2: create a tile (i.e. 2 triangles) based on 4 given points.
    // t1
    insertVec3(m_vertexData, topLeft, true);
    insertVec3(m_vertexData, (onFace) ? glm::vec3(0,-1,0) : calcNorm(topLeft), false); // n1
    insertVec3(m_vertexData, bottomLeft, true);
    insertVec3(m_vertexData, (onFace) ? glm::vec3(0,-1,0) : calcNorm(bottomLeft), false); // n2
    insertVec3(m_vertexData, bottomRight, true);
    insertVec3(m_vertexData, (onFace) ? glm::vec3(0,-1,0) : calcNorm(bottomRight), false); // n3
    // t2
    insertVec3(m_vertexData, topLeft, true);
    insertVec3(m_vertexData, (onFace) ? glm::vec3(0,-1,0) : calcNorm(topLeft), false); // n1
    insertVec3(m_vertexData, bottomRight, true);
    insertVec3(m_vertexData, (onFace) ? glm::vec3(0,-1,0) : calcNorm(bottomRight), false); // n2
    insertVec3(m_vertexData, topRight, true);
    insertVec3(m_vertexData, (onFace) ? glm::vec3(0,-1,0) : calcNorm(topRight), false); // n3
}

void Cone::makeFace(float y) {
    // face is composed of m_param2 triangles
    float thetaStep = (M_PI*2) / m_param2;
    glm::vec3 n = glm::vec3(0, (y > 0) ? 1 : -1, 0);
    // make center triangle
    for (float theta = 0; theta < 2*M_PI; theta += thetaStep) {
        // generate the three vertices of the triangle
        glm::vec3 v1 = cylToCart(glm::vec3(0.5 / m_param1, theta, y));
        // ensure proper counterclockwise ordering of vertices
        glm::vec3 v2 = cylToCart(glm::vec3(0.5 / m_param1, (y < 0) ? theta + thetaStep : theta - thetaStep, y));
        // final vertex is just origin at proper y value
        glm::vec3 v3 = cylToCart(glm::vec3(0, theta, y));
        insertVec3(m_vertexData, v1, true);
        insertVec3(m_vertexData, n, false);
        insertVec3(m_vertexData, v2, true);
        insertVec3(m_vertexData, n, false);
        insertVec3(m_vertexData, v3, true);
        insertVec3(m_vertexData, n, false);
    }
    // make outer trapezoids
    float radStep = 0.5 / m_param1;
    for (float theta = 0; theta < 2*M_PI; theta += thetaStep) {
        for (int i = 0; i < m_param1 - 1; i++) {
            float outerRad = 0.5 - radStep*i;
            float innerRad = 0.5 - radStep*(i+1);
            glm::vec3 topLeft = cylToCart(glm::vec3(innerRad, theta, y));
            glm::vec3 bottomLeft = cylToCart(glm::vec3(outerRad, theta, y));
            glm::vec3 topRight = cylToCart(glm::vec3(innerRad, (y < 0) ? theta + thetaStep : theta - thetaStep, y));
            glm::vec3 bottomRight = cylToCart(glm::vec3(outerRad, (y < 0) ? theta + thetaStep : theta - thetaStep, y));
            makeTile(topLeft, topRight, bottomLeft, bottomRight, true);
        }
    }
}

void Cone::setVertexData() {
    // TODO for Project 5: Lights, Camera
    // base
    makeFace(-0.5);

    // top
    float thetaStep = (M_PI*2) / m_param2;
    float yStep = 1.f / m_param1;
    // do top triangles
    for (float theta = 0; theta < 2*M_PI; theta += thetaStep) {
        // top vertex is tip of cone
        glm::vec3 v1 = cylToCart(glm::vec3(0, theta, 0.5));
        // other two are down yStep, radius yStep/2
        glm::vec3 v2 = cylToCart(glm::vec3(yStep / 2, theta, 0.5 - yStep));
        glm::vec3 v3 = cylToCart(glm::vec3(yStep / 2, theta - thetaStep, 0.5 - yStep));
        insertVec3(m_vertexData, v1, true);
        // for tip normal, generate a point horizontally cenetered on the face
        insertVec3(m_vertexData, calcNorm(cylToCart(glm::vec3(yStep / 2, (theta + (theta - thetaStep)) / 2, 0.5 - yStep))), false);
        insertVec3(m_vertexData, v2, true);
        insertVec3(m_vertexData, calcNorm(v2), false);
        insertVec3(m_vertexData, v3, true);
        insertVec3(m_vertexData, calcNorm(v3), false);
    }

    // do trapezoids
    for (float theta = 0; theta < 2*M_PI; theta += thetaStep) {
        for (int i = 0; i < m_param1 - 1; i++) {
            float botY = i*yStep - 0.5;
            float topY = (i+1)*yStep - 0.5;
            float botR = 0.5 - i*yStep/2;
            float topR = 0.5 - (i+1)*yStep/2;
            glm::vec3 topLeft = cylToCart(glm::vec3(topR, theta, topY));
            glm::vec3 bottomLeft = cylToCart(glm::vec3(botR, theta, botY));
            glm::vec3 topRight = cylToCart(glm::vec3(topR, theta - thetaStep, topY));
            glm::vec3 bottomRight = cylToCart(glm::vec3(botR, theta - thetaStep, botY));
            makeTile(topLeft, topRight, bottomLeft, bottomRight, false);
        }
    }
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cone::insertVec3(std::vector<float> &data, glm::vec3 v, bool isPos) {
    // move vector into world space via ctm
    glm::vec4 worldSpaceVec = m_shape.ctm * glm::vec4(v, (isPos) ? 1 : 0);
    if (!isPos) worldSpaceVec = normalize(worldSpaceVec);
    // store float data
    data.push_back(worldSpaceVec.x);
    data.push_back(worldSpaceVec.y);
    data.push_back(worldSpaceVec.z);
}
