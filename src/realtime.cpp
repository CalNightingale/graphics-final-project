#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "glm/gtx/string_cast.hpp"
#include "settings.h"
#include "./utils/shaderloader.h"
#include "shapes/cone.h"
#include "shapes/cube.h"
#include "shapes/cylinder.h"
#include "shapes/sphere.h"
#include "utils/sceneparser.h"

// ================== Project 5: Lights, Camera

Realtime::Realtime(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_prev_mouse_pos = glm::vec2(size().width()/2, size().height()/2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;

    // If you must use this function, do not edit anything above this
}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    // Students: anything requiring OpenGL calls when the program exits should be done here
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);

    glDeleteProgram(m_phong_shader);
    glDeleteProgram(m_tex_shader);
    glDeleteTextures(1, &m_fbo_texture);
    glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
    glDeleteFramebuffers(1, &m_fbo);

    this->doneCurrent();
}

void Realtime::initializeGL() {
    m_devicePixelRatio = this->devicePixelRatio();

    m_defaultFBO = 2; // CHANGE THIS IF NECESSARY FOR YOUR DEVICE
    m_fbo_width = size().width() * m_devicePixelRatio;
    m_fbo_height = size().height() * m_devicePixelRatio;

    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();

    // Initializing GL.
    // GLEW (GL Extension Wrangler) provides access to OpenGL functions.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    // set default GL values
    glClearColor(0,0,0,1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // Tells OpenGL how big the screen is
    // delete this maybe glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here
    // load shaders
    m_phong_shader = ShaderLoader::createShaderProgram("resources/shaders/phong.vert", "resources/shaders/phong.frag");
    m_tex_shader = ShaderLoader::createShaderProgram("resources/shaders/texture.vert", "resources/shaders/texture.frag");
    // gen & bind vbo
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    // gen & bind vao
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    // Add position and normal attributes to vao
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*6, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*6, reinterpret_cast<void*>(sizeof(float)*3));

    // unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    // STUFF FOR FBO
    glUseProgram(m_tex_shader);
    glUniform1i(glGetUniformLocation(m_tex_shader, "tex"), 0);
    std::vector<GLfloat> fullscreen_quad_data =
    { //     POSITIONS    //
        -1.0f,  1.0f, 0.0f, // tl
         0.0f,  1.0f, // tl in uv
        -1.0f, -1.0f, 0.0f, // bl
         0.0f,  0.0f, // bl in uv
         1.0f, -1.0f, 0.0f, // br
         1.0f,  0.0f, // br in uv
         1.0f,  1.0f, 0.0f, // tr
         1.0f,  1.0f, // tr in uv
        -1.0f,  1.0f, 0.0f, // tl
         0.0f,  1.0f, // tl in uv
         1.0f, -1.0f, 0.0f, // br
         1.0f,  0.0f, // br in uv
    };

    // Generate and bind a VBO and a VAO for a fullscreen quad
    glGenBuffers(1, &m_fullscreen_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_fullscreen_vbo);
    glBufferData(GL_ARRAY_BUFFER, fullscreen_quad_data.size()*sizeof(GLfloat), fullscreen_quad_data.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &m_fullscreen_vao);
    glBindVertexArray(m_fullscreen_vao);

    // Task 14: modify the code below to add a second attribute to the vertex attribute array
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<void*>(sizeof(float)*3));

    // Unbind the fullscreen quad's VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    makeFBO();
    glUseProgram(0);

    // REMOVE LATER! FOR TESTING
    genTestBlockData();
}

/**
 * @brief Realtime::makeFBO ported from lab 11: generate an empty FBO
 */
