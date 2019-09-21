/**********************************************************************
Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
********************************************************************/
#include "shaderManager.h"

ShaderManager::ShaderManager()
{

}

ShaderManager::ShaderManager(std::string assetPath)
{
    this->assetPath = assetPath;
}

ShaderManager::~ShaderManager()
{
    for (auto iter = shadercache_.begin(); iter != shadercache_.end(); ++iter)
    {
//        gl::DeleteProgram(iter->second.getShaderProgId());
    }
}

//ShaderProgram ShaderManager::getProgram(std::string const& name)
//{
//    auto iter = shadercache_.find(name);
//
//    if (iter != shadercache_.end())
//    {
//        return iter->second;
//    }
//    else
//    {
//        shadercache_[name] = ShaderProgram(name);
//        return shadercache_[name];
//    }
//}

std::shared_ptr<ShaderProgram> ShaderManager::getProgramPointer(std::string const& name)
{
    auto iter = shadercache_.find(name);
    
    if (iter != shadercache_.end())
    {
        return iter->second;
    }
    else
    {
        shadercache_[name] = std::make_shared<ShaderProgram>(assetPath, name);
        return shadercache_[name];
    }
}
