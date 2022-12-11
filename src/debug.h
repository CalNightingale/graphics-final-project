#pragma once
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <iostream>

namespace Debug
{
    // TASK 2: Add file name and line number parameters
    inline void glErrorCheck(const char* filename, int linenum){
        GLenum error = glGetError();
        while(error != GL_NO_ERROR){
            // Task 2: Edit this print statement to be more descriptive
            std::cout<<filename<<":"<<linenum<<":  "<<error<<std::endl;
            error = glGetError();
        }
    }

    // TASK 3: Add a preprocessor directive to automate the writing of this function
    #define glErrorCheck() glErrorCheck(__FILE__, __LINE__)
}