void Realtime::makeFBO(){
    makeCurrent();
    // Task 19: Generate and bind an empty texture, set its min/mag filter interpolation, then unbind
    glGenTextures(1, &m_fbo_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fbo_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_fbo_width, m_fbo_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    // Task 20: Generate and bind a renderbuffer of the right size, set its format, then unbind
    glGenRenderbuffers(1, &m_fbo_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_fbo_renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_fbo_width, m_fbo_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    // Task 18: Generate and bind an FBO
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    // Task 21: Add our texture as a color attachment, and our renderbuffer as a depth+stencil attachment, to our FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fbo_texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_fbo_renderbuffer);
    // Task 22: Unbind the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
}

/**
 * @brief Realtime::initializeShader send all of the parameters the shader needs to the shader.
 * MUST BE CALLED WITH VAO BOUND!!!
 * @return the locations of ambient, diffuse, specular, and shininess lighting parameters.
 * These must be set on a shape-by-shape basis and cannot be set in this method
 */
std::tuple<GLint, GLint, GLint, GLint> Realtime::initializeShader() {
    // handle camera matricies & position
    GLint viewLoc = glGetUniformLocation(m_phong_shader, "m_view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &m_view[0][0]);

    GLint projLoc = glGetUniformLocation(m_phong_shader, "m_proj");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &m_proj[0][0]);

    GLint cameraPosLoc = glGetUniformLocation(m_phong_shader, "cameraPos");
    glm::vec4 cameraPos = inverse(m_view) * glm::vec4(0,0,0,1);
    glUniform3fv(cameraPosLoc, 1, &cameraPos[0]);

    // handle global properties
    GLint kaLoc = glGetUniformLocation(m_phong_shader, "ka");
    glUniform1f(kaLoc, m_sceneData.globalData.ka);

    GLint kdLoc = glGetUniformLocation(m_phong_shader, "kd");
    glUniform1f(kdLoc, m_sceneData.globalData.kd);

    GLint ksLoc = glGetUniformLocation(m_phong_shader, "ks");
    glUniform1f(ksLoc, m_sceneData.globalData.ks);

    // handle lights
    for (int j = 0; j < m_sceneData.lights.size(); j++) {
        SceneLightData curLight = m_sceneData.lights[j];
        std::string intStr = "lightIntensities[" + std::to_string(j) + "]";
        GLint lightIntLoc = glGetUniformLocation(m_phong_shader, intStr.c_str());

        std::string dirStr = "lightDirs[" + std::to_string(j) + "]";
        GLint lightDirLoc = glGetUniformLocation(m_phong_shader, dirStr.c_str());

        std::string posStr = "lightPositions[" + std::to_string(j) + "]";
        GLint lightPosLoc = glGetUniformLocation(m_phong_shader, posStr.c_str());

        std::string typeStr = "lightTypes[" + std::to_string(j) + "]";
        GLint lightTypeLoc = glGetUniformLocation(m_phong_shader, typeStr.c_str());

        std::string funcStr = "lightFunctions[" + std::to_string(j) + "]";
        GLint lightFuncLoc = glGetUniformLocation(m_phong_shader, funcStr.c_str());

        std::string angleStr = "lightAngles[" + std::to_string(j) + "]";
        GLint lightAngleLoc = glGetUniformLocation(m_phong_shader, angleStr.c_str());

        std::string penumbraStr = "lightPenumbras[" + std::to_string(j) + "]";
        GLint lightPenumbraLoc = glGetUniformLocation(m_phong_shader, penumbraStr.c_str());

        glUniform4fv(lightIntLoc, 1, &curLight.color[0]);
        glUniform3f(lightDirLoc, curLight.dir.x, curLight.dir.y, curLight.dir.z);
        glUniform3f(lightPosLoc, curLight.pos.x, curLight.pos.y, curLight.pos.z);
        glUniform3f(lightFuncLoc, curLight.function.x, curLight.function.y, curLight.function.z);
        glUniform1f(lightAngleLoc, curLight.angle);
        glUniform1f(lightPenumbraLoc, curLight.penumbra);

        // LIGHT TYPE ENUMERATION: 1 = directional, 2 = point, 3 = spot, 4 = area
        switch (curLight.type) {
        case LightType::LIGHT_DIRECTIONAL:
            glUniform1i(lightTypeLoc, 1);
            break;
        case LightType::LIGHT_POINT:
            glUniform1i(lightTypeLoc, 2);
            break;
        case LightType::LIGHT_SPOT:
            glUniform1i(lightTypeLoc, 3);
            break;
        case LightType::LIGHT_AREA:
            glUniform1i(lightTypeLoc, 4);
            break;
        }
    }

    // return locations of material properties
    GLint ambLoc = glGetUniformLocation(m_phong_shader, "amb");
    GLint diffLoc = glGetUniformLocation(m_phong_shader, "diff");
    GLint specLoc = glGetUniformLocation(m_phong_shader, "spec");
    GLint shineLoc = glGetUniformLocation(m_phong_shader, "shininess");

    return {ambLoc, diffLoc, specLoc, shineLoc};
}

void Realtime::paintGL() {
    // Students: anything requiring OpenGL calls every frame should be done here
    // Task 24: Bind FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    // Task 28: Call glViewport
    glViewport(0, 0, m_fbo_width, m_fbo_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintGeometry();
    // Task 25: Bind the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    // Task 26: Clear the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Task 27: Call paintTexture to draw our FBO color attachment texture
    paintTexture(m_fbo_texture, settings.perPixelFilter, settings.kernelBasedFilter);
}

// modified from lab 11's paintExampleGeometry() method
void Realtime::paintGeometry() {
    glUseProgram(m_phong_shader);

    // initialize shader
    auto [ambLoc, diffLoc, specLoc, shineLoc] = initializeShader();

    // bind vbo
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBindVertexArray(m_vao);
    // iterate through each shape in the scene and render it
    for (const Shape &s : m_shapeData) renderShape(s, ambLoc, diffLoc, specLoc, shineLoc);

    // Unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
}

/**
 * @brief genShapeFromBlock converts block data into Shape data which can be rendered
 * @param block
 * @return Shape object to render
 */
Shape genShapeFromBlock(const Block &block) {
    // Shape has 2 parts: RenderShapeData data and std::vector<float> vertexData
    // RenderShapeData has 2 parts: ScenePrimitive primitive and glm::mat4 ctm
    // ScenePrimitive has 2 parts: PrimitiveType type and SceneMaterial material
    //      also has std::string meshfile but we don't use this
    // SceneMaterial has: ambient, diffuse, spec vec4s, shininess float, and other stuff we don't need
    // generate RenderShapeData
    SceneColor amb = SceneColor{0.5,0,0.5,1};
    SceneColor diff = SceneColor{1,1,0,1};
    SceneColor spec = SceneColor{1,1,1,1};
    float shine = 30;
    SceneMaterial mat = SceneMaterial{amb, diff, spec, shine};
    ScenePrimitive prim = ScenePrimitive{PrimitiveType::PRIMITIVE_CUBE, mat};
    // compute ctm: blocks in this world are all unit sized, simply have to be translated to proper location
    glm::mat4 ctm = Cube::getTranslationMatrix(block.pos.x, block.pos.y, block.pos.z);
    RenderShapeData rsd = RenderShapeData{prim, ctm};
    // generate vertexData
    Cube cube;
    cube.updateParams(2, rsd); // IGNORE SETTINGS; USE MINIMAL TESELLATION (its a cube - no difference)
    std::vector<float> vertexData = cube.generateShape();

    return Shape{rsd, vertexData};
}

// Modified from lab 11
void Realtime::paintTexture(GLuint texture, bool pixelFilter, bool kernelFilter){
    glUseProgram(m_tex_shader);

    // Pass postprocessing bools & image size to shader
    glUniform1i(glGetUniformLocation(m_tex_shader, "invert"), pixelFilter);
    glUniform1i(glGetUniformLocation(m_tex_shader, "blur"), kernelFilter);
    glUniform1i(glGetUniformLocation(m_tex_shader, "width"), m_fbo_width);
    glUniform1i(glGetUniformLocation(m_tex_shader, "height"), m_fbo_height);

    // bind vao & vbo
    glBindBuffer(GL_ARRAY_BUFFER, m_fullscreen_vbo);
    glBindVertexArray(m_fullscreen_vao);

    // Task 10: Bind "texture" to slot 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    // unbind everything
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
}

/**
 * @brief Realtime::renderShape draws the current shape. MUST BE CALLED WITH VAO BOUND!!!
 * @param shape shape struct to render
 * @param ambLoc location of ambient lighting parameter in shader
 * @param diffLoc location of diffuse lighting parameter in shader
 * @param specLoc location of specular lighting parameter in shader
 * @param shineLoc location of shininess lighting parameter in shader
 */
void Realtime::renderShape(Shape shape, GLint ambLoc, GLint diffLoc, GLint specLoc, GLint shineLoc) {
    // set material properties for shader
    glUniform4fv(ambLoc, 1, &shape.data.primitive.material.cAmbient[0]);
    glUniform4fv(diffLoc, 1, &shape.data.primitive.material.cDiffuse[0]);
    glUniform4fv(specLoc, 1, &shape.data.primitive.material.cSpecular[0]);
    glUniform1f(shineLoc, shape.data.primitive.material.shininess);


    // set vbo data to current shape
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*shape.vertexData.size(), shape.vertexData.data(), GL_STATIC_DRAW);
    // draw shape
    glDrawArrays(GL_TRIANGLES, 0, shape.vertexData.size() / 6);
}

void Realtime::resizeGL(int w, int h) {
    glDeleteTextures(1, &m_fbo_texture);
    glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
    glDeleteFramebuffers(1, &m_fbo);
    // Tells OpenGL how big the screen is
    //glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);
    m_fbo_width = size().width() * m_devicePixelRatio;
    m_fbo_height = size().height() * m_devicePixelRatio;
    makeFBO();
    // Students: anything requiring OpenGL calls when the program starts should be done here
    rebuildMatrices();
}

/**
 * @brief Realtime::computeSceneShapeData iterate through the shapes in the scene and compute their vertex data.
 * Store both information about their shape primitive and vertex data in m_shapeData, as both are needed for rendering
 */
void Realtime::computeSceneShapeData(const std::vector<RenderShapeData> &shapes) {
    m_shapeData.clear();
    for (const RenderShapeData &shape : shapes) {
        std::vector<float> curVertexData;
        switch(shape.primitive.type) {
        case PrimitiveType::PRIMITIVE_SPHERE: {
            Sphere sphere;
            sphere.updateParams(settings.shapeParameter1, settings.shapeParameter2, shape);
            curVertexData = sphere.generateShape();
            break;
        } case PrimitiveType::PRIMITIVE_CONE: {
            Cone cone;
            cone.updateParams(settings.shapeParameter1, settings.shapeParameter2, shape);
            curVertexData = cone.generateShape();
            break;
        } case PrimitiveType::PRIMITIVE_CUBE: {
            Cube cube;
            cube.updateParams(settings.shapeParameter1, shape);
            curVertexData = cube.generateShape();
            break;
        } case PrimitiveType::PRIMITIVE_CYLINDER: {
            Cylinder cylinder;
            cylinder.updateParams(settings.shapeParameter1, settings.shapeParameter2, shape);
            curVertexData = cylinder.generateShape();
            break;
        } default:
            std::cout << "WARNING: SKIPPING UNSUPPORTED SHAPE TYPE" << std::endl;
        }
        m_shapeData.push_back(Shape{shape, curVertexData});
    }
}

void Realtime::computeBlockShapeData() {
    m_shapeData.clear();
    for (const Block &block : m_blockData) {
        m_shapeData.push_back(genShapeFromBlock(block));
    }
}

void Realtime::genTestBlockData() {
    m_blockData.push_back(Block{glm::vec3(0,0,0), Grass});
    m_blockData.push_back(Block{glm::vec3(1,0,0), Stone});
    m_blockData.push_back(Block{glm::vec3(0,0,1), Dirt});

}

void Realtime::populateSceneData() {
    m_sceneData.cameraData.aperture = 0;
    m_sceneData.cameraData.focalLength = 0;
    m_sceneData.cameraData.heightAngle = 0.523599;
    m_sceneData.cameraData.pos = glm::vec4(-6.000000, 4.000000, 4.000000, 1.000000);
    m_sceneData.cameraData.look = glm::vec4(6.000000, -4.000000, -4.000000, 0.000000);
    m_sceneData.cameraData.up = glm::vec4(0.000000, 1.000000, 0.000000, 0.000000);

    m_sceneData.globalData.ka = 0.2;
    m_sceneData.globalData.kd = 0.5;
    m_sceneData.globalData.ks = 0.5;
    m_sceneData.globalData.kt = 0;

    // create a point light (simulate the sun being the light source)
    m_sceneData.lights.clear();
    SceneLightData light = SceneLightData{0, LightType::LIGHT_POINT, SceneColor{1,1,1,1}, glm::vec3(1,0,0), glm::vec4(3,3,3,1), glm::vec4(-2,-4,-6,0), 0, 0, 0, 0};
    m_sceneData.lights.push_back(light);
}

void Realtime::sceneChanged() {
    //SceneParser::parse(settings.sceneFilePath, m_sceneData);
    populateSceneData();
    rebuildMatrices();
    //computeSceneShapeData(m_sceneData.shapes);
    genTestBlockData();
    computeBlockShapeData();
    update(); // asks for a PaintGL() call to occur
}

void Realtime::settingsChanged() {
    rebuildMatrices();
    //computeSceneShapeData(m_sceneData.shapes);
    computeBlockShapeData();
    update(); // asks for a PaintGL() call to occur
}

/**
 * @brief Realtime::rebuildMatrices Recalculate the view and projection matrices using the currently
 * loaded scene and settings
 */
void Realtime::rebuildMatrices() {
    // create view matrix
    glm::vec3 w = -glm::normalize(m_sceneData.cameraData.look);
    glm::vec3 v = glm::normalize(glm::vec3(m_sceneData.cameraData.up) - glm::dot(glm::vec3(m_sceneData.cameraData.up), w) * w);
    glm::vec3 u = glm::cross(v, w);
    glm::mat4 camera_translate = glm::mat4(1, 0, 0, 0,
                                           0, 1, 0, 0,
                                           0, 0, 1, 0,
                                           -m_sceneData.cameraData.pos.x, -m_sceneData.cameraData.pos.y, -m_sceneData.cameraData.pos.z, 1);
    glm::mat4 camera_rotate = glm::mat4(u.x, v.x, w.x, 0,
                                        u.y, v.y, w.y, 0,
                                        u.z, v.z, w.z, 0,
                                          0,   0,   0, 1);
    m_view = camera_rotate * camera_translate;

    // create projection matrix
    glm::mat4 gl_matrix = glm::mat4(1, 0, 0, 0,
                                    0, 1, 0, 0,
                                    0, 0,-2, 0,
                                    0, 0,-1, 1);
    float c = -settings.nearPlane / settings.farPlane;
    glm::mat4 perspective = glm::mat4(1, 0,       0,    0,
                                      0, 1,       0,    0,
                                      0, 0, 1/(1+c),   -1,
                                      0, 0,-c/(1+c),    0);
    float widthAngle = m_sceneData.cameraData.heightAngle * ((float) width() / (float) height());
    float sx = 1 / (settings.farPlane * tan(widthAngle / 2));
    float sy = 1 / (settings.farPlane * tan(m_sceneData.cameraData.heightAngle / 2));
    float sz = 1 / settings.farPlane;
    glm::mat4 scaling = glm::mat4(sx, 0,  0, 0,
                                  0, sy,  0, 0,
                                  0,  0, sz, 0,
                                  0,  0,  0, 1);
    m_proj = gl_matrix * perspective * scaling;
}

// ================== Project 6: Action!

void Realtime::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;
}

void Realtime::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void Realtime::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void Realtime::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}

