#include <iostream>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glm/ext.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/norm.hpp"
#include "geometry.h"
#include "materialInfo.h"
#include "mesh.h"
#include "utilities.h"
#include "EigenGLMUtils.h"

extern "C" {
#include "utilities/rigidBodyKinematics.h"
}

#ifndef M_PI
#define M_PI 3.141592653589793
#endif
#ifndef AU
#define AU                  149597870.693 /* astronomical unit in units of kilometers */
#endif
#ifndef SOLAR_FLUX_EARTH
#define SOLAR_FLUX_EARTH    1372.5398 /* solar flux at earth in units of watts/meters^2 */
#endif

//#define DEBUG_OPENGL

Geometry::Geometry() : m_rootNode(new Node)
    , m_lightIntensity(glm::vec3(1))
    , m_bbox_dirty(true)
{
    m_rootNode->name = "root";
    m_rootNode->transformation = glm::mat4x4(1);
    m_rootNode->idx = 0;
    m_modelMatrix = glm::mat4x4(1.0f);
    nodeIdxCounter = 0;
}

Geometry::Geometry(std::string pathToModelFile)
: Geometry()
{
    this->load(pathToModelFile);
}

Geometry::~Geometry()
{
    this->destroyGeometryBufferObjects();
}

void Geometry::setTexture(GLuint texObj)
{
    
}

bool Geometry::load(std::string pathToFile)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(pathToFile, aiProcess_FindInvalidData |
                                             aiProcess_JoinIdenticalVertices |
                                             aiProcess_GenNormals |
//                                             aiProcess_FixInfacingNormals |
                                             aiProcess_Triangulate
                                             //                                             /*aiProcess_ImproveCacheLocality | */
                                             /*aiProcess_CalcTangentSpace | */
                                             /*aiProcess_GenSmoothNormals*/
                                             /*aiProcess_GenUVCoords | */
                                             /*aiProcess_FlipUVs | */
                                             /*aiProcess_SortByPType*/);
    if(!scene) {
        std::cout << "Error loading file: (assimp:) " << importer.GetErrorString() << std::endl;
        return false;
    }
    
    if(scene->HasMaterials()) {
        for(unsigned int i = 0; i < scene->mNumMaterials; i++) {
            std::shared_ptr<MaterialInfo> material = processMaterial(scene->mMaterials[i]);
            m_materials.push_back(material);
        }
    }
    
    if(scene->HasMeshes()) {
        for(unsigned int i = 0; i < scene->mNumMeshes; i++) {
            m_meshes.push_back(processMesh(scene->mMeshes[i]));
        }
    } else {
        std::cout << "Error: No meshes found" << std::endl;
        return false;
    }
    
    if(scene->mRootNode != NULL) {
        Node *rootNode = new Node;
        processNode(scene, scene->mRootNode, 0, *rootNode);
        m_rootNode.reset(rootNode);
    } else {
        std::cout << "Error loading model" << std::endl;
        return false;
    }
    
    this->computeNodeBBox(m_rootNode.get());
    
    return true;
}

bool Geometry::initializeWithShaderId(ShaderProgram program)
{
    /* Create and bind vertex array object to current gl context
     (there can only be one per gl context) */
    gl::GenVertexArrays(1, &m_vertexArrayObjectID);
    gl::BindVertexArray(m_vertexArrayObjectID);
    this->createBuffers();
    this->m_shaderProgram = ShaderProgram(program);
    unsigned int index = gl::GetUniformBlockIndex(m_shaderProgram.getShaderProgId(), "nodeTransforms");CHECK_GL_ERROR;
    gl::UniformBlockBinding(m_shaderProgram.getShaderProgId(), index, 0);CHECK_GL_ERROR;
    // Call to unbind
    gl::BindVertexArray(0);
    
    return true;
}

bool Geometry::initializeWithShaderIdPtr(std::shared_ptr<ShaderProgram> program)
{
    /* Create and bind vertex array object to current gl context
     (there can only be one per gl context) */
    gl::GenVertexArrays(1, &m_vertexArrayObjectID);
    gl::BindVertexArray(m_vertexArrayObjectID);
    this->m_shaderProgramPtr = program;
    this->createBuffers();
    
    
//    unsigned int index = gl::GetUniformBlockIndex(m_shaderProgramPtr->getShaderProgId(), "nodeTransforms"); CHECK_GL_ERROR;
//    gl::UniformBlockBinding(m_shaderProgramPtr->getShaderProgId(), index, 0); CHECK_GL_ERROR;
    
    // Call to unbind
    gl::BindVertexArray(0);
    
    return true;
}

