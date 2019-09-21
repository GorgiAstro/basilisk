//
//  utilities.h
//  AVS basilisk
//
//  Created by Patrick Kenneally on 12/6/17.
//

#ifndef utilities_h
#define utilities_h
#include <stdio.h>
#include <cassert>
#include <glload/gl_4_1.hpp>

static GLenum CheckOpenGLError(const char* fname, int line)
{
    GLenum err = gl::GetError();
    if (err != gl::NO_ERROR_)
    {
        printf("OpenGL error %08x, at %s:%i\n", err, fname, line);
        return err;
    }
    return err;
}

//#ifdef _DEBUG
//#define GL_CHECK(stmt) do { \
//stmt; \
//CheckOpenGLError(#stmt, __FILE__, __LINE__); \
//} while (0)
//#else
//#define GL_CHECK(stmt) stmt
//#endif

#define CHECK_GL_ERROR assert(CheckOpenGLError(__FILE__, __LINE__) == 0)



#endif /* utilities_h */
