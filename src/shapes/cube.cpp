#include "cube.h"
#include "utils/sceneparser.h"
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <ostream>

void Cube::updateParams(int param1, RenderShapeData shape) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_shape = shape;
    setVertexData();
}

glm::mat4 getScalingMatrix(float sx, float sy, float sz) {
    // Task 1: return the transformation matrix for 3D scaling
    // Task 4: modify the function to return a 4x4 matrix
    glm::mat4 scaling = glm::mat4(0.f);
    scaling[0][0] = sx;
    scaling[1][1] = sy;
    scaling[2][2] = sz;
    scaling[3][3] = 1;
    return scaling;
}

glm::mat4 getRotationMatrixX(float angle) {
    // Task 2: return the matrix for rotation about the x-axis
    // Task 4: modify the function to return a 4x4 matrix
    auto radAngle = glm::radians(angle);
    return glm::mat4(1.f, 0, 0, 0,
                     0, cos(radAngle), sin(radAngle), 0,
                     0, -sin(radAngle), cos(radAngle), 0,
                     0, 0, 0, 1);
}

glm::mat4 getRotationMatrixY(float angle) {
    // Task 2: return the matrix for rotation about the y-axis
    // Task 4: modify the function to return a 4x4 matrix
    auto radAngle = glm::radians(angle);
    return glm::mat4(cos(radAngle), 0, -sin(radAngle), 0,
                     0, 1, 0, 0,
                     sin(radAngle), 0, cos(radAngle), 0,
                     0, 0, 0, 1);
}

glm::mat4 getRotationMatrixZ(float angle) {
    // Task 2: return the matrix for rotation about the z-axis
    // Task 4: modify the function to return a 4x4 matrix
    auto radAngle = glm::radians(angle);
    return glm::mat4(cos(radAngle), sin(radAngle), 0, 0,
                     -sin(radAngle), cos(radAngle), 0, 0,
                     0, 0, 1, 0,
                     0, 0, 0, 1);
}

glm::mat4 getTranslationMatrix(float dx, float dy, float dz) {
    // Task 3: return the matrix for translation
    glm::mat4 translate = glm::mat4(1.f);
    translate[3][0] = dx;
    translate[3][1] = dy;
    translate[3][2] = dz;
    return translate;
}


glm::vec3 calcNorm(glm::vec3 x, glm::vec3 u, glm::vec3 v) {
    return normalize(glm::cross(u - x, v - x));
}

glm::vec3 transformPoint(glm::vec3 point, int xOff, int yOff, int totalMeshes) {
    //point /= totalMeshes;
    point.x += xOff;
    point.y += yOff;
    return point;
}