glm::mat3 computeRotationMatrix(float theta, glm::vec3 u) {
    return glm::mat3(cos(theta) + u.x*u.x*(1 - cos(theta)), u.x*u.y*(1 - cos(theta)) - u.z*sin(theta), u.x*u.z*(1 - cos(theta)) + u.y*sin(theta),
                     u.x*u.y*(1 - cos(theta)) + u.z*sin(theta), cos(theta) + u.y*u.y*(1-cos(theta)), u.y*u.z*(1-cos(theta)) - u.x*sin(theta),
                     u.x*u.z*(1 - cos(theta)) - u.y*sin(theta), u.y*u.z*(1 - cos(theta)) + u.x*sin(theta), cos(theta) + u.z*u.z*(1 - cos(theta)));
}

void Realtime::mouseMoveEvent(QMouseEvent *event) {
    if (m_mouseDown) {
        int posX = event->position().x();
        int posY = event->position().y();
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        // Use deltaX and deltaY here to rotate
        if (m_mouseDown) {
            glm::mat3 xRot = computeRotationMatrix(5 * (float)deltaX / width(), glm::vec3(0,1,0));
            m_sceneData.cameraData.pos = glm::vec4(xRot * glm::vec3(m_sceneData.cameraData.pos), 1);
            m_sceneData.cameraData.look = glm::vec4(xRot * glm::vec3(m_sceneData.cameraData.look), 0);
            m_sceneData.cameraData.up = glm::vec4(xRot * glm::vec3(m_sceneData.cameraData.up), 0);
            glm::vec3 verticalAxis = normalize(cross(glm::vec3(m_sceneData.cameraData.look), glm::vec3(m_sceneData.cameraData.up)));
            glm::mat3 yRot = computeRotationMatrix(5 * (float)deltaY / height(), verticalAxis);
            m_sceneData.cameraData.pos = glm::vec4(yRot * glm::vec3(m_sceneData.cameraData.pos), 1);
            m_sceneData.cameraData.look = glm::vec4(yRot * glm::vec3(m_sceneData.cameraData.look), 0);
            m_sceneData.cameraData.up = glm::vec4(yRot * glm::vec3(m_sceneData.cameraData.up), 0);
        }

        rebuildMatrices();
        update(); // asks for a PaintGL() call to occur
    }
}

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    // Use deltaTime and m_keyMap here to move around
    // handle forward motion
    if (m_keyMap[Qt::Key_W]) {
        m_sceneData.cameraData.pos += 5*deltaTime*normalize(m_sceneData.cameraData.look);
    }
    // backward
    if (m_keyMap[Qt::Key_S]) {
        m_sceneData.cameraData.pos -= 5*deltaTime*normalize(m_sceneData.cameraData.look);
    }
    // left
    if (m_keyMap[Qt::Key_A]) {
        glm::vec4 lateralDirection = glm::vec4(glm::cross(glm::vec3(m_sceneData.cameraData.look), glm::vec3(m_sceneData.cameraData.up)), 0);
        m_sceneData.cameraData.pos -= 5*deltaTime*normalize(lateralDirection);
    }
    // right
    if (m_keyMap[Qt::Key_D]) {
        glm::vec4 lateralDirection = glm::vec4(glm::cross(glm::vec3(m_sceneData.cameraData.look), glm::vec3(m_sceneData.cameraData.up)), 0);
        m_sceneData.cameraData.pos += 5*deltaTime*normalize(lateralDirection);
    }
    // up
    if (m_keyMap[Qt::Key_Space]) {
        m_sceneData.cameraData.pos += 5*deltaTime*glm::vec4(0,1,0,0);
    }
    // down
    if (m_keyMap[Qt::Key_Control]) {
        m_sceneData.cameraData.pos += 5*deltaTime*glm::vec4(0,-1,0,0);
    }

    rebuildMatrices();
    update(); // asks for a PaintGL() call to occur
}
