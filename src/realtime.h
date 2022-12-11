#pragma once

// Defined before including GLEW to suppress deprecation messages on macOS
#include "shapes/Shape.h"
#include "block.h"
#include "utils/sceneparser.h"
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>

class Realtime : public QOpenGLWidget
{
public:
    Realtime(QWidget *parent = nullptr);
    void finish();                                      // Called on program exit
    void sceneChanged();
    void settingsChanged();
    static void insertVec3(std::vector<float> &data, glm::vec3 v);
    static void insertVertex(std::vector<float> &data, glm::vec3 pos, glm::vec3 norm, glm::vec3 color);

public slots:
    void tick(QTimerEvent* event);                      // Called once per tick of m_timer

protected:
    void initializeGL() override;                       // Called once at the start of the program
    void paintGL() override;                            // Called whenever the OpenGL context changes or by an update() request
    void resizeGL(int width, int height) override;      // Called when window size changes

private:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    void rebuildMatrices();
    std::tuple<GLint, GLint, GLint, GLint> initializeShader();
    void renderShape(Shape shape, GLint ambLoc, GLint diffLoc, GLint specLoc, GLint shineLoc);

    // GL variables
    GLuint m_phong_shader;    //Stores id for shader program
    GLuint m_tex_shader;
    GLuint m_vbo;   //Stores id for vbo
    GLuint m_vao;   //Stores id for vao
    GLuint m_fullscreen_vbo;
    GLuint m_fullscreen_vao;

    // Scene variables
    RenderData m_sceneData;
    std::vector<Shape> m_shapeData;
    glm::mat4 m_view  = glm::mat4(1);
    glm::mat4 m_proj  = glm::mat4(1);

    // Tick Related Variables
    int m_timer;                                        // Stores timer which attempts to run ~60 times per second
    QElapsedTimer m_elapsedTimer;                       // Stores timer which keeps track of actual time between frames

    // Input Related Variables
    bool m_mouseDown = false;                           // Stores state of left mouse button
    glm::vec2 m_prev_mouse_pos;                         // Stores mouse position
    std::unordered_map<Qt::Key, bool> m_keyMap;         // Stores whether keys are pressed or not

    // Device Correction Variables
    int m_devicePixelRatio;

    // project 6 stuff: framebuffers, etc.
    GLuint m_defaultFBO;
    GLuint m_fbo;
    GLuint m_fbo_texture;
    GLuint m_fbo_renderbuffer;
    int m_fbo_width;
    int m_fbo_height;
    void makeFBO();
    void paintTexture(GLuint texture, bool pixelFilter, bool kernelFilter);
    void paintGeometry();

    // FINAL PROJECT STUFF
    std::vector<Block> m_blockData;
    void computeBlockShapeData();
    void genTestBlockData();
    void populateSceneData();

    // skybox stuff
    // some of this isn't used but is still in here
    GLuint m_skyboxVAO, m_skyboxVBO;
    GLuint m_skyboxTexture;
    GLuint m_skybox_shader;
    void setupSkybox();
    void paintSkybox();
    void updateSkybox();
    std::vector<float> m_verticiesVector;
    int dayLength = 5000;
    int time;
    glm::vec4 skyColor;

    float sunPhi = -M_PI;
    float sunTheta = M_PI;

    glm::vec3 lightPos = glm::vec3{0,-1,0};

    glm::vec4 dayColor{13.f / 255.f,174.f / 255.f, 255.f / 255.f, 1};
    glm::vec4 nightColor{25.f / 255.f, 12.f / 255.f, 59.f / 255.f, 1};
    glm::vec4 sunsetColor{238.f / 255.f, 175.f / 255.f, 97.f / 255.f, 1};
};