void Cube::makeTile(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {
    // Task 2: create a tile (i.e. 2 triangles) based on 4 given points.
    // t1
    insertVec3(m_vertexData, topLeft, true);
    insertVec3(m_vertexData, calcNorm(topLeft, bottomLeft, bottomRight), false); // n1
    insertVec3(m_vertexData, bottomLeft, true);
    insertVec3(m_vertexData, calcNorm(bottomLeft, bottomRight, topLeft), false); // n2
    insertVec3(m_vertexData, bottomRight, true);
    insertVec3(m_vertexData, calcNorm(bottomRight, topLeft, bottomLeft), false); // n3
    // t2
    insertVec3(m_vertexData, topLeft, true);
    insertVec3(m_vertexData, calcNorm(topLeft, bottomRight, topRight), false); // n1
    insertVec3(m_vertexData, bottomRight, true);
    insertVec3(m_vertexData, calcNorm(bottomRight, topRight, topLeft), false); // n2
    insertVec3(m_vertexData, topRight, true);
    insertVec3(m_vertexData, calcNorm(topRight, topLeft, bottomRight), false); // n3
}

void Cube::makeFace(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {
    // Task 3: create a single side of the cube out of the 4
    //         given points and makeTile()
    // Note: think about how param 1 affects the number of triangles on
    //       the face of the cube
    float totalLength = glm::max(abs(topRight.x - bottomLeft.x), abs(topRight.y - bottomLeft.y));
    float singleTileLength = totalLength / m_param1;
    // Determine which face we are on and generate a CTM to transform
    // front facing tile to this plane
    glm::vec3 norm = calcNorm(topLeft, bottomLeft, bottomRight);
    glm::mat4 scale = getScalingMatrix(singleTileLength, singleTileLength, singleTileLength);
    glm::mat4 rot, trans;
    trans = glm::mat4(1.f);
    rot = glm::mat4(1.f);
    if (norm[0] == 1) {
        // right
        rot = getRotationMatrixZ(0) * getRotationMatrixY(90) * getRotationMatrixX(0);
        trans = getTranslationMatrix(totalLength / 2, -totalLength / 2, 0.5);
    } else if (norm[0] == -1) {
        // left
        rot = getRotationMatrixZ(0) * getRotationMatrixY(270) * getRotationMatrixX(0);
        trans = getTranslationMatrix(-totalLength / 2, -totalLength / 2, -totalLength + 0.5);
    } else if (norm[1] == 1) {
        // top
        rot = getRotationMatrixZ(0) * getRotationMatrixY(0) * getRotationMatrixX(270);
        trans = getTranslationMatrix(-totalLength / 2, totalLength / 2, 0.5);
    } else if (norm[1] == -1) {
        // bot
        rot = getRotationMatrixZ(0) * getRotationMatrixY(0) * getRotationMatrixX(90);
        trans = getTranslationMatrix(-totalLength / 2, -totalLength / 2, -totalLength + 0.5);
    } else if (norm[2] == 1) {
        // front
        trans = getTranslationMatrix(-totalLength / 2, -totalLength / 2, 0.5);
    } else if (norm[2] == -1) {
        // back
        rot = getRotationMatrixZ(0) * getRotationMatrixY(180) * getRotationMatrixX(0);
        trans = getTranslationMatrix(totalLength / 2,-totalLength / 2,-totalLength + 0.5);
    }
    glm::mat4 ctm = trans * rot * scale;

    // iterate through face
    for (int x = 0; x < m_param1; x++) {
        for (int y = 0; y < m_param1; y++) {
            // create tile on xy plane
            glm::vec4 tl = glm::vec4(x, y+1, 0, 1);
            glm::vec4 tr = glm::vec4(x+1, y+1, 0, 1);
            glm::vec4 bl = glm::vec4(x, y, 0, 1);
            glm::vec4 br = glm::vec4(x+1, y, 0, 1);
            // transform with ctm
            tl = ctm * tl;
            tr = ctm * tr;
            bl = ctm * bl;
            br = ctm * br;
            // create tile
            makeTile(tl, tr, bl, br);
        }
    }
}

void Cube::setVertexData() {
    // Uncomment these lines for Task 2, then comment them out for Task 3:

    //makeTile(glm::vec3(-0.5f,  0.5f, 0.5f),
    //         glm::vec3( 0.5f,  0.5f, 0.5f),
    //         glm::vec3(-0.5f, -0.5f, 0.5f),
    //         glm::vec3( 0.5f, -0.5f, 0.5f));

    // Uncomment these lines for Task 3:

    makeFace(glm::vec3(-0.5f,  0.5f, 0.5f),
             glm::vec3( 0.5f,  0.5f, 0.5f),
             glm::vec3(-0.5f, -0.5f, 0.5f),
             glm::vec3( 0.5f, -0.5f, 0.5f));

    // Task 4: Use the makeFace() function to make all 6 sides of the cube
    makeFace(glm::vec3(0.5f,  0.5f, 0.5f),
             glm::vec3( 0.5f,  0.5f, -0.5f),
             glm::vec3(0.5f, -0.5f, 0.5f),
             glm::vec3( 0.5f, -0.5f, -0.5f));

    makeFace(glm::vec3(-0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f,  0.5f, 0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f, 0.5f));

    makeFace(glm::vec3(0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f,  0.5f, -0.5f),
             glm::vec3(0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f));

    makeFace(glm::vec3(0.5f,  0.5f, 0.5f),
             glm::vec3(-0.5f,  0.5f, 0.5f),
             glm::vec3(0.5f, 0.5f,-0.5f),
             glm::vec3(-0.5f,0.5f, -0.5f));

    makeFace(glm::vec3(-0.5f, -0.5f, 0.5f),
             glm::vec3( 0.5f, -0.5f, 0.5f),
             glm::vec3(-0.5f, -0.5f,-0.5f),
             glm::vec3( 0.5f, -0.5f,-0.5f));
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cube::insertVec3(std::vector<float> &data, glm::vec3 v, bool isPos) {
    // move vector into world space via ctm
    glm::vec4 worldSpaceVec = m_shape.ctm * glm::vec4(v, (isPos) ? 1 : 0);
    if (!isPos) worldSpaceVec = normalize(worldSpaceVec);
    // store float data
    data.push_back(worldSpaceVec.x);
    data.push_back(worldSpaceVec.y);
    data.push_back(worldSpaceVec.z);
}
