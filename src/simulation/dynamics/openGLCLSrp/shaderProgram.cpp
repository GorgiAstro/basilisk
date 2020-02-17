/*

	Copyright 2011 Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>
#include "util.h"
#include "utilities.h"
#include "shaderProgram.h"

static void loadFileContents(std::string const& name, std::vector<char>& contents, bool binary = false)
{
    std::ifstream in(name, std::ios::in | (std::ios_base::openmode)(binary?std::ios::binary : 0));
    
    if (in)
    {
        contents.clear();
        
        std::streamoff beg = in.tellg();
        
        in.seekg(0, std::ios::end);
        
        std::streamoff fileSize = in.tellg() - beg;
        
        in.seekg(0, std::ios::beg);
        
        contents.resize(static_cast<unsigned>(fileSize));
        
        in.read(&contents[0], fileSize);
    }
    else
    {
        throw std::runtime_error("Cannot read the contents of a file");
    }
}

ShaderProgram::ShaderProgram()
{
    m_shaderProg = 0;
}

ShaderProgram::ShaderProgram(std::string assetDir, std::string name) : ShaderProgram()
{
    this->name = name;
    this->assetPath = assetDir + "shaders";
    this->m_shaderProg = this->compileProgram();
}

ShaderProgram::~ShaderProgram()
{
    // Delete the intermediate shader objects that have been added to the program
    // The list will only contain something if shaders were compiled but the object itself
    // was destroyed prior to linking.
    for (ShaderObjList::iterator it = m_shaderObjList.begin() ; it != m_shaderObjList.end() ; it++)
    {
        gl::DeleteShader(*it); CHECK_GL_ERROR;
    }

    if (m_shaderProg != 0)
    {
        gl::DeleteProgram(m_shaderProg); CHECK_GL_ERROR;
        m_shaderProg = 0;
    }
}

GLuint ShaderProgram::getShaderProgId()
{
    return this->m_shaderProg;
}

bool ShaderProgram::init()
{
    m_shaderProg = gl::CreateProgram(); CHECK_GL_ERROR;

    if (m_shaderProg == 0) {
        fprintf(stderr, "Error creating shader program\n");
        return false;
    }

    return true;
}

// Use this method to add shaders to the program. When finished - call finalize()
bool ShaderProgram::addShader(GLenum ShaderType, std::string pFilename)
{
    std::string s;
    
    if (!ReadFile(pFilename.c_str(), s)) {
        return false;
    }
    
    GLuint shaderObj = gl::CreateShader(ShaderType);CHECK_GL_ERROR;

    if (shaderObj == 0) {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        return false;
    }

    // Save the shader object - will be deleted in the destructor
    m_shaderObjList.push_back(shaderObj);

    const GLchar* p[1];
    p[0] = s.c_str();
    GLint len = (GLint)s.size();

    gl::ShaderSource(shaderObj, 1, p, &len);CHECK_GL_ERROR;

    gl::CompileShader(shaderObj);CHECK_GL_ERROR;

    GLint result = gl::TRUE_;
    gl::GetShaderiv(shaderObj, gl::COMPILE_STATUS, &result);CHECK_GL_ERROR;
    
    if(result == gl::FALSE_)
    {
        std::vector<char> log;
        
        gl::GetShaderiv(shaderObj, gl::INFO_LOG_LENGTH, &len);CHECK_GL_ERROR;
        
        log.resize(len);
        
        gl::GetShaderInfoLog(shaderObj, len, &result, &log[0]);CHECK_GL_ERROR;
        
        gl::DeleteShader(shaderObj);CHECK_GL_ERROR;
        
        throw std::runtime_error(std::string(log.begin(), log.end()));
        
        return false;
    }

    gl::AttachShader(m_shaderProg, shaderObj);CHECK_GL_ERROR;
    return true;
}


// After all the shaders have been added to the program call this function
// to link and validate the program.
bool ShaderProgram::finalize()
{
    gl::LinkProgram(m_shaderProg);CHECK_GL_ERROR;

    GLint result = gl::TRUE_;
    gl::GetProgramiv(m_shaderProg, gl::LINK_STATUS, &result);CHECK_GL_ERROR;
    if(result == gl::FALSE_)
    {
        GLint length = 0;
        std::vector<char> log;

        gl::GetProgramiv(m_shaderProg, gl::INFO_LOG_LENGTH, &length);CHECK_GL_ERROR;

        log.resize(length);

        gl::GetProgramInfoLog(m_shaderProg, length, &result, &log[0]);CHECK_GL_ERROR;

        gl::DeleteProgram(m_shaderProg);CHECK_GL_ERROR;

        throw std::runtime_error(std::string(log.begin(), log.end()));
    }

//    gl::ValidateProgram(m_shaderProg);
//    gl::GetProgramiv(m_shaderProg, gl::VALIDATE_STATUS, &Success);
//    if (!Success) {
//        gl::GetProgramInfoLog(m_shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
//        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
//     //   return false;
//    }

    // Delete the intermediate shader objects that have been added to the program
    for (ShaderObjList::iterator it = m_shaderObjList.begin() ; it != m_shaderObjList.end() ; it++) {
        gl::DeleteShader(*it);CHECK_GL_ERROR;
    }

    m_shaderObjList.clear();

    return GLCheckError();
}

static GLuint compileShader(std::vector<GLchar> const& shader_source, GLenum type)
{
    GLuint shader = gl::CreateShader(type); CHECK_GL_ERROR;
    
    GLint len = static_cast<GLint>(shader_source.size());
    GLchar const* source_array = &shader_source[0];
    
    gl::ShaderSource(shader, 1, &source_array, &len); CHECK_GL_ERROR;
    gl::CompileShader(shader); CHECK_GL_ERROR;
    
    GLint result = gl::TRUE_;
    gl::GetShaderiv(shader, gl::COMPILE_STATUS, &result); CHECK_GL_ERROR;
    
    if(result == gl::FALSE_)
    {
        std::vector<char> log;
        
        gl::GetShaderiv(shader, gl::INFO_LOG_LENGTH, &len); CHECK_GL_ERROR;
        
        log.resize(len);
        
        gl::GetShaderInfoLog(shader, len, &result, &log[0]); CHECK_GL_ERROR;
        
        gl::DeleteShader(shader); CHECK_GL_ERROR;
        
        throw std::runtime_error(std::string(log.begin(), log.end()));
        
        return 0;
    }
    
    return shader;
}

GLuint ShaderProgram::compileProgram()
{
    GLuint program = gl::CreateProgram(); CHECK_GL_ERROR;
    
    std::string vsname = this->assetPath + "/" + this->name + ".vsh";
    std::string fsname = this->assetPath + "/" + this->name + ".fsh";
    
    // Need to wrap the shader program here to be exception-safe
    std::vector<GLchar> source;
    
    loadFileContents(vsname, source);
    GLuint vertex_shader = compileShader(source, gl::VERTEX_SHADER);
    
    /// This part is not exception safe:
    /// if the VS compilation succeeded
    /// and PS compilation fails then VS object will leak
    /// fix this by wrapping the shaders into a class
    loadFileContents(fsname, source);
    GLuint frag_shader = compileShader(source, gl::FRAGMENT_SHADER);
    
    gl::AttachShader(program, vertex_shader); CHECK_GL_ERROR;
    gl::AttachShader(program, frag_shader); CHECK_GL_ERROR;
    
    gl::DeleteShader(vertex_shader); CHECK_GL_ERROR;
    gl::DeleteShader(frag_shader); CHECK_GL_ERROR;
//    gl::BindAttribLocation(program, 0, "vertexPosition_modelSpace");
//    gl::BindAttribLocation(program, 1, "vertexNormal_modelSpace");
//    gl::BindAttribLocation(program, 2, "vertexUV");
    
    gl::LinkProgram(program); CHECK_GL_ERROR;
    
    GLint result = gl::TRUE_;
    gl::GetProgramiv(program, gl::LINK_STATUS, &result); CHECK_GL_ERROR;
    
    if(result == gl::FALSE_)
    {
        GLint length = 0;
        std::vector<char> log;
        
        gl::GetProgramiv(program, gl::INFO_LOG_LENGTH, &length); CHECK_GL_ERROR;
        
        log.resize(length);
        
        gl::GetProgramInfoLog(program, length, &result, &log[0]); CHECK_GL_ERROR;
        
        gl::DeleteProgram(program); CHECK_GL_ERROR;
        
        throw std::runtime_error(std::string(log.begin(), log.end()));
    }
    
    return program;
}

void ShaderProgram::enable()
{
    GLint result = gl::TRUE_;
    //GLboolean res = gl::IsProgram(this->m_shaderProg); CHECK_GL_ERROR;
    gl::ValidateProgram(this->m_shaderProg); CHECK_GL_ERROR;
    gl::GetProgramiv(this->m_shaderProg, gl::VALIDATE_STATUS, &result); CHECK_GL_ERROR;
    if(result == gl::FALSE_)
    {
        GLint length = 0;
        std::vector<char> log;
        
        gl::GetProgramiv(m_shaderProg, gl::INFO_LOG_LENGTH, &length);CHECK_GL_ERROR;
        
        log.resize(length);
        
        gl::GetProgramInfoLog(m_shaderProg, length, &result, &log[0]);CHECK_GL_ERROR;
        
        gl::DeleteProgram(m_shaderProg);CHECK_GL_ERROR;
        
        throw std::runtime_error(std::string(log.begin(), log.end()));
    }
    gl::UseProgram(m_shaderProg); CHECK_GL_ERROR;
}


GLint ShaderProgram::getUniformLocation(const char* pUniformName)
{
    GLuint location = gl::GetUniformLocation(m_shaderProg, pUniformName); CHECK_GL_ERROR;

    if (location == INVALID_UNIFORM_LOCATION) {
        fprintf(stderr, "Warning! Unable to get the location of uniform '%s'\n", pUniformName);
    }

    return location;
}

GLint ShaderProgram::getProgramParam(GLint param)
{
    GLint ret;
    gl::GetProgramiv(m_shaderProg, param, &ret);CHECK_GL_ERROR;
    return ret;
}