void Geometry::destroyGeometryBufferObjects()
{
    // Delete the buffer objects
    gl::DeleteBuffers(4, m_buffers);
    // Delete shader program
    gl::DeleteProgram(this->m_shaderProgram.getShaderProgId());
    /* Delete the vertex array object (deleting this
     does not also delete the buffer objects so both
     must be deleted. */
    gl::DeleteVertexArrays(1, &m_vertexArrayObjectID);
}

void Geometry::cleanup()
{
}

void Geometry::draw(glm::dmat4x4 cameraMatrix, glm::dmat4x4 projectionMatrix, float scaleFactor)
{
    gl::BindVertexArray(m_vertexArrayObjectID); CHECK_GL_ERROR;
//    this->updateBuffers(); CHECK_GL_ERROR;
    this->bindBuffers(); CHECK_GL_ERROR;
    this->m_shaderProgramPtr->enable(); CHECK_GL_ERROR;
    
    this->drawNode(m_rootNode.get(), cameraMatrix, projectionMatrix);
    
    this->unBindBuffers();
    gl::BindVertexArray(0); CHECK_GL_ERROR;
    gl::UseProgram(0); CHECK_GL_ERROR;
    gl::Finish(); CHECK_GL_ERROR;
}

void Geometry::drawNode(Geometry::Node *node,
                        glm::dmat4x4 cameraMatrix, glm::dmat4 projectionMatrix)
{
    GLenum err;
    if (node->meshes.size() > 0){
        glm::dmat4 modelViewMatrix = cameraMatrix * m_modelMatrix;
        glm::dmat4 normalMatrix = glm::transpose(glm::inverse(modelViewMatrix));
        
        GLuint id = this->m_shaderProgramPtr->getShaderProgId();
        
//        m_dcm_SB[3][1] = cameraMatrix[3][1];
//        m_dcm_SB[3][2] = cameraMatrix[3][2];
        
        gl::UniformMatrix4dv(gl::GetUniformLocation(id, "modelMatrix"), 1, gl::FALSE_, glm::value_ptr(m_modelMatrix)); CHECK_GL_ERROR;
        gl::UniformMatrix4fv(gl::GetUniformLocation(id, "nodeMatrix"), 1, gl::FALSE_, glm::value_ptr(node->transformation)); CHECK_GL_ERROR;
        gl::UniformMatrix4dv(gl::GetUniformLocation(id, "sunModelMatrix"), 1, gl::FALSE_, glm::value_ptr(m_dcm_SB)); CHECK_GL_ERROR;
        gl::UniformMatrix4dv(gl::GetUniformLocation(id, "viewMatrix"), 1, gl::FALSE_, glm::value_ptr(cameraMatrix)); CHECK_GL_ERROR;
        gl::UniformMatrix3dv(gl::GetUniformLocation(id, "normalMatrix"), 1, gl::FALSE_, glm::value_ptr(normalMatrix)); CHECK_GL_ERROR;
        gl::UniformMatrix4dv(gl::GetUniformLocation(id, "projectionMatrix"), 1, gl::FALSE_, glm::value_ptr(projectionMatrix)); CHECK_GL_ERROR;
        gl::Uniform3dv(gl::GetUniformLocation(id, "sHat_B"), 1, glm::value_ptr(rHat_SB_N)); CHECK_GL_ERROR;
        
        for(int i = 0; i < node->meshes.size(); i++) {
            gl::Uniform1f(gl::GetUniformLocation(id, "rho_s"), node->meshes[i]->material->rhoS); CHECK_GL_ERROR;
            gl::Uniform1f(gl::GetUniformLocation(id, "rho_d"), node->meshes[i]->material->rhoD); CHECK_GL_ERROR;
            
#ifdef DEBUG_OPENGL
            gl::Enable(gl::RASTERIZER_DISCARD);
#endif
            gl::DrawElements(node->meshes[i]->primitiveType,
                             node->meshes[i]->indexCount,
                             gl::UNSIGNED_INT,
                             (const void *)(node->meshes[i]->indexOffset * sizeof(unsigned int))); CHECK_GL_ERROR;
        }
    }
    
    // Recursively draw this node's child nodes
    for(int i = 0; i < node->nodes.size(); i++) {
        drawNode(&node->nodes[i], cameraMatrix, projectionMatrix);
    }
    while ((err = gl::GetError()) != gl::NO_ERROR_) {
        std::cerr << "OpenGL error: " << err << std::endl;
    }
}

