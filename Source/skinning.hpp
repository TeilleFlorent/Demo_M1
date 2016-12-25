#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/gtx/string_cast.hpp"
#include "glm/ext.hpp"

#include <algorithm>
using namespace std;

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <cmath>


#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define SAFE_DELETE(a) if( (a) != NULL ) delete (a); (a) = NULL;
#define SNPRINTF snprintf
#define GLCheckError() (glGetError() == GL_NO_ERROR)


#define myPI 3.141593
#define myPI_2 1.570796

#define INVALID_MATERIAL 0xFFFFFFFF

#define NUM_BONES_PER_VEREX 5 // nombre de bone influant la position d'un vertice
#define MAX_BONES 100        // maximum de bone dans un model

#define POSITION_LOCATION    0
#define TEX_COORD_LOCATION   2
#define NORMAL_LOCATION      1
#define TANGENT_LOCATION     3
#define BONE_ID_LOCATION     4
#define BONE_WEIGHT_LOCATION 8



class Texture_to_skinning {
    
    public:

        Texture_to_skinning(GLenum TextureTarget, const std::string& FileName);

        bool Load();
    
        void Bind(GLenum TextureUnit);

    private:
    	std::string m_fileName;
    	GLenum m_textureTarget;
    	GLuint m_textureObj;
    	SDL_Surface * t;

};



struct BoneInfo {

    glm::mat4 BoneOffset;
    glm::mat4 FinalTransformation;        

    BoneInfo()
    {
    	BoneOffset = glm::mat4(0.0);
    	FinalTransformation = glm::mat4(0.0);            
    }
};




// classe utilisé pour charger les animations des models animés 
class SkinnedMesh_animation{

public:

	Assimp::Importer m_Importer;

	const aiScene* m_pScene;

    SkinnedMesh_animation()
    {
        m_pScene = NULL;  
    }
    

    glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4* from);

    int max_bones(const aiMesh *mesh);

    bool LoadAnimation(const string& Filename);

};


// classe utilisé pour chargé les mesh animé
class SkinnedMesh
{
public:
   
   const aiScene* m_pScene;
   uint m_NumBones;
   vector<BoneInfo> m_BoneInfo;


    SkinnedMesh();

    ~SkinnedMesh()
    {
        Clear();
    }


    bool LoadMesh(const string& Filename);

    void Render();
    
    uint NumBones() const
    {
        return m_NumBones;
    }
    

    void Print_info_model();


    void BoneTransform(float TimeInSeconds, vector<glm::mat4>& Transforms, SkinnedMesh_animation Model);


private:

   
	glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4* from);

	int max_bones(const aiMesh *mesh);

    struct VertexBoneData
    {        
        uint IDs[NUM_BONES_PER_VEREX];
        float Weights[NUM_BONES_PER_VEREX];

        VertexBoneData()
        {
            Reset();
        };
        
        void Reset()
        {
            ZERO_MEM(IDs);
            ZERO_MEM(Weights);   
        }
    
        void AddBoneData(uint BoneID, float Weight)
        {
            for (uint i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(IDs) ; i++) {
                if (Weights[i] == 0.0) {
                    IDs[i]     = BoneID;
                    Weights[i] = Weight;
                    return;
                }        
            }
            // assert test qu'il n'y a pas trop de bones
            assert(0);
        }


    };



    void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
    
    void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
    
    void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);


    uint FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);

    uint FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);

    uint FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);

    

    const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const string NodeName);
    
    void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform, SkinnedMesh_animation Model);
    
    bool InitFromScene(const aiScene* pScene, const string& Filename);


    
    void InitMesh(uint MeshIndex, const aiMesh* paiMesh,
                    vector<glm::vec3>& Positions,
                    vector<glm::vec3>& Normals,
                    vector<glm::vec2>& TexCoords,
                    vector<glm::vec3>& Tangents,
                    vector<VertexBoneData>& Bones,
                    vector<uint>& Indices);

    
    void LoadBones(uint MeshIndex, const aiMesh* pMesh, vector<VertexBoneData>& Bones);


    bool InitMaterials(const aiScene* pScene, const string& Filename);


    void Clear();


    
    enum VB_TYPES {
    	INDEX_BUFFER,
    	POS_VB,
    	NORMAL_VB,
    	TEXCOORD_VB,
    	TANGENT_VB,
    	BONE_VB,
    	NUM_VBs            
    };

    GLuint m_VAO;
    GLuint m_Buffers[NUM_VBs];

    struct MeshEntry {
        MeshEntry()
        {
            NumIndices    = 0;
            BaseVertex    = 0;
            BaseIndex     = 0;
            MaterialIndex = INVALID_MATERIAL;
        }
        
        unsigned int NumIndices;
        unsigned int BaseVertex;
        unsigned int BaseIndex;
        unsigned int MaterialIndex;
    };
    
    vector<MeshEntry> m_Entries;
    vector<Texture_to_skinning*> m_Textures;
     
    map<string,uint> m_BoneMapping; // maps a bone name to its index
    glm::mat4 m_GlobalInverseTransform;
    int num_bones_per_vertex;
    
    Assimp::Importer m_Importer;
};



