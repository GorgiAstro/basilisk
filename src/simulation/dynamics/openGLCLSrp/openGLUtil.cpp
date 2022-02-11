#include "openGLUtil.h"

#include <iostream>
#include <fstream>

char* loadFile(const char *fname, GLint &fSize)
{
    std::ifstream file (fname,std::ios::in|std::ios::binary|std::ios::ate);
    if (file.is_open())
    {
        unsigned int size = (unsigned int)file.tellg();
        fSize = size;
        char *memblock = new char [size];
        file.seekg (0, std::ios::beg);
        file.read (memblock, size);
        file.close();
        std::cout << "file " << fname << " loaded" << std::endl;
        return memblock;
    }

    std::cout << "Unable to open file " << fname << std::endl;
    return NULL;
}

void printShaderInfoLog(GLint shader)
{
    int infoLogLen = 0;
    int charsWritten = 0;
    GLchar *infoLog;

    gl::GetShaderiv(shader, gl::INFO_LOG_LENGTH, &infoLogLen);

    if (infoLogLen > 1)
    {
        infoLog = new GLchar[infoLogLen];
        gl::GetShaderInfoLog(shader,infoLogLen, &charsWritten, infoLog);
        std::cout << "InfoLog:" << std::endl << infoLog << std::endl;
        delete [] infoLog;
    }
}

void printLinkInfoLog(GLint prog)
{
    int infoLogLen = 0;
    int charsWritten = 0;
    GLchar *infoLog;

    gl::GetProgramiv(prog, gl::INFO_LOG_LENGTH, &infoLogLen);

    if (infoLogLen > 1)
    {
        infoLog = new GLchar[infoLogLen];
        // error check for fail to allocate memory omitted
        gl::GetProgramInfoLog(prog,infoLogLen, &charsWritten, infoLog);
        std::cout << "InfoLog:" << std::endl << infoLog << std::endl;
        delete [] infoLog;
    }
}

shaders_t loadShaders(const char * vert_path, const char * frag_path) {
    GLuint f, v;

    char *vs,*fs;

    v = gl::CreateShader(gl::VERTEX_SHADER);
    f = gl::CreateShader(gl::FRAGMENT_SHADER);

    // load shaders & get length of each
    GLint vlen;
    GLint flen;
    vs = loadFile(vert_path,vlen);
    fs = loadFile(frag_path,flen);

    const char * vv = vs;
    const char * ff = fs;

    gl::ShaderSource(v, 1, &vv,&vlen);
    gl::ShaderSource(f, 1, &ff,&flen);

    GLint compiled;

    gl::CompileShader(v);
    gl::GetShaderiv(v, gl::COMPILE_STATUS, &compiled);
    if (!compiled)
    {
        std::cout << "Vertex shader not compiled." << std::endl;
    }
    printShaderInfoLog(v);

    gl::CompileShader(f);
    gl::GetShaderiv(f, gl::COMPILE_STATUS, &compiled);
    if (!compiled)
    {
        std::cout << "Fragment shader not compiled." << std::endl;
    }
    printShaderInfoLog(f);

    shaders_t out; out.vertex = v; out.fragment = f;

    delete [] vs; // dont forget to free allocated memory
    delete [] fs; // we allocated this in the loadFile function...

    return out;
}

void attachAndLinkProgram( GLuint program, shaders_t shaders) {
    gl::AttachShader(program, shaders.vertex);
    gl::AttachShader(program, shaders.fragment);

    gl::LinkProgram(program);
    GLint linked;
    gl::GetProgramiv(program,gl::LINK_STATUS, &linked);
    if (!linked)
    {
        std::cout << "Program did not link." << std::endl;
    }
    printLinkInfoLog(program);
}

GLuint initShaders(const char* vshaderpath, const char* fshaderpath)
{
    shaders_t shaders = loadShaders(vshaderpath, fshaderpath);
    GLuint shader_program = gl::CreateProgram();
    attachAndLinkProgram(shader_program, shaders);
    return shader_program;
}

GLuint createTexture2D(int width, int height, void* data)
{
    GLuint ret_val = 0;
    gl::GenTextures(1,&ret_val);
    gl::BindTexture(gl::TEXTURE_2D,ret_val);
    gl::TexImage2D(gl::TEXTURE_2D,0,gl::RGBA,width,height,0,gl::RGBA,gl::FLOAT,data);
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MAG_FILTER, gl::NEAREST);
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, gl::NEAREST);
    gl::GenerateMipmap(gl::TEXTURE_2D);
    gl::BindTexture(gl::TEXTURE_2D,0);
    return ret_val;
}

GLuint createBuffer(int size, const float* data, GLenum usage)
{
    GLuint ret_val = 0;
    gl::GenBuffers(1,&ret_val);
    gl::BindBuffer(gl::ARRAY_BUFFER,ret_val);
    gl::BufferData(gl::ARRAY_BUFFER,size*sizeof(float),data,usage);
    gl::BindBuffer(gl::ARRAY_BUFFER,0);
    return ret_val;
}