void Geometry::setDefaultMaterial(std::shared_ptr<MaterialInfo> material)
{
    m_defaultMaterial->ambientColor = material->ambientColor;
    m_defaultMaterial->diffuseColor = material->diffuseColor;
    m_defaultMaterial->specularColor = material->specularColor;
    m_defaultMaterial->shininess = material->shininess;
}

std::shared_ptr<Geometry::Node> Geometry::getRootNode()
{
    return m_rootNode;
}

std::shared_ptr<MaterialInfo> Geometry::addMaterial(std::string name, glm::vec4 ambientColor, glm::vec4 diffuseColor, glm::vec4 specularColor, float shininess)
{
    std::shared_ptr<MaterialInfo> m(new MaterialInfo);
    m->name = name;
    m->ambientColor = ambientColor;
    m->diffuseColor = diffuseColor;
    m->specularColor = specularColor;
    m->shininess = shininess;
    m_materials.push_back(m);
    return m;
}

void Geometry::updateMaterial(std::string name, float rho_s, float rho_d)
{
    std::vector<std::shared_ptr<MaterialInfo>>::iterator it;
    for(it = this->m_materials.begin(); it != this->m_materials.end(); it++)
    {
        if ((*it)->name == name) {
            (*it)->rhoD = rho_d;
            (*it)->rhoS = rho_s;
        }
    }
}

void Geometry::allocate(bool isAllocated, std::shared_ptr<Mesh> mesh, int &numVertices, int &numIndices)
{
    if(!isAllocated) {
        mesh->indexOffset = m_indices.size();
        mesh->indexCount = numIndices;
        mesh->vertexOffset = m_vertices.size();
        mesh->vertexCount = numVertices;
    }
    
    // If the new set of points is larger than the mesh vertex/indices counts then we need
    // to resize the geometry's knwoledge of number of vertices, indixes, normals and
    // texturecoords. This could also be done when numeVertices != mesh->vertexCount
    // however, it is likely less computationally expensive to simply use the oversized
    // mesh arrays than to resize them each time they change.
    if (numVertices > (int)mesh->vertexCount) {
        mesh->vertexCount = numVertices;
        int stopVertex = mesh->vertexOffset + numVertices;
        m_vertices.resize(stopVertex);
        m_normals.resize(stopVertex);
        m_textureCoords.resize(stopVertex);
    }
    if (numIndices > (int)mesh->indexCount) {
        mesh->indexCount = numIndices;
        int stopIndex = mesh->indexOffset + numIndices;
        m_indices.resize(stopIndex);
    }
    
    if(!isAllocated) {
        int stopIndex = mesh->indexOffset + numIndices;
        int stopVertex = mesh->vertexOffset + numVertices;
        m_vertices.resize(stopVertex);
        m_normals.resize(stopVertex);
        m_textureCoords.resize(stopVertex);
        m_indices.resize(stopIndex);
    }
    
}

std::string readFile(const char *filePath) {
    std::string content;
    std::ifstream fileStream(filePath, std::ios::in);
    
    if(!fileStream.is_open()) {
        std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl;
        return "";
    }
    
    std::string line = "";
    while(!fileStream.eof()) {
        std::getline(fileStream, line);
        content.append(line + "\n");
    }
    
    fileStream.close();
    return content;
}

int Geometry::getBuffer(GeometryBuffer buffer)
{
    return m_buffers[buffer];
}

