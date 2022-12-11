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

struct RGBA {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
    std::uint8_t a = 255;
};

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
    void genBiomeShapes();

    // biome mapping stuff
    bool loadImageFromFile(const QString &file);
    int m_biomeImg_width;
    int m_biomeImg_height;
    std::vector<RGBA> m_data;
    std::vector<RGBA> m_biomeColors;

    void recurseBiomes(int x, int y, int biomeID);
    void computeBiomeTypes();
    int m_biomeMap[256][256];
    float m_precipMap[256][256];
    float m_tempMap[256][256];
    float m_heightMap[256][256];

    void populateMaps();

};
