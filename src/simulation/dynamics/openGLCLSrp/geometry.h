#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <glload/gl_4_1.hpp>
#include <vector>
#include <unordered_set>
#include <memory>
#include "glm/glm.hpp"
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include "shaderProgram.h"
#include "boundingBox.h"

class RenderManager;
class MaterialInfo;
class Mesh;

// Class to be inherited by children of Geometry in order to pass in dynamic parameters
class GeometryUpdateParameters
{
public:
};

typedef enum  {
    INDEX_BUFFER = 0,
    POS_VB = 1,
    NORMAL_VB = 2,
    TEXCOORD_VB = 3,
    TRANSFEEDBACK_BUFFER = 4,
    UNIFORM_BUFFER = 5,
    MAX_BUFFER
} GeometryBuffer;

class Geometry
{
public:
    Geometry();
    Geometry(std::string pathToFile);
    ~Geometry();
    
    glm::dvec3  m_forceSRPTotal;
    glm::dvec3  m_torqueSRPTotal;
    typedef struct Node {
        std::string name;
        uint32_t idx;
        glm::mat4x4 transformation;
        std::vector<std::shared_ptr<Mesh>> meshes;
        std::vector<Node> nodes;
        BoundingBox bbox;
    } Node;

    void setTexture(GLuint texObj);
    bool load(std::string pathToFile);
    bool initialize();
    bool initializeWithShaderId(ShaderProgram program);
    bool initializeWithShaderIdPtr(std::shared_ptr<ShaderProgram> program);
    void destroyGeometryBufferObjects();
    virtual void update(GeometryUpdateParameters *) {}
    void draw(glm::dmat4x4 cameraMatrix, glm::dmat4x4 projectionMatrix, float scaleFactor);
    void cleanup();
    std::unordered_set<std::string> getTextureFileNames();
    std::shared_ptr<MaterialInfo> getDefaultMaterial() {
        return m_defaultMaterial;
    }
    void addTexture(std::string name, int ID);
    void setDefaultMaterial(std::shared_ptr<MaterialInfo> material);
    void setBodyToSunInInertialVector(double r_SB_N[3]);
    void setBodyToSunInBodyVector(double r_SB_B[3]);
    void setAttitude(glm::dvec3 sigma);
    void setNodeDcm(std::string name, glm::dmat4 dcm);
    void setDcmSB(glm::dmat4 mat);
    glm::dmat4 getDcmSB();
    void setInertialPosition(double r_N[3]);
    int numPrimitives();
    size_t byteSizeOfFaceNormals();
    size_t byteSizeOfCentroids();
    size_t byteSizeOfAreas();
    std::vector<glm::vec3> getVertices();
    std::vector<glm::vec3> getCentroids();
    std::vector<double> getAreas();
    std::vector<glm::vec3> getfaceNormals();
    BoundingBox getBoundingBox();
    std::shared_ptr<Mesh> getMesh(unsigned int idx);
    
    RenderManager* renderManager;
    int getBuffer(GeometryBuffer buffer);
    std::shared_ptr<Node> getRootNode();
    void updateMaterial(std::string name, float rho_s, float rho_d);
    
protected:

    std::shared_ptr<MaterialInfo> addMaterial(std::string name, glm::vec4 ambientColor, glm::vec4 diffuseColor, glm::vec4 specularColor,
            float shininess);

    // Allocates (if requested) the buffers for the number of vertices and indices specified
    // if already allocated, adjusts numVertices and numIndices to avoid buffer overrun
    void allocate(bool isAllocate, std::shared_ptr<Mesh> mesh, int &numVertices, int &numIndices);

private:
    std::vector<glm::vec3>                     m_vertices;
    std::vector<glm::vec3>                     m_normals;
    std::vector<glm::vec3>                     m_faceNormals;
    std::vector<glm::vec3>                     m_centroids;
    std::vector<glm::vec2>                     m_textureCoords;
    std::vector<unsigned int>                   m_indices;
    std::vector<double>                         m_areas;
    std::shared_ptr<MaterialInfo>               m_defaultMaterial;
    std::vector<std::shared_ptr<MaterialInfo>>  m_materials;
    std::vector<std::shared_ptr<Mesh>>          m_meshes;
    std::shared_ptr<Node>                       m_rootNode;
    ShaderProgram                               m_shaderProgram;
    std::shared_ptr<ShaderProgram>              m_shaderProgramPtr;
//    std::vector<std::shared_ptr<Texture>>       m_textures;
    glm::dmat4                                  m_modelMatrix;
    
    #define numElems(x) (sizeof(x) / sizeof(x[0]))
    GLuint      m_vertexArrayObjectID;
    GLuint      m_buffers[6];
    
    glm::dvec3  r_SB_B;
    glm::dvec3  rHat_SB_B;
    glm::dvec3  r_SB_N;  //!< [km] position vector from the sun
    glm::dvec3  rHat_SB_N;
    glm::dvec3  m_sigma_BN; //!< [unitless] spacecraft attitude MRP vector
    glm::dmat4  m_dcm_SB;
    glm::dvec3  m_lightIntensity;                   //!< [unitless] intensity of light to use for visual display of spacecraft
    double      m_solarFlux;                        //!< [w/m^2] solar flux
    double      m_distance2Sun;                     //!< [km] radius from spacecraft to the sun
    bool        m_bbox_dirty;
    
    bool createShaders();
    bool createBuffers();
    void updateBuffers();
    void updateUniformBufferTransforms(Node &node, unsigned int count);
    void bindBuffers();
    void unBindBuffers();
    void drawNode(Node *node, glm::dmat4x4 cameraMatrix, glm::dmat4x4 projectionMatrix);
    std::shared_ptr<MaterialInfo> processMaterial(aiMaterial *material);
    std::shared_ptr<Mesh> processMesh(aiMesh *mesh);
    void processNode(const aiScene *scene, aiNode *node, Node *parentNode, Node &newNode);
    void findNodeAndSetDcm(std::string name, Node &node, glm::dmat4 dcm);
    bool computeGeometryBBox(std::shared_ptr<Geometry::Node> rootNode);
    void computeNodeBBox(Geometry::Node *node);
    bool createTextures();
    void getMeshTextures(std::unordered_set<std::string> *textureNames, const Node *node);
//    Texture getTexture(std::string textureName);
    glm::dmat4x4 removeTranslationAndScale(glm::dmat4x4 transform);
    glm::mat4x4 assimpMat2Glm(aiMatrix4x4 input);
    glm::dvec3 sunPosition();
    uint32_t nodeIdxCounter; // this is such a hack because I don't want to write a recursive counter to index nodes
};

#endif // GEOMETRY_H