bool Geometry::createBuffers()
{
    bool success = true;
    //    GLuint ret_val = 0;
    //    gl::GenBuffers(1,&ret_val);
    //    gl::BindBuffer(gl::ARRAY_BUFFER,ret_val);
    //    gl::BufferData(gl::ARRAY_BUFFER,size*sizeof(float),data,usage);
    //    gl::BindBuffer(gl::ARRAY_BUFFER,0);
    
    // Create the buffers for the vertices atttributes
    gl::GenBuffers(numElems(m_buffers), m_buffers);
    
    // Generate and populate the buffers
    // Vertex buffer and attributes
    gl::BindBuffer(gl::ARRAY_BUFFER, m_buffers[POS_VB]);
    gl::BufferData(gl::ARRAY_BUFFER, sizeof(m_vertices[0]) * m_vertices.size(), &m_vertices[0],
                   gl::STATIC_DRAW);
    gl::EnableVertexAttribArray(0);
    gl::VertexAttribPointer(0, 3, gl::FLOAT, gl::FALSE_, 0, 0);
//    gl::VertexAttribLPointer(0, 3, gl::DOUBLE, 0, 0);
    
    // Normal buffer and attributes
//    gl::BindBuffer(gl::ARRAY_BUFFER, m_buffers[NORMAL_VB]);
//    gl::BufferData(gl::ARRAY_BUFFER, sizeof(m_normals[0]) * m_normals.size(), &m_normals[0],
//                   gl::STATIC_DRAW);
//    gl::EnableVertexAttribArray(2);
//    //    gl::VertexAttribPointer(1, 3, gl::FLOAT, gl::FALSE_, 0, 0);
//    gl::VertexAttribLPointer(2, 3, gl::DOUBLE, 0, 0);
    
    gl::BindBuffer(gl::ARRAY_BUFFER, m_buffers[NORMAL_VB]);
    gl::BufferData(gl::ARRAY_BUFFER, sizeof(m_normals[0]) * m_normals.size(), &m_normals[0],
                   gl::STATIC_DRAW);
    gl::EnableVertexAttribArray(1);
    gl::VertexAttribPointer(1, 3, gl::FLOAT, gl::FALSE_, 0, 0);
//    gl::VertexAttribLPointer(2, 3, gl::DOUBLE, 0, 0);

    
    // Texture buffer and attributes
    gl::BindBuffer(gl::ARRAY_BUFFER, m_buffers[TEXCOORD_VB]);
    gl::BufferData(gl::ARRAY_BUFFER, sizeof(m_textureCoords[0]) * m_textureCoords.size(), &m_textureCoords[0],
                   gl::STATIC_DRAW);
    gl::EnableVertexAttribArray(2);
    gl::VertexAttribPointer(2, 2, gl::FLOAT, gl::FALSE_, 0, 0);
//    gl::VertexAttribLPointer(4, 2, gl::DOUBLE, 0, 0);
    
    // Index buffer
    gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, m_buffers[INDEX_BUFFER]);
    gl::BufferData(gl::ELEMENT_ARRAY_BUFFER, sizeof(m_indices[0]) * m_indices.size(), &m_indices[0],
                   gl::STATIC_DRAW);
    
    // Transform feedback buffer
    //    gl::BindBuffer(gl::ARRAY_BUFFER, m_buffers[TRANSFEEDBACK_BUFFER]);
    //    gl::BufferData(gl::ARRAY_BUFFER, sizeof(m_vertices[0]) * m_indices.size()*m_sizeTFVars, nullptr, gl::STATIC_READ);
    
//    gl::GenBuffers(1, &m_buffers[GeometryBuffer::UNIFORM_BUFFER]);CHECK_GL_ERROR;
    
//    gl::BindBuffer(gl::UNIFORM_BUFFER, m_buffers[GeometryBuffer::UNIFORM_BUFFER]);CHECK_GL_ERROR;
//    gl::BufferData(gl::UNIFORM_BUFFER, sizeof(glm::dmat4)*m_meshes.size(), NULL, gl::STATIC_DRAW);CHECK_GL_ERROR; // allocate 150 bytes of memory
    
//    gl::BindBuffer(gl::UNIFORM_BUFFER, 0);CHECK_GL_ERROR;
    return success;
}

