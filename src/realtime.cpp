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

#include "src/voronoi/src/jc_voronoi.h"
#include "noise/biome.h"

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

    // Generate terrain
    populateSceneData();
    settings.farPlane = 1000;
    rebuildMatrices();
    genBiomeShapes();
    populateMaps();
    computeBiomeTypes();
    genBlockData();
    computeBlockShapeData();
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

    SceneColor scaledDiff = SceneColor{block.color.r / 255, block.color.g / 255, block.color.b / 255, 1};
    SceneMaterial mat = SceneMaterial{blockAmbient, scaledDiff, blockSpecular, blockShininess};
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

void Realtime::computeBlockShapeData() {
    m_shapeData.clear();
    for (const Block &block : m_blockData) {
        m_shapeData.push_back(genShapeFromBlock(block));
    }
}

void Realtime::genBlockData() {
    m_blockData.clear();
    for (int x = 0; x < settings.renderWidth; x++) {
        for (int y = 0; y < settings.renderWidth; y++) {
            int biomeID = m_biomeMap[y][x];
            SceneColor col = SceneColor{1,1,1,1};
            if (biomeID > -1) {
                col = m_biomeColors[biomeID];
                //std::cout << col.r << "," << col.g << "," << col.b << std::endl;
            }
            m_blockData.push_back(Block{glm::vec3(x, 0, y), col});
        }
    }
}

void Realtime::recurseBiomes(int x, int y, int biomeID) {
    if (x < 0 || y < 0 || x > 255 || y > 255) return; // OB point
    if (m_biomeMap[y][x] != 0) return; // edge OR already checked

    m_biomeMap[y][x] = biomeID; // set biomeID

    recurseBiomes(x + 1, y, biomeID);
    recurseBiomes(x - 1, y, biomeID);
    recurseBiomes(x, y + 1, biomeID);
    recurseBiomes(x, y - 1, biomeID);
}

// Use voronoi library to create the biomes we're after
void Realtime::genBiomeShapes() {
    // Initialize necessary variables
    memset(m_biomeMap, 0, sizeof(m_biomeMap));
    jcv_point points[settings.numBiomes];
    const jcv_site* sites;
    jcv_graphedge* graph_edge;
    jcv_rect bounding_box = { { 0.0f, 0.0f }, { static_cast<jcv_real>(settings.renderWidth), static_cast<jcv_real>(settings.renderWidth) } };

    // TODO ONCE FINISHED, USE time(NULL). Till then use 0 for testing - it will always gen the same diagram
    // srand(time(NULL));
    srand(0);

    // Generate center point for each biome
    for (int i = 0; i < settings.numBiomes; i++) {
      points[i].x = round((float)(rand()/(1.0f + RAND_MAX) * settings.renderWidth));
      points[i].y = round((float)(rand()/(1.0f + RAND_MAX) * settings.renderWidth));
    }

    // Create voronoi diagram using the points
    jcv_diagram diagram;
    memset(&diagram, 0, sizeof(jcv_diagram));
    jcv_diagram_generate(settings.numBiomes, (const jcv_point *)points, &bounding_box, 0, &diagram);

    // Iterate through edges and draw
    sites = jcv_diagram_get_sites(&diagram);
    m_blockData.clear();
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
          float stepsize = 0.001;
          for (float t = 0; t < 1; t += stepsize) {
              int x = round((1-t)*startX + t*endX);
              int y = round((1-t)*startY + t*endY);
              if (x == lastX && y == lastY) continue; // do not render repeats
              //m_blockData.push_back(Block{glm::vec3(x,0,y), Snow}); // COMMENT OUT TO AVOID RENDERING EDGES
              m_biomeMap[y][x] = -1; // record this coordinate as the edge
              lastX = x;
              lastY = y;
          }
          graph_edge = graph_edge->next;
      }
      int centX = round(sites[i].p.x);
      int centY = round(sites[i].p.y);
      recurseBiomes(centX, centY, sites[i].index + 1);
    }

    // Free the diagram's memory once done
    jcv_diagram_free(&diagram);
}

