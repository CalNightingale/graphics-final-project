#include "glm/gtx/transform.hpp"
#include "realtime.h"
#include <iostream>
#include "debug.h"
#include "./utils/shaderloader.h"
#include <math.h>
#include "noise/simplex.h"


void loadVectorAsCubemap(GLuint &textureId, std::vector<glm::vec4> mapData, int textureSize){
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    Debug::glErrorCheck();


    // to prevent seams
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    Debug::glErrorCheck();

    for (int i = 0; i < 6; i++){


        int TEXTURE_SIZE = textureSize;


        if (true)
        {
            glTexImage2D
            (
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0,
                GL_RGBA,
                TEXTURE_SIZE,
                TEXTURE_SIZE,
                0,
                GL_RGBA,
                GL_FLOAT,
                mapData.data()
            );
        }
        else
        {
            std::cout << "Failed to load texture: " << std::endl;
        }
    }

}

void Realtime::setupSkybox(){
    //Debug::glErrorCheck();

    m_verticiesVector.clear();


    float skyboxVertices[] =
    {
        //   Coordinates
        -1.0f, -1.0f,  1.0f,//        7--------6
         1.0f, -1.0f,  1.0f,//       /|       /|
         1.0f, -1.0f, -1.0f,//      4--------5 |
        -1.0f, -1.0f, -1.0f,//      | |      | |
        -1.0f,  1.0f,  1.0f,//      | 3------|-2
         1.0f,  1.0f,  1.0f,//      |/       |/
         1.0f,  1.0f, -1.0f,//      0--------1
        -1.0f,  1.0f, -1.0f
    };

    unsigned int skyboxIndices[] =
    {
        // Right
        1, 6, 2,
        6, 1, 5,
        // Left
        0, 7, 4,
        7, 0, 3,
        // Top
        4, 6, 5,
        6, 4, 7,
        // Bottom
        0, 2, 3,
        2, 0, 1,
        // Back
        0, 5, 1,
        5, 0, 4,
        // Front
        3, 6, 7,
        6, 3, 2
    };

    for(int i : skyboxIndices){
        m_verticiesVector.push_back(skyboxVertices[i*3]);
        m_verticiesVector.push_back(skyboxVertices[i*3 + 1]);
        m_verticiesVector.push_back(skyboxVertices[i*3 + 2]);
    }

    Debug::glErrorCheck();
    glGenVertexArrays(1, &m_skyboxVAO);
    Debug::glErrorCheck();
    glGenBuffers(1, &m_skyboxVBO);
    Debug::glErrorCheck();

    glBindBuffer(GL_ARRAY_BUFFER, m_skyboxVBO);
    Debug::glErrorCheck();
    glBufferData(GL_ARRAY_BUFFER, m_verticiesVector.size()*sizeof(GLfloat), m_verticiesVector.data(), GL_STATIC_DRAW);
    Debug::glErrorCheck();

    glBindVertexArray(m_skyboxVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    Debug::glErrorCheck();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    Debug::glErrorCheck();


    QString imageFolder = ":/resources/images/Yokohama/";

    QString facesCubemap[6] =
    {
        imageFolder + "posx.jpg",
        imageFolder + "negx.jpg",
        imageFolder + "posy.jpg",
        imageFolder + "negy.jpg",
        imageFolder + "posz.jpg",
        imageFolder + "negz.jpg"
    };

    glGenTextures(1, &m_skyboxTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxTexture);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    Debug::glErrorCheck();


    // to prevent seams
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    Debug::glErrorCheck();

    for (int i = 0; i < 6; i++){

//        QImage textureImage = QImage(facesCubemap[i]);

//        textureImage = textureImage.convertToFormat(QImage::Format_RGBA8888);

//        uchar* data = textureImage.bits();

        int TEXTURE_SIZE = 100;

        std::vector<glm::vec4> cloudmap{};
        cloudmap.reserve(TEXTURE_SIZE*TEXTURE_SIZE);

        Simplex simplex = Simplex();
        std::vector<std::vector<float>> cloudNoise = simplex.noiseMap(TEXTURE_SIZE, settings.cloudSize, 0, 1, 0.5, 2.0);
        std::vector<float> cloudNoiseFlattened;
        for(auto& row: cloudNoise) {
            for (auto& value: row) {
                cloudNoiseFlattened.push_back(value);
            }
        }


        for(int i = 0; i < TEXTURE_SIZE*TEXTURE_SIZE; i++){
            long randNum = random();
            randNum %= 1000;
            if (cloudNoiseFlattened[i] > 0.2){
                cloudmap.push_back(glm::vec4(1,1,1,1));
            } else {
                cloudmap.push_back(glm::vec4(0,0,0,1));
            }
        }

        if (true)
        {
            glTexImage2D
            (
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0,
                GL_RGBA,
                TEXTURE_SIZE,
                TEXTURE_SIZE,
                0,
                GL_RGBA,
                GL_FLOAT,
                cloudmap.data()
            );
        }
        else
        {
            std::cout << "Failed to load texture: " << std::endl;
        }
    }


    glGenTextures(1, &m_starTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_starTexture);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    Debug::glErrorCheck();


    // to prevent seams
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    Debug::glErrorCheck();

    for (int i = 0; i < 6; i++){
        int TEXTURE_SIZE = 1000;

        std::vector<glm::vec4> starmap{};
        starmap.reserve(TEXTURE_SIZE*TEXTURE_SIZE);
        for(int i = 0; i < TEXTURE_SIZE*TEXTURE_SIZE; i++){
            long randNum = random();
            randNum %= 1000;

            if(randNum < 10){
                starmap.push_back(glm::vec4(1,1,1,1));
            } else {
                starmap.push_back(glm::vec4(0,0,0,1));
            }
        }

        if (true)
        {
            glTexImage2D
            (
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0,
                GL_RGBA,
                TEXTURE_SIZE,
                TEXTURE_SIZE,
                0,
                GL_RGBA,
                GL_FLOAT,
                starmap.data()
            );
        }
        else
        {
            std::cout << "Failed to load texture: " << std::endl;
        }
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    m_skybox_shader = ShaderLoader::createShaderProgram(":/resources/shaders/skybox.vert", ":/resources/shaders/skybox.frag");

    Debug::glErrorCheck();
}





void Realtime::paintSkybox(){
    // activate the skybox shader
    glDepthFunc(GL_LEQUAL);
    glUseProgram(m_skybox_shader);
    float timeRatio = (float) time / (float) dayLength;


    if(timeRatio < .5){
        float linInterp = (timeRatio  / .5);
        skyColor = (linInterp * dayColor) + (1-linInterp)*nightColor;
    } else{
        float linInterp = ((timeRatio - .5) / (1 - .5));
        skyColor = (linInterp * nightColor) + (1-linInterp)*dayColor;
    }



    glm::mat4 skyboxView = glm::mat4(glm::mat3(m_view));
    glUniformMatrix4fv(glGetUniformLocation(m_skybox_shader, "view"), 1, GL_FALSE, &skyboxView[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_skybox_shader, "projection"), 1, GL_FALSE, &m_proj[0][0]);
    glUniform4fv(glGetUniformLocation(m_skybox_shader, "skyColor"), 1, &skyColor[0]);
    glUniform4fv(glGetUniformLocation(m_skybox_shader, "lightPos"), 1, &m_sceneData.lights.at(0).pos[0]);

    glUniform1i(glGetUniformLocation(m_skybox_shader, "skybox"), 0);
    glUniform1i(glGetUniformLocation(m_skybox_shader, "starTexture"), 1);
    glUniform1f(glGetUniformLocation(m_skybox_shader, "time"), float(time));



    glUniform1f(glGetUniformLocation(m_skybox_shader, "sunTheta"), sunTheta);
    glUniform1f(glGetUniformLocation(m_skybox_shader, "sunPhi"), sunPhi);



    glBindVertexArray(m_skyboxVAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxTexture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_starTexture);



    glDrawArrays(GL_TRIANGLES, 0, m_verticiesVector.size() / 3);

    glBindVertexArray(0);
    glUseProgram(0);
    Debug::glErrorCheck();
}



void Realtime::updateSkybox(){
    time ++;
    time %= dayLength;

//    sunTheta += (2*M_PI) / (float) dayLength;
    lightPos = glm::vec3(glm::vec4(lightPos, 1) * glm::rotate(float(M_PI / ((float) dayLength / 2.f)), glm::vec3(1,0,0)));



    float timeRatio = (float) time / (float) dayLength;


    if(timeRatio < .5){
        float linInterp = (timeRatio  / .5);
        skyColor = (linInterp * dayColor) + (1-linInterp)*nightColor;
        return;
    }
    float linInterp = ((timeRatio - .5) / (1 - .5));
    skyColor = (linInterp * nightColor) + (1-linInterp)*dayColor;
    return;



}