void Geometry::updateBuffers()
{
    gl::BindBuffer(gl::ARRAY_BUFFER, m_buffers[POS_VB]);
    gl::BufferData(gl::ARRAY_BUFFER, m_vertices.size() * sizeof(m_vertices[0]), m_vertices.data(), gl::STATIC_DRAW);
    gl::BindBuffer(gl::ARRAY_BUFFER, 0);
    
    gl::BindBuffer(gl::ARRAY_BUFFER, m_buffers[NORMAL_VB]);
    gl::BufferData(gl::ARRAY_BUFFER, m_normals.size() * sizeof(m_normals[0]), m_normals.data(), gl::STATIC_DRAW);
    gl::BindBuffer(gl::ARRAY_BUFFER, 0);

//    gl::BindBuffer(gl::ARRAY_BUFFER, m_buffers[NORMAL_VB]);
//    gl::BufferData(gl::ARRAY_BUFFER, m_faceNormals.size() * sizeof(m_faceNormals[0]), m_faceNormals.data(), gl::STATIC_DRAW);
//    gl::BindBuffer(gl::ARRAY_BUFFER, 0);
    
    gl::BindBuffer(gl::ARRAY_BUFFER, m_buffers[TEXCOORD_VB]);
    gl::BufferData(gl::ARRAY_BUFFER, m_textureCoords.size() * sizeof(m_textureCoords[0]), m_textureCoords.data(), gl::STATIC_DRAW);
    gl::BindBuffer(gl::ARRAY_BUFFER, 0);
    
    gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, m_buffers[INDEX_BUFFER]);
    gl::BufferData(gl::ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(m_indices[0]), m_indices.data(), gl::STATIC_DRAW);
    gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, 0);
    
//    gl::BindBuffer(gl::UNIFORM_BUFFER, m_buffers[GeometryBuffer::UNIFORM_BUFFER]);
//
//    for (int i = 0; i < m_rootNode->nodes.size(); i++)
//    {
//        gl::BufferSubData(gl::UNIFORM_BUFFER, i*sizeof(glm::mat4), sizeof(glm::mat4), &m_rootNode->nodes[i].transformation);
//    }
//    gl::BindBuffer(gl::UNIFORM_BUFFER, 0);
}

void Geometry::bindBuffers()
{
//    gl::BindBuffer(gl::ARRAY_BUFFER, m_buffers[NORMAL_VB]); CHECK_GL_ERROR;
    gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, m_buffers[INDEX_BUFFER]); CHECK_GL_ERROR;
//    gl::BindBuffer(gl::UNIFORM_BUFFER, m_buffers[GeometryBuffer::UNIFORM_BUFFER]); CHECK_GL_ERROR;
    //    gl::BindBuffer(gl::ARRAY_BUFFER, m_buffers[TEXCOORD_VB]); CHECK_GL_ERROR;
    //    gl::BindBufferBase(gl::TRANSFORM_FEEDBACK_BUFFER, 0, m_buffers[TRANSFEEDBACK_BUFFER]); CHECK_GL_ERROR;
}

void Geometry::unBindBuffers()
{
//    gl::BindBuffer(gl::ARRAY_BUFFER, 0); CHECK_GL_ERROR;
    gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, 0); CHECK_GL_ERROR;
//    gl::BindBuffer(gl::UNIFORM_BUFFER, 0); CHECK_GL_ERROR;
    //    gl::BindBuffer(gl::ARRAY_BUFFER, 0); CHECK_GL_ERROR;
    //    gl::BindBufferBase(gl::TRANSFORM_FEEDBACK_BUFFER, 0, 0); CHECK_GL_ERROR;
}

std::shared_ptr<MaterialInfo> Geometry::processMaterial(aiMaterial *material)
{
    std::shared_ptr<MaterialInfo> m(new MaterialInfo);
    aiString mname;
    material->Get(AI_MATKEY_NAME, mname);
    if(mname.length > 0) {
        m->name = mname.C_Str();
    }
    
    aiColor3D ambientColor(0.0f, 0.0f, 0.0f);
    aiColor3D diffuseColor(0.0f, 0.0f, 0.0f);
    aiColor3D specularColor(0.0f, 0.0f, 0.0f);
    float shininess = 0.0;
    
    material->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor);
    material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
    material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);
    material->Get(AI_MATKEY_SHININESS, shininess);
    
    m->ambientColor = glm::vec4(ambientColor.r, ambientColor.g, ambientColor.b, 1.0);
    m->diffuseColor = glm::vec4(diffuseColor.r, diffuseColor.g, diffuseColor.b, 1.0);
    m->specularColor = glm::vec4(specularColor.r, specularColor.g, specularColor.b, 1.0);
    m->shininess = shininess;
    m->rhoD = diffuseColor.r;
    m->rhoS = specularColor.r;
    
    if(m->shininess == 0.0) {
        m->shininess = 5.0;
    }
    
    return m;
}

