#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "settings.h"
#include "./utils/shaderloader.h"
#include "shapes/cube.h"
#include "utils/sceneparser.h"

#include "src/voronoi/src/jc_voronoi.h"
#include "noise/biome.h"
#include <random>

#define GL_SILENCE_DEPRECATION

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
    // Anything requiring OpenGL calls when the program starts should be done here
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

    // load shaders
    m_phong_shader = ShaderLoader::createShaderProgram("resources/shaders/phong.vert", "resources/shaders/phong.frag");
    m_tex_shader = ShaderLoader::createShaderProgram("resources/shaders/texture.vert", "resources/shaders/texture.frag");
    // gen & bind vbo
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
   //computeBlockShapeData();

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

    // Initialize vertex data once (we can do this since everything is a cube)
    Cube cube;
    cube.updateParams(2);
    vertexData = cube.generateShape();
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertexData.size(), vertexData.data(), GL_STATIC_DRAW);

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

    // Generate terrain
    populateBoundaryNoise();
    populateSceneData();
    settings.farPlane = 1000;
    rebuildMatrices();
    genBiomeShapes();
    populateMaps();
    computeBiomeTypes();
    populateHeights();
    genBlockData();
    computeBlockShapeData();
    setupSkybox();
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
    //glm::vec4 cameraPos = glm::vec4(0,0,0,1); // TODO which one?
    glUniform3fv(cameraPosLoc, 1, &cameraPos[0]);

    GLint toonCountLocation = glGetUniformLocation(m_phong_shader, "toonCount");
    glUniform1i(toonCountLocation, settings.toonParam);


    GLint toonIncrementsLocation = glGetUniformLocation(m_phong_shader, "toonIncrements");
    glUniform1f(toonIncrementsLocation, (1.0f/settings.toonParam));
    //std::cout << settings.toonCheck << std::endl;

    glUniform1i(glGetUniformLocation(m_phong_shader, "toonOn"), settings.toonCheck);

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
        if (settings.toonCheck == true){
            glUniform3f(lightPosLoc, 0, 30, (settings.shapeParameter5/10)-250);
        }
        else if (settings.toonCheck == false){
            glUniform3f(lightPosLoc, curLight.pos.x, curLight.pos.y, curLight.pos.z);
        }

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
    //runTrack();

    if (settings.playerCheck == true){
    walkMode();
    }
    if (settings.marvelCheck == true){
    marvelBeauty();
    }
    // Anything requiring OpenGL calls every frame should be done here

    if (time != settings.shapeParameter5) {
        time = settings.shapeParameter5;
        // recalculate position of light based on time of day
        m_sceneData.lights[0].pos = glm::vec4(0,1000*sin(time*M_PI / 5000),-1000*cos(time*M_PI / 5000),1);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    glViewport(0, 0, m_fbo_width, m_fbo_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintGeometry();
    paintSkybox();
}

// modified from lab 11's paintExampleGeometry() method
void Realtime::paintGeometry() {
    glUseProgram(m_phong_shader);

    // initialize shader
    auto [ambLoc, diffLoc, specLoc, shineLoc] = initializeShader();

    // bind vbo
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBindVertexArray(m_vao);

    // Render terrain blocks
    for (int i = 0; i < m_shapeData.size()-1; i++) {
        renderShape(m_shapeData[i], ambLoc, diffLoc, specLoc, shineLoc);
    }

    // Render ocean
    renderShape(m_shapeData[m_shapeData.size()-1], ambLoc, diffLoc, specLoc, shineLoc);

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

    // Generate RenderShapeData
    SceneColor scaledDiff = SceneColor{block.color.r / 255.0f, block.color.g / 255.0f, block.color.b / 255.0f, 1};
    SceneMaterial mat = SceneMaterial{blockAmbient, scaledDiff, blockSpecular, blockShininess};
    ScenePrimitive prim = ScenePrimitive{PrimitiveType::PRIMITIVE_CUBE, mat};
    glm::mat4 ctm = glm::mat4(1.f);
    // Position blocks in the world; sink by y/2 because we're going to stretch by a factor of y
    ctm[3][0] = block.pos.x;
    ctm[3][1] = block.pos.y / 2;
    ctm[3][2] = block.pos.z;
    // Lazy way to render ocean block (which is the only blue block): x and z get stretched out by render width
    if (block.color.b == 255) {
        ctm[0][0] = settings.renderWidth;
        ctm[1][1] = 1;
        ctm[2][2] = settings.renderWidth;
    } else {
        // Every other block (which is part of the terrain) gets y stretched
        ctm[1][1] = block.pos.y;
    }

    glm::mat4 inverseCTM = glm::inverse(ctm);
    glm::mat3 inverseTransposeCTM = glm::transpose(glm::mat3(inverseCTM));

    RenderShapeData rsd = RenderShapeData{prim, ctm};

    return Shape{rsd, inverseTransposeCTM};
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

    GLint modelLoc = glGetUniformLocation(m_phong_shader, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &shape.data.ctm[0][0]);
    GLint itModelLoc = glGetUniformLocation(m_phong_shader, "it_model");
    glUniformMatrix3fv(itModelLoc, 1, GL_FALSE, &shape.inverseCTM[0][0]);

    // draw shape
    glDrawArrays(GL_TRIANGLES, 0, vertexData.size() / 6);
}

void Realtime::resizeGL(int w, int h) {
    glDeleteTextures(1, &m_fbo_texture);
    glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
    glDeleteFramebuffers(1, &m_fbo);
    // Tells OpenGL how big the screen is
    m_fbo_width = size().width() * m_devicePixelRatio;
    m_fbo_height = size().height() * m_devicePixelRatio;
    makeFBO();
    // Students: anything requiring OpenGL calls when the program starts should be done here
    rebuildMatrices();
}

void Realtime::computeBlockShapeData() {
    m_shapeData.clear();
    for (const Block &block : m_blockData) {
        m_shapeData.push_back(genShapeFromBlock(block));
    }
}

int Realtime::getHeight(int x, int z) {
    if (x >= 0 && x < settings.renderWidth && z >= 0 && z < settings.renderWidth) return m_heightMap[z][x];
    return 0;
}

void Realtime::genBlockData() {
    m_blockData.clear();
    for (int x = 0; x < settings.renderWidth; x++) {
        for (int z = 0; z < settings.renderWidth; z++) {
            int biomeID = m_biomeMap[z][x];
            int y = m_heightMap[z][x];

            if (y == 0) {
                // This is the ocean; we won't render any blocks and just render a single stretched-out ocean block
                continue;
            }
            // set edges to a neighboring biome
            if (biomeID == -1 || biomeID == -10) biomeID = getNonEdgeNeighborID(x, z);
            SceneColor col = m_biomeColors[biomeID];
            // For now just place each block at its highest point (later the blocks will be lowered and stretched)
            m_blockData.push_back(Block{glm::vec3(x-(float)settings.renderWidth/2.0, y, z-(float)settings.renderWidth/2.0), col});
        }
    }

    m_blockData.push_back(Block{glm::vec3(0, -1, 0), SceneColor{0,0,255,1}}); // The ocean block is the last block in the list

}
std::vector<glm::vec3> Realtime::createTrackPositions(glm::vec3 pos1, glm::vec3 pos2){
    std::vector<glm::vec3> returnVector;
    float xIncrement = (pos2[0] - pos1[0])/10.f;
    float yIncrement = (pos2[1] - pos1[2])/10.f;
    float zIncrement = (pos2[1] - pos1[2])/10.f;

    for (int i = 0; i < 10; i++){
        glm::vec3 newPosition;
        newPosition[0] = pos1[0]+(i*xIncrement);
        newPosition[1] = pos1[1]+(i*yIncrement);
        newPosition[2] = pos1[2]+(i*zIncrement);

        returnVector.push_back(newPosition);
    }
    return returnVector;
}


void Realtime::walkMode(){
    settings.movementSpeed = 10;
    int walkingXpos = (round(m_sceneData.cameraData.pos[0])+settings.renderWidth/2.0);
    int walkingZpos = (round(m_sceneData.cameraData.pos[2])+settings.renderWidth/2.0);

     m_sceneData.cameraData.pos[1] = m_heightMap[walkingZpos][walkingXpos] + 3.f;

     rebuildMatrices();
}

//get camera pos and dir depending on marvel angle.
void Realtime::marvelBeauty(){
    float radius = 120.f;
    float newCameraPosX = radius * sin(3.14159265359 * 2 * marvelRotationAngle / 360.f);
    float newCameraPosY = 60;
    float newCameraPosZ = radius * cos(3.14159265359 * 2 * marvelRotationAngle / 360.f);

    m_sceneData.cameraData.pos[0] = newCameraPosX;
    m_sceneData.cameraData.pos[1] = newCameraPosY;
    m_sceneData.cameraData.pos[2] = newCameraPosZ;

    glm::vec4 newViewVector = glm::vec4(1.f, 1.f, 1.f, 1.f) - m_sceneData.cameraData.pos;
    glm::vec4 newUpVector = glm::vec4(0.0f, 1.f, 0.f, 0.f);

    m_sceneData.cameraData.up = newUpVector;
    m_sceneData.cameraData.look = newViewVector;

    rebuildMatrices();
}


// Go in all 4 directions until a biome is found, return biome ID with shortest distance traveled
int Realtime::getNonEdgeNeighborID(int x, int y) {
    std::vector<int> candidates;
    std::vector<int> distances;
    int cur = x + 1;
    int dist = 1;
    while (cur < settings.renderWidth) {
        int candidate = m_biomeMap[y][cur];
        if (candidate > -1) {
            candidates.push_back(candidate);
            distances.push_back(dist);
            break;
        }
        cur++;
        dist++;
    }

    cur = x - 1;
    dist = 1;
    while (cur > 0) {
        int candidate = m_biomeMap[y][cur];
        if (candidate > -1) {
            candidates.push_back(candidate);
            distances.push_back(dist);
            break;
        }
        cur--;
        dist++;
    }

    cur = y + 1;
    dist = 1;
    while (cur < settings.renderWidth) {
        int candidate = m_biomeMap[cur][x];
        if (candidate > -1) {
            candidates.push_back(candidate);
            distances.push_back(dist);
            break;
        }
        cur++;
        dist++;
    }

    cur = y - 1;
    dist = 1;
    while (cur > 0) {
        int candidate = m_biomeMap[cur][x];
        if (candidate > -1) {
            candidates.push_back(candidate);
            distances.push_back(dist);
            break;
        }
        cur--;
        dist++;
    }

    int minDist = settings.renderWidth * 2;
    for (int i = 0; i < candidates.size(); i++) {
        if (candidates[i] < minDist) minDist = candidates[i];
    }
    return minDist;
}

void Realtime::recurseBiomes(int x, int y, int biomeID) {
    if (x < 0 || y < 0 || x > 255 || y > 255) return; // OB point
    if (m_biomeMap[y][x] >= -1) return; // edge OR already checked

    m_biomeMap[y][x] = biomeID; // set biomeID

    recurseBiomes(x + 1, y, biomeID);
    recurseBiomes(x - 1, y, biomeID);
    recurseBiomes(x, y + 1, biomeID);
    recurseBiomes(x, y - 1, biomeID);
}

// Use voronoi library to create the biomes we're after
void Realtime::genBiomeShapes() {
    // Initialize necessary variables
    for (int x = 0; x < settings.renderWidth; x++) {
        for (int y = 0; y < settings.renderWidth; y++) {
            m_biomeMap[y][x] = -10;
        }
    }
    jcv_point points[settings.numBiomes];
    const jcv_site* sites;
    jcv_graphedge* graph_edge;
    jcv_rect bounding_box = { { 0.0f, 0.0f }, { static_cast<jcv_real>(settings.renderWidth), static_cast<jcv_real>(settings.renderWidth) } };

    // TODO ONCE FINISHED, USE time(NULL). Till then use 0 for testing - it will always gen the same diagram
    // srand(time(NULL));
    //srand(m_seed);

    std::mt19937 e2(m_seed);
    std::uniform_real_distribution<> dist(0, 1);

    // Generate center point for each biome
    for (int i = 0; i < settings.numBiomes; i++) {
      points[i].x = round((float)(dist(e2) * settings.renderWidth));
      points[i].y = round((float)(dist(e2) * settings.renderWidth));
    }

    // Create voronoi diagram using the points
    jcv_diagram diagram;
    memset(&diagram, 0, sizeof(jcv_diagram));
    jcv_diagram_generate(settings.numBiomes, (const jcv_point *)points, &bounding_box, 0, &diagram);

    // Iterate through edges and draw
    sites = jcv_diagram_get_sites(&diagram);
    for (int i=0; i<diagram.numsites; i++) {
      graph_edge = sites[i].edges;
      while (graph_edge) {
          // This approach will potentially print shared edges twice
          int startX = round(graph_edge->pos[0].x);
          int startY = round(graph_edge->pos[0].y);
          int endX = round(graph_edge->pos[1].x);
          int endY = round(graph_edge->pos[1].y);
          int lastX = -1;
          int lastY = -1;
          // parameterize edge with t and iterate over it
          //float stepsize = 0.001;
          float nSteps = 1000;
          //for (float t = 0; t < 1; t += stepsize) {
          for (int t = 0; t < nSteps; t += 1) {
              float tStep = (float)t / nSteps;
              int x = round((1-tStep)*startX + tStep*endX);
              int y = round((1-tStep)*startY + tStep*endY);
              if (x == lastX && y == lastY) continue; // do not render repeats
              m_biomeMap[y][x] = -1; // record this coordinate as the edge
              lastX = x;
              lastY = y;
          }
          graph_edge = graph_edge->next;
      }
      int centX = round(sites[i].p.x);
      int centY = round(sites[i].p.y);
      recurseBiomes(centX, centY, sites[i].index);
    }

    // Free the diagram's memory once done
    jcv_diagram_free(&diagram);

    // blur the biome edges
    int blurredMap[settings.renderWidth][settings.renderWidth];
    for (int x = 0; x < settings.renderWidth; x++) {
        for (int y = 0; y < settings.renderWidth; y++) {
            int i = (int)m_boundaryNoise[y][x].x;
            int j = (int)m_boundaryNoise[y][x].y;
            blurredMap[y][x] = m_biomeMap[i][j]; // MAY NEED TO SWITCH THIS i,j
        }
    }
    // replace biomeMap with blurred map
    for (int x = 0; x < settings.renderWidth; x++) {
        for (int y = 0; y < settings.renderWidth; y++) {
            m_biomeMap[y][x] = blurredMap[y][x];
        }
    }
}



void Realtime::computeBiomeTypes() {
    loadImageFromFile("resources/biome_image.png");
    // create necessary variables
    int biomeID;
    float temp;
    float precip;
    tempSums.clear();
    precipSums.clear();
    biomeSize.clear();
    // set sums for each biome to 0 to start
    for (int i = 0; i < settings.numBiomes; i++) {
        tempSums.push_back(0);
        precipSums.push_back(0);
        biomeSize.push_back(0);
    }
    // iterate through all blocks, keep track of temp and precip for each biome
    for (int x = 0; x < settings.renderWidth; x++) {
        for (int y = 0; y < settings.renderWidth; y++) {
            biomeID = m_biomeMap[y][x];
            if (biomeID == -1) {
                biomeID = getNonEdgeNeighborID(x, y);
            }
            tempSums[biomeID] += m_tempMap[y][x];
            precipSums[biomeID] += m_precipMap[y][x];
            biomeSize[biomeID]++;
        }
    }
    // iteate through each biome, compute avg temp and precip, map to biome type
    m_biomeColors.clear();
    m_biomeColors.reserve(settings.numBiomes);
    m_biomeTypes.clear();
    m_biomeTypes.reserve(settings.numBiomes);
    for (int i = 0; i < settings.numBiomes; i++) {
        float avgTemp = tempSums[i] / (biomeSize[i]+1);
        float avgPrecip = precipSums[i] / (biomeSize[i]+1);
        int tempImgIndex = round(avgTemp * m_biomeImg_width);
        int precipImgIndex = round(avgPrecip * m_biomeImg_height);

        m_biomeColors[i] = m_data[precipImgIndex * m_biomeImg_width + tempImgIndex];
        // populate biome types
        if (m_biomeColors[i].r == 255) { // desert
            m_biomeTypes[i] = 0;
        } else if (m_biomeColors[i].r == 184) { // savanna
            m_biomeTypes[i] = 1;
        } else if (m_biomeColors[i].r == 188) { // tropical woodland
            m_biomeTypes[i] = 2;
        } else if (m_biomeColors[i].r == 190) { // tundra
            m_biomeTypes[i] = 3;
        } else if (m_biomeColors[i].r == 106) { // seasonal forest
            m_biomeTypes[i] = 4;
        } else if (m_biomeColors[i].r == 33) { // rainforest
            m_biomeTypes[i] = 5;
        } else if (m_biomeColors[i].r == 86) { // temperate forest
            m_biomeTypes[i] = 6;
        } else if (m_biomeColors[i].r == 34) { // temperate rainforest
            m_biomeTypes[i] = 7;
        } else if (m_biomeColors[i].r == 35) { // boreal forest
            m_biomeTypes[i] = 8;
        } else{
            m_biomeTypes[i] = 0;
        }
    }
}

// Default settings for scene modifiable here
void Realtime::populateSceneData() {
    m_sceneData.cameraData.aperture = 0;
    m_sceneData.cameraData.focalLength = 0;
    m_sceneData.cameraData.heightAngle = 0.523599;
    //TOP DOWN VIEW
    m_sceneData.cameraData.pos = glm::vec4(0, 50, 0, 1);
    m_sceneData.cameraData.look = glm::vec4(0, -1, 0, 0);
    m_sceneData.cameraData.up = glm::vec4(1, 0, 0, 0);

    m_sceneData.globalData.ka = 0.2;
    m_sceneData.globalData.kd = 0.5;
    m_sceneData.globalData.ks = 0.5;
    m_sceneData.globalData.kt = 0;

    // create a directional light (simulate the sun being the light source)
    m_sceneData.lights.clear();
    SceneLightData light = SceneLightData{0, LightType::LIGHT_POINT, SceneColor{1,1,1,1}, glm::vec3(1,0,0), glm::vec4(settings.renderWidth/4,settings.maxHeight*2,-settings.renderWidth/4,1)};
    m_sceneData.lights.push_back(light);
}

void Realtime::sceneChanged() {
    populateSceneData();
    rebuildMatrices();
    //genTestBlockData();
    computeBlockShapeData();
    update(); // asks for a PaintGL() call to occur
}

void Realtime::settingsChanged() {
    if (m_seed != settings.shapeParameter1 || settings.numBiomes != settings.shapeParameter2 || settings.maxHeight != settings.shapeParameter3) {
        computeBlockShapeData();
        m_seed = settings.shapeParameter1;
        settings.numBiomes = fmax(1, settings.shapeParameter2);
        settings.maxHeight = settings.shapeParameter3;
        populateBoundaryNoise();
        settings.farPlane = 500;
        genBiomeShapes();
        populateMaps();
        computeBiomeTypes();
        populateHeights();
        genBlockData();
        computeBlockShapeData();
        update(); // asks for a PaintGL() call to occur
    }
}
void Realtime::teleportToOrigin(){
    float newCameraPosX = 0;
    float newCameraPosZ = 0;

    m_sceneData.cameraData.pos[0] = newCameraPosX;
    m_sceneData.cameraData.pos[2] = newCameraPosZ;


    rebuildMatrices();
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
            glm::mat3 xRot = computeRotationMatrix(settings.mouseMovementSpeed * (float)deltaX / width(), glm::vec3(0,1,0));
            //m_sceneData.cameraData.pos = glm::vec4(xRot * glm::vec3(m_sceneData.cameraData.pos), 1);
            m_sceneData.cameraData.look = glm::vec4(xRot * glm::vec3(m_sceneData.cameraData.look), 0);
            m_sceneData.cameraData.up = glm::vec4(xRot * glm::vec3(m_sceneData.cameraData.up), 0);
            glm::vec3 verticalAxis = normalize(cross(glm::vec3(m_sceneData.cameraData.look), glm::vec3(m_sceneData.cameraData.up)));
            glm::mat3 yRot = computeRotationMatrix(settings.mouseMovementSpeed * (float)deltaY / height(), verticalAxis);
            //m_sceneData.cameraData.pos = glm::vec4(yRot * glm::vec3(m_sceneData.cameraData.pos), 1);
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

    trackCounter += deltaTime*3;
//    std::cout << int(trackCounter) << std::endl;

    marvelRotationAngle += deltaTime*3;
    if (marvelRotationAngle >= 360.f){
        marvelRotationAngle = 0.f;
    }

    // Use deltaTime and m_keyMap here to move around
    // handle forward motion
    if (m_keyMap[Qt::Key_W]) {
        m_sceneData.cameraData.pos += settings.movementSpeed*deltaTime*normalize(m_sceneData.cameraData.look);
    }
    // backward
    if (m_keyMap[Qt::Key_S]) {
        m_sceneData.cameraData.pos -= settings.movementSpeed*deltaTime*normalize(m_sceneData.cameraData.look);
    }
    // left
    if (m_keyMap[Qt::Key_A]) {
        glm::vec4 lateralDirection = glm::vec4(glm::cross(glm::vec3(m_sceneData.cameraData.look), glm::vec3(m_sceneData.cameraData.up)), 0);
        m_sceneData.cameraData.pos -= settings.movementSpeed*deltaTime*normalize(lateralDirection);
    }
    // right
    if (m_keyMap[Qt::Key_D]) {
        glm::vec4 lateralDirection = glm::vec4(glm::cross(glm::vec3(m_sceneData.cameraData.look), glm::vec3(m_sceneData.cameraData.up)), 0);
        m_sceneData.cameraData.pos += settings.movementSpeed*deltaTime*normalize(lateralDirection);
    }
    // up
    if (m_keyMap[Qt::Key_Space]) {
        m_sceneData.cameraData.pos += settings.movementSpeed*deltaTime*glm::vec4(0,1,0,0);
    }
    // down
    if (m_keyMap[Qt::Key_Control]) {
        m_sceneData.cameraData.pos += settings.movementSpeed*deltaTime*glm::vec4(0,-1,0,0);
    }

    rebuildMatrices();
    update(); // asks for a PaintGL() call to occur
}

void Realtime::populateMaps() {
    Biome biome = Biome();
    std::vector<std::vector<float>> precMapVector = biome.createPreciptiationMap(settings.renderWidth,  m_seed);
    std::vector<std::vector<float>> tempMapVector = biome.createTemperatureMap(settings.renderWidth,  m_seed);
    std::vector<std::vector<std::vector<float>>> heightMaps;

    for (int i = 0; i < settings.renderWidth; i++) {
        for (int j = 0; j < settings.renderWidth; j++) {
            m_precipMap[i][j] = precMapVector[i][j];
            m_tempMap[i][j] = tempMapVector[i][j];
        }
    }
}

void Realtime::populateHeights() {
    Biome biome = Biome();

    std::vector<std::vector<float>> landMask = biome.createLandMask(settings.renderWidth, m_seed);
    biome.blurMask(landMask);

    std::vector<std::vector<std::vector<float>>> multipleHeightMaps;

    for (int b = 0; b < 9; b++) {
        std::vector<std::vector<float>> heightMap = biome.createBiomeHeightMap(settings.renderWidth, m_seed, b);
        multipleHeightMaps.push_back(heightMap);
    }

    std::vector<std::vector<std::vector<float>>> biomeMasks;

    for (int b = 0; b < 9; b++) {
        std::vector<std::vector<float>> biomeMask(settings.renderWidth, std::vector<float>(settings.renderWidth));
        for (int i = 0; i < settings.renderWidth; i++) {
            for (int j = 0; j < settings.renderWidth; j++) {
                if (m_biomeTypes[m_biomeMap[i][j]] == b) { // TODO change
                    biomeMask[i][j] = 1;
                }
            }
        }
        biome.blurMask(biomeMask);
        biomeMasks.push_back(biomeMask);
    }

    std::vector<std::vector<float>> heightMapAcc(settings.renderWidth, std::vector<float>(settings.renderWidth));

    for (int b = 0; b < 9; b++) {
        heightMapAcc = biome.applyMask(heightMapAcc, multipleHeightMaps[b], biomeMasks[b]);
    }

    std::vector<std::vector<float>> flatMap(settings.renderWidth, std::vector<float>(settings.renderWidth));
    biome.applyMask(heightMapAcc, flatMap, landMask);

    for (int i = 0; i < settings.renderWidth; i++) {
        for (int j = 0; j < settings.renderWidth; j++) {
            m_heightMap[i][j] = round(settings.maxHeight * heightMapAcc[i][j]);
        }
    }
}

bool Realtime::loadImageFromFile(const QString &file) {
    QImage myImage;
    if (!myImage.load(file)) {
        std::cout<<"Failed to load in image"<<std::endl;
        return false;
    }
    myImage = myImage.convertToFormat(QImage::Format_RGBX8888);
    m_biomeImg_width = myImage.width();
    m_biomeImg_height = myImage.height();
    QByteArray arr = QByteArray::fromRawData((const char*) myImage.bits(), myImage.sizeInBytes());

    m_data.clear();
    m_data.reserve(m_biomeImg_width * m_biomeImg_height);
    for (int i = 0; i < arr.size() / 4.f; i++){
        m_data.push_back(SceneColor{(std::uint8_t) arr[4*i], (std::uint8_t) arr[4*i+1], (std::uint8_t) arr[4*i+2], (std::uint8_t) arr[4*i+3]});
    }
    return true;
}

void Realtime::populateBoundaryNoise() {
    Simplex simplex = Simplex();
    std::vector<std::vector<float>> boundaryNoiseX = simplex.noiseMap(settings.renderWidth, 32, m_seed + 123, 8);
    std::vector<std::vector<float>> boundaryNoiseY = simplex.noiseMap(settings.renderWidth, 32, m_seed + 456, 8);
    for (int i = 0; i < settings.renderWidth; i++) {
        for (int j = 0; j < settings.renderWidth; j++) {
            m_boundaryNoise[i][j] = glm::vec2(i, j) + settings.boundaryDisplacement * glm::vec2(boundaryNoiseX[i][j], boundaryNoiseY[i][j]);
            m_boundaryNoise[i][j] = glm::clamp(m_boundaryNoise[i][j], 0.0f, (float)settings.renderWidth-1);
        }
    }
}
