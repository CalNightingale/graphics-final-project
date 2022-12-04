#include "sceneparser.h"
#include "scenefilereader.h"
#include "glm/gtx/transform.hpp"

#include <chrono>
#include <memory>
#include <iostream>

void buildCTM(SceneNode *curNode, glm::mat4 matrix, std::vector<RenderShapeData> *shapes) {
    // generate a matrix for this node's transformations
    glm::mat4 curTransforms = glm::mat4(1.0);
    for (SceneTransformation *t : curNode->transformations) {
        glm::mat4 curTransform;
        switch(t->type) {
        case TransformationType::TRANSFORMATION_TRANSLATE:
            curTransform = glm::translate(t->translate);
            break;
        case TransformationType::TRANSFORMATION_SCALE:
            curTransform = glm::scale(t->scale);
            break;
        case TransformationType::TRANSFORMATION_ROTATE:
            curTransform = glm::rotate(t->angle, t->rotate);
            break;
        case TransformationType::TRANSFORMATION_MATRIX:
            curTransform = t->matrix;
            break;
        }
        curTransforms *= curTransform;
    }
    //std::cout << glm::to_string(curTransforms) << std::endl;
    for (ScenePrimitive *prim : curNode->primitives) {
        RenderShapeData curData = {*prim, matrix * curTransforms};
        shapes->push_back(curData);
    }
        //std::cout << "children found. handling" << std::endl;
        // there are children to handle, recurse through and deal with them
    for (SceneNode *newNode : curNode->children) {
        buildCTM(newNode, matrix * curTransforms, shapes);
    }
}

bool SceneParser::parse(std::string filepath, RenderData &renderData) {
    ScenefileReader fileReader = ScenefileReader(filepath);
    bool success = fileReader.readXML();
    if (!success) {
        return false;
    }

    // Task 4: populate renderData with global data, lights, and camera data
    renderData.cameraData = fileReader.getCameraData();
    renderData.globalData = fileReader.getGlobalData();
    renderData.lights = fileReader.getLights();

    // Task 5: populate renderData's list of primitives and their transforms.
    //         This will involve traversing the scene graph, and we recommend you
    //         create a helper function to do so!
    renderData.shapes.clear();
    buildCTM(fileReader.getRootNode(), glm::mat4(1.0), &renderData.shapes);
    return true;
}