std::shared_ptr<Mesh> Geometry::processMesh(aiMesh *mesh)
{
    std::shared_ptr<Mesh> newMesh(new Mesh);
    newMesh->name = mesh->mName.length != 0 ? mesh->mName.C_Str() : "";
    newMesh->indexOffset = m_indices.size();
    unsigned int indexCountBefore = m_indices.size();
    int vertexIndexOffset = m_vertices.size();
    newMesh->vertexOffset = m_vertices.size();
    newMesh->vertexCount = mesh->mNumVertices;
    
    // Get vertices
    if(mesh->mNumVertices > 0) {
        for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
            aiVector3D &vec = mesh->mVertices[i];
            m_vertices.push_back(glm::vec3(vec.x, vec.y, vec.z));
        }
    }
    
    // Get normals
    if(mesh->HasNormals()) {
        for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
            aiVector3D &vec = mesh->mNormals[i];
            m_normals.push_back(glm::vec3(vec.x, vec.y, vec.z));
        }
    }
    
    // Get texture coordinates
    for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
        if(mesh->HasTextureCoords(0)){
            aiVector3D &vec = mesh->mTextureCoords[0][i];
            m_textureCoords.push_back(glm::vec2(vec.x, vec.y));
        } else {
            m_textureCoords.push_back(glm::vec2(0, 0));
        }
    }
    
    // Get mesh indices
    for(unsigned int i = 0;  i < mesh->mNumFaces; i++) {
        aiFace *face = &mesh->mFaces[i];
        if(face->mNumIndices != 3) {
            std::cout << "Warning: Mesh face with not exactly 3 indices, ignoring this primitive." << std::endl;
            continue;
        }
        unsigned int idx0 = face->mIndices[0] + vertexIndexOffset;
        unsigned int idx1 = face->mIndices[1] + vertexIndexOffset;
        unsigned int idx2 = face->mIndices[2] + vertexIndexOffset;
        m_indices.push_back(idx0);
        m_indices.push_back(idx1);
        m_indices.push_back(idx2);
        
        // Compute centroid for the face
        this->m_centroids.push_back((this->m_vertices[idx0]
                                     + this->m_vertices[idx1]
                                     + this->m_vertices[idx2])/3.0f);
        
        // Get the normal of any of the indicies which make up the face.
        // We can choose any of the indicies as they each have the same
        // given we are importing the model through ASSIMP with either
        // normals generated in Blender or aiProcess_GenNormals computing
        // per face 'hard' normals rather than vetex averaged 'smooth' normals.
        this->m_faceNormals.push_back(m_normals.at(idx0));
//        std::cout << "this->m_faceNormals " << this->m_faceNormals.back()[0] << " " << this->m_faceNormals.back()[1] << " " << this->m_faceNormals.back()[2] << std::endl;
//        this->m_faceNormals.push_back(glm::vec3(1.0, 0.0, 0.0));
       
        // compute area of face
        glm::vec3 edge1 = this->m_vertices[idx1] - this->m_vertices[idx0];
        glm::vec3 edge2 = this->m_vertices[idx2] - this->m_vertices[idx0];
        glm::vec3 crossProdEdges = glm::cross(edge1, edge2);
        this->m_areas.push_back(0.5*glm::length(crossProdEdges));
    }
    
    newMesh->indexCount = m_indices.size() - indexCountBefore;
    newMesh->material = m_materials.at(mesh->mMaterialIndex);
    newMesh->primitiveType = gl::TRIANGLES;
    
    return newMesh;
}

