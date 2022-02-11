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

#ifndef ShaderProgram_H
#define	ShaderProgram_H

#include <list>
#include <glload/gl_4_1.hpp>

class ShaderProgram
{
public:
    ShaderProgram();
    ShaderProgram(std::string assetDir, std::string name);
    virtual ~ShaderProgram();
    virtual bool init();
    void enable();
    GLuint getShaderProgId();
    
protected:
    GLuint compileProgram();
    bool addShader(GLenum ShaderType, std::string pFilename);
    bool finalize();
    GLint getUniformLocation(const char* pUniformName);
    GLint getProgramParam(GLint param);
    GLuint m_shaderProg;    
    
private:
    typedef std::list<GLuint> ShaderObjList;
    ShaderObjList m_shaderObjList;
    std::string assetPath;
    std::string name;
};

#endif	/* TECHNIQUE_H */

