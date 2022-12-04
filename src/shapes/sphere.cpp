#include "./src/shapes/sphere.h"
#include "utils/sceneparser.h"
#include <iostream>

void Sphere::updateParams(int param1, int param2, RenderShapeData shape) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    m_shape = shape;
    setVertexData();
}

glm::vec3 calcNorm(glm::vec3 surfacePt) {
    // for spheres, normal is direction from origin to surface point
    // this assumes origin is 0,0,0
    return normalize(surfacePt);
}

// takes vector in spherical coordinates, returns cartesian
glm::vec3 sphereicalToCartesian(glm::vec3 v) {
    return glm::vec3(v[0]*sin(v[2])*cos(v[1]), v[0]*sin(v[2])*sin(v[1]), v[0]*cos(v[2]));
}

void Sphere::makeTile(glm::vec3 topLeft,
                      glm::vec3 topRight,
                      glm::vec3 bottomLeft,
                      glm::vec3 bottomRight) {
    insertVec3(m_vertexData, topLeft, true);
    insertVec3(m_vertexData, calcNorm(topLeft), false); // n1

    insertVec3(m_vertexData, bottomLeft, true);
    insertVec3(m_vertexData, calcNorm(bottomLeft), false); // n2


    insertVec3(m_vertexData, bottomRight, true);
    insertVec3(m_vertexData, calcNorm(bottomRight), false); // n3


    // t2
    insertVec3(m_vertexData, topLeft, true);
    insertVec3(m_vertexData, calcNorm(topLeft), false); // n1
    insertVec3(m_vertexData, bottomRight, true);
    insertVec3(m_vertexData, calcNorm(bottomRight), false); // n2
    insertVec3(m_vertexData, topRight, true);
    insertVec3(m_vertexData, calcNorm(topRight), false); // n3
}

void Sphere::makeWedge(float currentTheta, float nextTheta) {
    // Task 6: create a single wedge of the sphere using the
    //         makeTile() function you implemented in Task 5
    // Note: think about how param 1 comes into play here!
    // get height angle in radians
    float wedgeHeightAngle = glm::radians(180.f / m_param1);
    for(float heightStart = 0; heightStart < M_PI; heightStart += wedgeHeightAngle) {
        glm::vec3 topLeft = sphereicalToCartesian(glm::vec3(0.5, currentTheta, heightStart));
        glm::vec3 topRight = sphereicalToCartesian(glm::vec3(0.5, nextTheta, heightStart));
        glm::vec3 bottomLeft = sphereicalToCartesian(glm::vec3(0.5, currentTheta, heightStart+wedgeHeightAngle));
        glm::vec3 bottomRight = sphereicalToCartesian(glm::vec3(0.5, nextTheta, heightStart+wedgeHeightAngle));
        makeTile(topLeft, topRight, bottomLeft, bottomRight);
    }
}

void Sphere::makeSphere() {
    // Task 7: create a full sphere using the makeWedge() function you
    //         implemented in Task 6
    // Note: think about how param 2 comes into play here!
    float wedgeWidthAngle = (M_PI*2) / m_param2;
    for(float theta = 0; theta < M_PI*2; theta += wedgeWidthAngle) {
        makeWedge(theta, theta + wedgeWidthAngle);
    }
}

void Sphere::setVertexData() {
    makeSphere();
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Sphere::insertVec3(std::vector<float> &data, glm::vec3 v, bool isPos) {
    // move vector into world space via ctm
    glm::vec4 worldSpaceVec = m_shape.ctm * glm::vec4(v, (isPos) ? 1 : 0);
    if (!isPos) worldSpaceVec = normalize(worldSpaceVec);
    // store float data
    data.push_back(worldSpaceVec.x);
    data.push_back(worldSpaceVec.y);
    data.push_back(worldSpaceVec.z);
}