void Geometry::processNode(const aiScene *scene, aiNode *node, Geometry::Node *parentNode, Geometry::Node &newNode)
{
    newNode.name = node->mName.length != 0 ? node->mName.C_Str() : "";
    newNode.transformation = assimpMat2Glm(node->mTransformation);
    newNode.meshes.resize(node->mNumMeshes);
    nodeIdxCounter++;
    newNode.idx = nodeIdxCounter;
    for(unsigned int i = 0; i < node->mNumMeshes; i++) {
        std::shared_ptr<Mesh> mesh = m_meshes[node->mMeshes[i]];
        newNode.meshes[i] = mesh;
    }
    for(unsigned int i = 0; i < node->mNumChildren; i++) {
        newNode.nodes.push_back(Node());
        processNode(scene, node->mChildren[i], parentNode, newNode.nodes[i]);
    }
}

void Geometry::computeNodeBBox(Geometry::Node *node)
{
    // If this is a leaf node then compute the bbox and return
    if (node->nodes.size() == 0)
    {
        std::vector<glm::dvec3> vertices;
        for(unsigned int i = 0; i < node->meshes.size(); i++) {
            std::vector<glm::vec3>::const_iterator first = m_vertices.begin() + node->meshes[i]->vertexOffset;
            std::vector<glm::vec3>::const_iterator last = m_vertices.begin() + node->meshes[i]->vertexOffset + node->meshes[i]->vertexCount;
            vertices.insert(vertices.end(), first, last);
        }
        node->bbox = BoundingBox(vertices);
        node->bbox.transform(glm::dmat4(node->transformation));
        
        return;
    }
    
    // This is not a leaf so we loop through all child nodes and call the function again
    std::vector<glm::dvec3> bBoxUnion;
    std::vector<Geometry::Node>::iterator nodeItr = node->nodes.begin();
    for(nodeItr = node->nodes.begin(); nodeItr != node->nodes.end(); nodeItr++)
    {
        this->computeNodeBBox(&(*nodeItr));
        // for each node accumulate the child bounding boxes
        
        std::vector<glm::dvec3> tmpBoxVertices = nodeItr->bbox.getVec3BBoxQuadMeshVertices();
        bBoxUnion.insert( bBoxUnion.end(), tmpBoxVertices.begin(), tmpBoxVertices.end() );
    }
    node->bbox = BoundingBox(bBoxUnion);
    node->bbox.transform(glm::dmat4(node->transformation));
    return;
}

BoundingBox Geometry::getBoundingBox()
{
    if (m_bbox_dirty) this->computeNodeBBox(this->m_rootNode.get());
    return m_rootNode->bbox;
}

std::unordered_set<std::string> Geometry::getTextureFileNames()
{
    std::unordered_set<std::string> result;
    this->getMeshTextures(&result, m_rootNode.get());
    return result;
}

void Geometry::getMeshTextures(std::unordered_set<std::string> *textureNames, const Node *node)
{
    for(int i = 0; i < node->meshes.size(); i++) {
        if (!textureNames->count(node->meshes[i]->textureFile))
        textureNames->insert(node->meshes[i]->textureFile);
    }
    for(int i = 0; i < node->nodes.size(); i++) {
        this->getMeshTextures(textureNames, &node->nodes[i]);
    }
}

void Geometry::addTexture(std::string name, int ID)
{
    //    std::vector<std::string> textures = this->getTextureFileNames();
    
    return;
}

glm::dmat4x4 Geometry::removeTranslationAndScale(glm::dmat4x4 transform)
{
    glm::dmat4x4 normalTransform = transform;
    
    // set 4th row
    normalTransform[3] = glm::dvec4(0, 0, 0, 1);
    
    // set 4th column
    normalTransform[0][3] = 0;
    normalTransform[1][3] = 0;
    normalTransform[2][3] = 0;
    normalTransform[3][3] = 1;
    
    return normalTransform;
}

glm::mat4x4 Geometry::assimpMat2Glm(aiMatrix4x4 input)
{
    float vals[16] = {0.0f};
    for (unsigned int i=0; i<4; ++i) {
        for (unsigned int j=0; j<4; ++j)
        {
            vals[i*4+j] = input[i][j];
        }
    }
    return glm::make_mat4x4(vals);
}

glm::dvec3 Geometry::sunPosition()
{
    return glm::normalize(this->r_SB_B)*this->m_distance2Sun;
}