void Realtime::computeBiomeTypes() {
    loadImageFromFile("resources/biomeMap.jpg");
    // create necessary variables
    int biomeID;
    float temp;
    float precip;
    float tempSums[settings.numBiomes];
    float precipSums[settings.numBiomes];
    int biomeSize[settings.numBiomes];
    // set sums for each biome to 0 to start
    for (int i = 0; i < settings.numBiomes; i++) {
        tempSums[i] = 0;
        precipSums[i] = 0;
        biomeSize[i] = 0;
    }
    // iterate through all blocks, keep track of temp and precip for each biome
    for (int x = 0; x < settings.renderWidth; x++) {
        for (int y = 0; y < settings.renderWidth; y++) {
            biomeID = m_biomeMap[y][x] - 1; // biomeIDs are 1-indexed
            if (biomeID == -1) continue; // skip edges for now
            tempSums[biomeID] += m_tempMap[y][x];
            precipSums[biomeID] += m_precipMap[y][x];
            biomeSize[biomeID]++;
        }
    }
    // iteate through each biome, compute avg temp and precip, map to biome type
    m_biomeColors.clear();
    m_biomeColors.reserve(m_biomeImg_width * m_biomeImg_height);
    for (int i = 0; i < settings.numBiomes; i++) {
        float avgTemp = tempSums[i] / biomeSize[i];
        float avgPrecip = precipSums[i] / biomeSize[i];
        int tempImgIndex = round(avgTemp * m_biomeImg_width);
        int precipImgIndex = round(avgPrecip * m_biomeImg_height);
        SceneColor color = m_data[precipImgIndex * m_biomeImg_width + tempImgIndex];
        m_biomeColors[i] = color;
    }
}

// WANT TO MODIFY DEFAULT SETTINGS??? DO SO HERE!!!
void Realtime::populateSceneData() {
    m_sceneData.cameraData.aperture = 0;
    m_sceneData.cameraData.focalLength = 0;
    m_sceneData.cameraData.heightAngle = 0.523599;
    m_sceneData.cameraData.pos = glm::vec4(255/2, 500, 255/2, 1);
    m_sceneData.cameraData.look = glm::vec4(0, -1, 0, 0);
    m_sceneData.cameraData.up = glm::vec4(1, 0, 0, 0);

    m_sceneData.globalData.ka = 0.2;
    m_sceneData.globalData.kd = 0.5;
    m_sceneData.globalData.ks = 0.5;
    m_sceneData.globalData.kt = 0;

    // create a directional light (simulate the sun being the light source)
    m_sceneData.lights.clear();
    SceneLightData light = SceneLightData{0, LightType::LIGHT_DIRECTIONAL, SceneColor{1,1,1,1}, glm::vec3(1,0,0), glm::vec4(3,3,3,1), glm::vec4(-2,-4,-6,0), 0, 0, 0, 0};
    m_sceneData.lights.push_back(light);
}

void Realtime::sceneChanged() {
    //SceneParser::parse(settings.sceneFilePath, m_sceneData);
    populateSceneData();
    rebuildMatrices();
    //genTestBlockData();
    computeBlockShapeData();
    update(); // asks for a PaintGL() call to occur
}

void Realtime::settingsChanged() {
    rebuildMatrices();
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
            glm::mat3 xRot = computeRotationMatrix(settings.movementSpeed * (float)deltaX / width(), glm::vec3(0,1,0));
            m_sceneData.cameraData.pos = glm::vec4(xRot * glm::vec3(m_sceneData.cameraData.pos), 1);
            m_sceneData.cameraData.look = glm::vec4(xRot * glm::vec3(m_sceneData.cameraData.look), 0);
            m_sceneData.cameraData.up = glm::vec4(xRot * glm::vec3(m_sceneData.cameraData.up), 0);
            glm::vec3 verticalAxis = normalize(cross(glm::vec3(m_sceneData.cameraData.look), glm::vec3(m_sceneData.cameraData.up)));
            glm::mat3 yRot = computeRotationMatrix(settings.movementSpeed * (float)deltaY / height(), verticalAxis);
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
    std::vector<std::vector<float>> precMapVector = biome.createPreciptiationMap(256, 0);
    std::vector<std::vector<float>> tempMapVector = biome.createTemperatureMap(256, 0);
    std::vector<std::vector<float>> heightMap = biome.createBiomeHeightMap(256, 0, 3);

    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            m_precipMap[i][j] = precMapVector[i][j];
            m_tempMap[i][j] = tempMapVector[i][j];
            m_heightMap[i][j] = heightMap[i][j];
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
