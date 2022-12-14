#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

struct Settings {
    std::string sceneFilePath;
    int shapeParameter1 = 0;
    int shapeParameter2 = 1;
    int shapeParameter3 = 100;
    int shapeParameter4 = 128;
    int shapeParameter5 = 2500;
    float nearPlane = 1;
    float farPlane = 1;
    bool perPixelFilter = false;
    bool kernelBasedFilter = false;
    bool extraCredit1 = false;
    bool extraCredit2 = false;
    bool extraCredit3 = false;
    bool extraCredit4 = false;

    bool marvelCheck = false;
    bool toonCheck = false;
    bool playerCheck = false;
    int movementSpeed = 20;
    int mouseMovementSpeed = 5;
    static constexpr int renderWidth = 200;
    int numBiomes = 10;
    int maxHeight = 100;
    int cloudSize = 10;
    float boundaryDisplacement = 8;
    int toonParam = 1;
};


// The global Settings object, will be initialized by MainWindow
extern Settings settings;

#endif // SETTINGS_H