void Geometry::setBodyToSunInInertialVector(double r_SB_N[3])
{
    this->r_SB_N[0] = r_SB_N[0];
    this->r_SB_N[1] = r_SB_N[1];
    this->r_SB_N[2] = r_SB_N[2];
    this->rHat_SB_N = glm::normalize(this->r_SB_N);
    m_distance2Sun = glm::l2Norm(this->r_SB_N);
    m_solarFlux = SOLAR_FLUX_EARTH*((AU/m_distance2Sun)*(AU/m_distance2Sun));
}

void Geometry::setBodyToSunInBodyVector(double r_SB_B[3])
{
    this->r_SB_B[0] = r_SB_B[0];
    this->r_SB_B[1] = r_SB_B[1];
    this->r_SB_B[2] = r_SB_B[2];
    this->rHat_SB_B = glm::normalize(this->r_SB_B);
    m_distance2Sun = glm::l2Norm(this->r_SB_B);
    m_solarFlux = SOLAR_FLUX_EARTH*((AU/m_distance2Sun)*(AU/m_distance2Sun));
}

void Geometry::setDcmSB(glm::dmat4 mat)
{
    this->m_dcm_SB = mat;
}

glm::dmat4 Geometry::getDcmSB()
{
    return this->m_dcm_SB;
}

void Geometry::setAttitude(glm::dvec3 sigma)
{
    m_sigma_BN = sigma;
    double C[3][3];
    double tmp_sigma[3];
    tmp_sigma[0] = sigma[0];
    tmp_sigma[1] = sigma[1];
    tmp_sigma[2] = sigma[2];
    MRP2C(tmp_sigma, C);
    m_modelMatrix[0][0] = C[0][0];
    m_modelMatrix[0][1] = C[0][1];
    m_modelMatrix[0][2] = C[0][2];
    m_modelMatrix[1][0] = C[1][0];
    m_modelMatrix[1][1] = C[1][1];
    m_modelMatrix[1][2] = C[1][2];
    m_modelMatrix[2][0] = C[2][0];
    m_modelMatrix[2][1] = C[2][1];
    m_modelMatrix[2][2] = C[2][2];
}

//void Geometry::updateUniformBufferTransforms(Node &node, unsigned int count){
//    
//    if (node.nodes.size() == 0)
//    {
//        
//    }
//    for(unsigned int i = 0; i < node.nodes.size(); i++) {
//        this->findNodeAndSetDcm(name, node.nodes[i], dcm);
//    }
//}

void Geometry::setNodeDcm(std::string name, glm::dmat4 dcm)
{
    for(unsigned int i = 0; i < m_rootNode->nodes.size(); i++)
    {
        this->findNodeAndSetDcm(name, m_rootNode->nodes[i], dcm);
    }
}

void Geometry::findNodeAndSetDcm(std::string name, Node &node, glm::dmat4 dcm)
{
    if (node.name == name)
    {
        node.transformation = dcm;
        m_bbox_dirty = true;
    }
    for(unsigned int i = 0; i < node.nodes.size(); i++) {
        this->findNodeAndSetDcm(name, node.nodes[i], dcm);
    }
}

int Geometry::numPrimitives()
{
    return (int)this->m_indices.size()/3; // this assumes only triangle primitives
}

size_t Geometry::byteSizeOfFaceNormals()
{
    return sizeof(m_faceNormals[0]) * m_faceNormals.size();
}

size_t Geometry::byteSizeOfCentroids()
{
    return sizeof(m_centroids[0]) * m_centroids.size();
}

size_t Geometry::byteSizeOfAreas()
{
    return sizeof(m_areas[0]) * m_areas.size();
}

std::vector<glm::vec3> Geometry::getVertices()
{
    return this->m_vertices;
}

std::vector<glm::vec3> Geometry::getCentroids()
{
    return this->m_centroids;
}

std::vector<double> Geometry::getAreas()
{
    return this->m_areas;
}

std::vector<glm::vec3> Geometry::getfaceNormals()
{
    return this->m_faceNormals;
}

std::shared_ptr<Mesh> Geometry::getMesh(unsigned int idx)
{
    return this->m_meshes[idx];
}
