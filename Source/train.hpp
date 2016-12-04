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


#define RUN_MODE 2



struct objet{
  float angle;
  float acca;
  float var;
  float scale;
  float x,y,z;
  float start;
  float dt;
  float bouge;
  float t;
  float t0;
  double alpha;
  float AmbientStr;
  float DiffuseStr;
  float SpecularStr;
  int ShiniStr;
  float shadow_darkness;
};
typedef struct objet objet;


struct light{
  glm::vec3 lightPos;  
  glm::vec3 save_lightPos;
  glm::vec3 lightColor;
  glm::vec3 lightSpecularColor;
};
typedef struct light light;



struct Particle {
    glm::vec2 Position, Velocity;
    glm::vec4 Color;
    GLfloat Life;
    int actual_frame;
    int sens;
    float scale;
    Particle() : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f) { }
};


GLint TextureFromFile(const char* path, string directory);


/////////////////////////////////////

// SHADER CLASS
class Shader
{
public:
    GLuint Program;
    
    Shader(){

    }

    Shader(const GLchar* vertexPath, const GLchar* fragmentPath)
    {
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        vShaderFile.exceptions (std::ifstream::badbit);
        fShaderFile.exceptions (std::ifstream::badbit);
        try
        {
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            vShaderFile.close();
            fShaderFile.close();
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (std::ifstream::failure e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        }
        const GLchar* vShaderCode = vertexCode.c_str();
        const GLchar * fShaderCode = fragmentCode.c_str();
        GLuint vertex, fragment;
        GLint success;
        GLchar infoLog[512];
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertex, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragment, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        this->Program = glCreateProgram();
        glAttachShader(this->Program, vertex);
        glAttachShader(this->Program, fragment);
        glLinkProgram(this->Program);
        glGetProgramiv(this->Program, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(this->Program, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        glDeleteShader(vertex);
        glDeleteShader(fragment);

    }
    void Use() 
    { 
        glUseProgram(this->Program); 
    }



    void set_shader(const GLchar* vertexPath, const GLchar* fragmentPath)
    {
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        vShaderFile.exceptions (std::ifstream::badbit);
        fShaderFile.exceptions (std::ifstream::badbit);
        try
        {
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            vShaderFile.close();
            fShaderFile.close();
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (std::ifstream::failure e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        }
        const GLchar* vShaderCode = vertexCode.c_str();
        const GLchar * fShaderCode = fragmentCode.c_str();
        GLuint vertex, fragment;
        GLint success;
        GLchar infoLog[512];
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertex, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragment, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        this->Program = glCreateProgram();
        glAttachShader(this->Program, vertex);
        glAttachShader(this->Program, fragment);
        glLinkProgram(this->Program);
        glGetProgramiv(this->Program, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(this->Program, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        glDeleteShader(vertex);
        glDeleteShader(fragment);

    }


    void set_shader2(const GLchar* vertexPath,const GLchar* geoPath, const GLchar* fragmentPath)
    {
        std::string vertexCode;
        std::string geoCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream gShaderFile;
        std::ifstream fShaderFile;
        vShaderFile.exceptions (std::ifstream::badbit);
        gShaderFile.exceptions (std::ifstream::badbit);
        fShaderFile.exceptions (std::ifstream::badbit);
        try
        {
            vShaderFile.open(vertexPath);
            gShaderFile.open(geoPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, gShaderStream, fShaderStream;
            vShaderStream << vShaderFile.rdbuf();
            gShaderStream << gShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            vShaderFile.close();
            gShaderFile.close();
            fShaderFile.close();
            vertexCode = vShaderStream.str();
            geoCode = gShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (std::ifstream::failure e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        }
        const GLchar* vShaderCode = vertexCode.c_str();
        const GLchar* gShaderCode = geoCode.c_str();
        const GLchar * fShaderCode = fragmentCode.c_str();
        GLuint vertex, geo, fragment;
        GLint success;
        GLchar infoLog[512];
        
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertex, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }


        geo = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geo, 1, &gShaderCode, NULL);
        glCompileShader(geo);
        glGetShaderiv(geo, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(geo, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog << std::endl;
        }

        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragment, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }

        this->Program = glCreateProgram();
        glAttachShader(this->Program, vertex);
        glAttachShader(this->Program, geo);
        glAttachShader(this->Program, fragment);
        glLinkProgram(this->Program);
        glGetProgramiv(this->Program, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(this->Program, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        glDeleteShader(vertex);
        glDeleteShader(geo);
        glDeleteShader(fragment);

    }


};

//////////////////////////////////


struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
};


struct Texture {
    GLuint id;
    string type;
    aiString path;
};


////////////////////////////////

// MESH CLASSE
class Mesh {
public:
    //  Mesh Data  
    vector<Vertex> vertices;
    vector<GLuint> indices;
    vector<Texture> textures;
    float shininess;

    Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures, float shini_mesh)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
        this->shininess = shini_mesh;

        this->setupMesh();
    }


    // render du mesh
    void Draw(Shader shader, int id) 
    {
        // var pour bind la bonne tex
        GLuint diffuseNr = 1;
        GLuint specularNr = 1;
        GLuint normalNr = 1;

        for(GLuint i = 0; i < this->textures.size(); i++)
        {
            
            //std::cout << "ite = " << i << std::endl;

            glActiveTexture(GL_TEXTURE0 + i); // active la texture qu'il faut

            stringstream ss;
            string number;
            string name = this->textures[i].type;

            //std::cout << "tex = " << name << ", i = " << i << std::endl;

            if(name == "texture_diffuse")
                ss << diffuseNr++; // bricolage pour generer le string pour l'uniform avec le bon numero de texture
            if(name == "texture_specular")
                ss << specularNr++; // bricolage pour generer le string pour l'uniform avec le bon numero de texture
            if(name == "texture_normal")
                ss << normalNr++; // bricolage pour generer le string pour l'uniform avec le bon numero de texture
            
            number = ss.str(); 

            // du coup envoi le string correct correspondant a la texture traité
            //std::cout << "uniform sampler name = " << (name + number) << std::endl;

            glUniform1i(glGetUniformLocation(shader.Program, (name + number).c_str()), i);
            glBindTexture(GL_TEXTURE_2D, this->textures[i].id); // bind la tex qui correspond au string generer et envoyer au juste avant
        }
       

        // draw le mesh
        glBindVertexArray(this->VAO);
        
        if(id == 2.0)
            glUniform1f(glGetUniformLocation(shader.Program, "ShiniSTR"), this->shininess);

        if(id == 5.0){
           glActiveTexture(GL_TEXTURE7);
           glBindTexture(GL_TEXTURE_2D, this->textures[4].id);
        }

        glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
        //glDrawArrays(GL_TRIANGLES, 0, this->indices.size());
        glBindVertexArray(0);


        // dé bind toute les tex utilisé pour le draw
        for (GLuint i = 0; i < this->textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }


private:
  
    GLuint VAO, VBO, EBO;

    // initialise tout les buffer etc
    void setupMesh()
    {
        
        glGenVertexArrays(1, &this->VAO);
        glGenBuffers(1, &this->VBO);
        glGenBuffers(1, &this->EBO);

        glBindVertexArray(this->VAO);
       
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);  

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);

        // Vertex Positions
        glEnableVertexAttribArray(0);   
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
        // Vertex Normals
        glEnableVertexAttribArray(1);   
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));
        // Vertex Texture Coords
        glEnableVertexAttribArray(2);   
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, TexCoords));
        
        // Vertex Tangent
        glEnableVertexAttribArray(3);   
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Tangent));


        glBindVertexArray(0);
    }
};

//////////////////////////////////

// MODEL CLASSE
class Model 
{
public:

    Assimp::Importer importer;
    const aiScene* scene;
    int NumVertices;

    // constructeur
    Model(){

    }

    // draw tout les meshes du model
    void Draw(Shader shader, glm::mat4 modelview2,bool test)
    {
        glm::mat4 Msend;

        for(GLuint i = 0; i < this->meshes.size(); i++){

            //std::cout << "test = " << this->meshes.size() << std::endl;
              
              Msend = glm::mat4(1.0);

              if(test){
                  if(  i == 26 || i == 6 ){

                    Msend = modelview2;

                    glUniform1f(glGetUniformLocation(shader.Program, "var"), 3.0);

                    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model2"), 1, GL_FALSE, glm::value_ptr(Msend));

                }else{

                    glUniform1f(glGetUniformLocation(shader.Program, "var"), 2.0);

                }
            }
            
            this->meshes[i].Draw(shader, this->model_id);

        }
    }
    

    //void Load_Model(GLchar* path){
    void Load_Model(string path, int id){

        this->model_id = id;
        this->loadModel(path);

    }

    void Print_info_model(){

        float res = 0;
        cout << "\nCLASSIC MODEL:\n" << "nbMeshes = " << meshes.size() << endl;
        
        for(unsigned int i = 0; i < meshes.size(); i++){
            cout << "mesh " << i << ", nbVertices = " << meshes[i].vertices.size() << endl;
            //cout << "nbIndices = " << meshes[i].indices.size() << endl;
            res+=meshes[i].vertices.size();

        }

        NumVertices = res;
        cout << "nb_vertices_total = " << res << "\n" << endl;
        

    }


    //private:
    // data du model
    vector<Mesh> meshes;
    string directory;
    vector<Texture> textures_loaded;    // variable bricolage optimisation (ne charge pas deux foix les meme texture)
    int model_id;
    
    void loadModel(string path)
    {

        /*aiString bla;   
        importer.GetExtensionList(bla);
        cout << bla.C_Str() << endl; // print tous les format supporté
*/

        //cout << "id = " << model_id << endl;
        scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

        /*if(this->model_id == 5){
            std::cout << "TEST" << std::endl;
        }*/


        if(!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            cout << "ERROR ASSIMP (dans la fonction loadModel) " << importer.GetErrorString() << endl;
            return;
        }

        this->directory = path.substr(0, path.find_last_of('/'));

        this->processNode(scene->mRootNode, scene, 0);
    }


    void processNode(aiNode* node, const aiScene* scene, int num_mesh)
    {


        for(GLuint i = 0; i < node->mNumMeshes; i++)
        {

            //cout << "num mesh = " << num_mesh << endl;

            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]]; 

            //cout << "mesh_name = " << mesh->mName.data << endl;

            if(mesh->mName != aiString((std::string("Paladin_J_Nordstrom_Sword"))) && mesh->mName != aiString((std::string("Paladin_J_Nordstrom_Shield"))))
                this->meshes.push_back(this->processMesh(mesh, scene, num_mesh));  


        }
       
        for(GLuint i = 0; i < node->mNumChildren; i++)
        {

            this->processNode(node->mChildren[i], scene, i);
        }

    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene, int num_mesh)
    {

       
        vector<Vertex> vertices;
        vector<GLuint> indices;
        vector<Texture> textures;



        for(GLuint i = 0; i < mesh->mNumVertices; i++)
        {

            Vertex vertex;
            glm::vec3 vector; 
            // Position
            
            /*if(num_mesh == 26 || num_mesh == 6){
                vector.x = mesh->mVertices[i].x;
            }else{*/
                vector.x = mesh->mVertices[i].x;
            //}
            /*if(num_mesh == 26 || num_mesh == 6){
                vector.y = mesh->mVertices[i].y+31.0;
            }else{*/
                vector.y = mesh->mVertices[i].y;
            //}
        
            /*if(num_mesh == 26 || num_mesh == 6){
                vector.z = mesh->mVertices[i].z;
            }else{*/
                vector.z = mesh->mVertices[i].z;
            //}
        
            //std::cout << "x = " << vector.x << ", y = " << vector.y << std::endl;
                
            vertex.Position = vector;
            // Normal
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            //std::cout << "x = " << vector.x << ", y = " << vector.y << ", z = " << vector.z << std::endl;
                
            vertex.Normal = vector;
            // Texture Coord
            if(mesh->mTextureCoords[0]) // verifie si il y a des tex coord
            {
                //printf("IL Y A DES TEX COORD\n");

                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x; 
                vec.y = mesh->mTextureCoords[0][i].y;
                //std::cout << "x = " << vec.x << ", y = " << vec.y << std::endl;
                vertex.TexCoords = vec;
            }
            else{
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
                printf("PAS DE TEX COORD\n");
            }

            // ADD TANGENT
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector; 

            //std::cout << "test = " << vertex.Tangent.x << vertex.Tangent.y << vertex.Tangent.z << std::endl; 

            vertices.push_back(vertex);
        }

        for(GLuint i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for(GLuint j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }


        //std::cout << "test1 = " << scene->HasTextures() << std::endl;

        // concerne les material texture
        if(/*mesh->mMaterialIndex >= 0*/ true)
        {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            
            //std::cout << "test1 = " << scene->HasTextures() << std::endl;


            // Diffuse maps
            vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", num_mesh);
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            if(model_id != 2 && model_id != 3 && model_id != 5){
            // Specular maps
                vector<Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", num_mesh);
                textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
           // Specular maps
                vector<Texture> normalMaps = this->loadMaterialTextures(material, aiTextureType_NORMALS , "texture_normal", num_mesh);
                textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
            }
        }
        
        float shini_mesh = 2.0;
        if(num_mesh == 87 || num_mesh == 89 || num_mesh == 90 || num_mesh == 112 || num_mesh == 129 || num_mesh == 130 || num_mesh == 131 || (num_mesh >=132 && num_mesh <= 144) 
            || num_mesh == 154 || num_mesh == 160 || num_mesh == 162 || num_mesh == 163 || num_mesh == 164 || num_mesh == 167 || (num_mesh >= 168 && num_mesh <= 184)){
            shini_mesh = 1.0;
        }
        if(num_mesh == 207){
            shini_mesh = 8.0;
        }
        

        //std::cout << "test = " << num_mesh << std::endl;
        return Mesh(vertices, indices, textures, shini_mesh);
    }

    
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName, int num_mesh)
    {


        vector<Texture> textures;


        // FOR HOUSE MESH ONLY
        if(this -> model_id == 2){

            GLboolean skip = false;

            string temp1,temp2,temp3;
            if(num_mesh == 87 || num_mesh == 89 || num_mesh == 90 || num_mesh == 112 || num_mesh == 129 || num_mesh == 130 || num_mesh == 131 || (num_mesh >=132 && num_mesh <= 144) 
                || num_mesh == 154 || num_mesh == 160 || num_mesh == 162 || num_mesh == 163 || num_mesh == 164 || num_mesh == 167 || (num_mesh >= 168 && num_mesh <= 184)){
                temp1 = "walls/wood_roof.jpg";
                temp2 = "walls/wall_normal.jpg";
            }else{
                if(num_mesh == 207){
                    temp1 = "floor/concrete_base_color.png";
                    temp2 = "floor/concrete_normal.png";
                    temp3 = "floor/concrete_AO.png";
                }else{
                    temp1 = "walls/wood_wall.jpg";
                    temp2 = "walls/wall_normal.jpg";
                }
            }
    
            aiString str;
            str.Set(temp1);
            

            for(GLuint j = 0; j < textures_loaded.size(); j++)
            {
                if(textures_loaded[j].path == str)
                {
                    string temp = str.data;
                    textures.push_back(textures_loaded[j]);
                    if(temp.find("jpg") != std::string::npos){
                        textures.push_back(textures_loaded[j+1]);
                    }

                    skip = true; 
                    break;
                }
                
            }
            if(!skip){
             // base
               Texture texture;
               texture.id = TextureFromFile(str.C_Str() , this->directory);
               texture.path = str;
               temp1 = "texture_diffuse";
               texture.type = temp1;
               textures.push_back(texture);
               this->textures_loaded.push_back(texture);  
            
             // normal
               if(!temp2.empty()){
                 str.Set(temp2);
                 texture.id = TextureFromFile(str.C_Str() , this->directory);
                 texture.path = str;
                 temp2 = "texture_normal";
                 texture.type = temp2;
                 textures.push_back(texture);
                 this->textures_loaded.push_back(texture);  
               }

              // AO
               if(!temp3.empty()){
                 str.Set(temp3);
                 texture.id = TextureFromFile(str.C_Str() , this->directory);
                 texture.path = str;
                 temp3 = "texture_specular";
                 texture.type = temp3;
                 textures.push_back(texture);
                 this->textures_loaded.push_back(texture);  
               }

           }

       }

       // FOR SWORD ONLY
        if(this -> model_id == 3){

            GLboolean skip = false;

            string temp1;
            
            aiString str;
            

            if(!skip){
             // base
               temp1 = "Longsword_LP_DefaultMaterial_BaseColor.png";            
               str.Set(temp1);

               Texture texture;
               texture.id = TextureFromFile(str.C_Str() , this->directory);
               texture.path = str;
               temp1 = "texture_diffuse";
               texture.type = temp1;
               textures.push_back(texture);
               this->textures_loaded.push_back(texture);  
            
             // normal
                 temp1 = "Longsword_LP_DefaultMaterial_Normal.png";            
                 str.Set(temp1);
            
                 texture.id = TextureFromFile(str.C_Str() , this->directory);
                 texture.path = str;
                 temp1 = "texture_normal";
                 texture.type = temp1;
                 textures.push_back(texture);
                 this->textures_loaded.push_back(texture);  
               
              // metallness map
                 temp1 = "Longsword_LP_DefaultMaterial_RoughnessMetallic.png";            
                 str.Set(temp1);

                 texture.id = TextureFromFile(str.C_Str() , this->directory);
                 texture.path = str;
                 temp1 = "texture_specular";
                 texture.type = temp1;
                 textures.push_back(texture);
                 this->textures_loaded.push_back(texture);  
               
           }

       }

       // FOR TREE ONLY
       if(this->directory.find("tree2") != std::string::npos){

            GLboolean skip = false;

            string temp1;
            
            aiString str;

            mat->GetTexture(type, 0, &str);
            temp1 = str.data;

            if(temp1.find("branches") != std::string::npos){
                temp1 = "Tree_Dry_1_branches_Diffuse.jpg";
            }else{
                temp1 = "Tree_Dry_1_Diffuse2048.jpg";
            }

            str.Set(temp1);

            for(GLuint j = 0; j < textures_loaded.size(); j++)
            {
                if(textures_loaded[j].path == str && model_id == 6.0)
                {
                    string temp = str.data;
                    textures.push_back(textures_loaded[j]);
                    //textures.push_back(textures_loaded[j+1]);

                    skip = true; 

                    return textures;
                }
            }

            if(!skip){

               Texture texture;
               texture.id = TextureFromFile(str.C_Str() , this->directory);
               texture.path = str;
               temp1 = "texture_diffuse";
               texture.type = temp1;
               textures.push_back(texture);
               this->textures_loaded.push_back(texture);  


               return textures;

           }


        
       }

       // FOR SHIELD ONLY
        if(this -> model_id == 5){

            GLboolean skip = false;

            string temp1;
            
            aiString str;

             // base
            temp1 = "Shield-Texture.jpg";            
            str.Set(temp1);

            for(GLuint j = 0; j < textures_loaded.size(); j++)
            {
                if(textures_loaded[j].path == str && model_id == 5.0)
                {
                    string temp = str.data;
                    textures.push_back(textures_loaded[j]);
                    textures.push_back(textures_loaded[j+1]);
                    textures.push_back(textures_loaded[j+2]);
                    textures.push_back(textures_loaded[j+3]);
                    textures.push_back(textures_loaded[j+4]);


                    skip = true; 

                    return textures;
                }
            }

            if(!skip){

               Texture texture;
               texture.id = TextureFromFile(str.C_Str() , this->directory);
               texture.path = str;
               temp1 = "texture_diffuse";
               texture.type = temp1;
               textures.push_back(texture);
               this->textures_loaded.push_back(texture);  
            
             // normal
               temp1 = "Shield-NM.jpg";            
               str.Set(temp1);
               texture.id = TextureFromFile(str.C_Str() , this->directory);
               texture.path = str;
               temp1 = "texture_normal";
               texture.type = temp1;
               textures.push_back(texture);
               this->textures_loaded.push_back(texture);  
               
              // AO map
               temp1 = "Shield-AO.jpg";            
               str.Set(temp1);

               texture.id = TextureFromFile(str.C_Str() , this->directory);
               texture.path = str;
               temp1 = "texture_specular";
               texture.type = temp1;
               textures.push_back(texture);
               this->textures_loaded.push_back(texture);  

                // specular map
               temp1 = "Shield-Spec.jpg";            
               str.Set(temp1);

               texture.id = TextureFromFile(str.C_Str() , this->directory);
               texture.path = str;
               temp1 = "texture_specular";
               texture.type = temp1;
               textures.push_back(texture);
               this->textures_loaded.push_back(texture);  
               
               // metalness map
               temp1 = "Shield-Gloss.jpg";            
               str.Set(temp1);

               texture.id = TextureFromFile(str.C_Str() , this->directory);
               texture.path = str;
               temp1 = "texture_metalnes";
               texture.type = temp1;
               textures.push_back(texture);
               this->textures_loaded.push_back(texture);  
               

               return textures;
           }

       }




    
        for(GLuint i = 0; i < mat->GetTextureCount(type); i++)
        {

            aiString str;
            mat->GetTexture(type, i, &str);
            
            string test_path = str.data;
            
            // MODIF PATH TEXTURE 
            if (test_path.find("Paladin_diffuse") != std::string::npos) {
                test_path = "Paladin_diffuse.png";
                str.Set(test_path);
            }

            if (test_path.find("Paladin_specular") != std::string::npos) {
                test_path = "Paladin_specular.png";
                str.Set(test_path);
            }

            if (test_path.find("Paladin_normal") != std::string::npos) {
                test_path = "Paladin_normal.png";
                str.Set(test_path);
            }
            
            //std::cout << "str = " << str.data << std::endl;


            GLboolean skip = false;
            for(GLuint j = 0; j < textures_loaded.size(); j++)
            {
                if(textures_loaded[j].path == str)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; 
                    break;
                }
            }
            if(!skip)
            {   
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str;
                textures.push_back(texture);
                this->textures_loaded.push_back(texture);  
            }


            // FOR FEU MESH ONLY
            if(this->directory.find("Models/feu/") != std::string::npos){

                Texture texture;
                string temp = "ohniste4UVcompletnormal1.png";
                str.Set(temp);
                texture.id = TextureFromFile(str.C_Str() , this->directory);
                texture.path = str;
                temp = "texture_normal";
                texture.type = temp;
                textures.push_back(texture);
                this->textures_loaded.push_back(texture);  
                
            }

        }

      /*  if(this->model_id == 2){
            std::cout << "nb tex = " << textures.size() << std::endl;
        }*/
        return textures;
    }
};




//////////////////////////////////

static SDL_Window * initWindow(int w, int h, SDL_GLContext * poglContext);
static void quit(void);
static void initGL(SDL_Window * win);
static void initData(void);
static void resizeGL(SDL_Window * win);
static void loop(SDL_Window * win);
static void manageEvents(SDL_Window * win);
static void draw(void);
static void printFPS(void);
static void mobile_move(objet*,int);
GLuint loadCubemap(vector<const GLchar*>);
static GLfloat * buildSphere(int, int);
void Pre_rendu_feu(glm::mat4, glm::mat4,float);
void RenderShadowedObjects(bool);
GLint TextureFromFile(const char* path, string directory);
void SetBoneTransform(uint , const glm::mat4&, int);
float MyBarryCentric(glm::vec3 , glm::vec3 , glm::vec3 , glm::vec2);
glm::mat4 door_script();
glm::mat4 zombie_script();
static void marche_script();
glm::vec3 walk_script();
void camera_script();
void audio_script();
void fire_script();
void fall_script();
void script();
void Pre_rendu_cubeMap();
void Pre_rendu_shadow_house(glm::mat4, glm::mat4);

  
/////////////////////////////////
  

float rand_FloatRange(float a, float b)
{
    return ((b-a)*((float)rand()/RAND_MAX))+a;
}



void print_mat4(glm::mat4 matrix){

//double dArray[16] = {0.0};
    const float *pSource = (const float*)glm::value_ptr(matrix);
    
    printf("\n");

    for (int i = 0; i < 4;i++){
        for (int j = 0; j < 4; j++){
            printf("%.3f ", pSource[(4 * j) + i]);
        }
        printf("\n");
    }
    //dArray[i] = pSource[i];

    printf("\n");
}


// fonction utilisé pendant le chargement de model pour recuperer les texture
GLint TextureFromFile(const char* path, string directory)
{

    float aniso;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso); // get la valeur pour l'aniso

    SDL_Surface * t = NULL;

    string filename = string(path);
    filename = directory + '/' + filename;
    GLuint textureID;
    glGenTextures(1, &textureID);
    t = IMG_Load(filename.c_str());


    std::cout << "test1 = " << path << std::endl;
    std::cout << "test2 = " << directory << std::endl;     
    std::cout << "test3 = " << filename << std::endl;     
    

/*    if(!t)
        printf("image null\n");*/

    glBindTexture(GL_TEXTURE_2D, textureID);
    
    if(t->format->format == SDL_PIXELFORMAT_RGB332
        || t->format->format == SDL_PIXELFORMAT_RGB444
        || t->format->format == SDL_PIXELFORMAT_RGB555
        || t->format->format == SDL_PIXELFORMAT_RGB565
        || t->format->format == SDL_PIXELFORMAT_RGB24
        || t->format->format == SDL_PIXELFORMAT_RGB888
        //|| t->format->format == SDL_PIXELFORMAT_RGBX8888
        || t->format->format == SDL_PIXELFORMAT_RGB565
        || t->format->format == SDL_PIXELFORMAT_BGR555
        || t->format->format == SDL_PIXELFORMAT_BGR565
        || t->format->format == SDL_PIXELFORMAT_BGR24
        || t->format->format == SDL_PIXELFORMAT_BGR888){
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels);
    }else{
    
    if(t->format->format == SDL_PIXELFORMAT_RGBA4444
        || t->format->format == SDL_PIXELFORMAT_RGBA5551
        || t->format->format == SDL_PIXELFORMAT_ARGB4444
        || t->format->format == SDL_PIXELFORMAT_ABGR4444
        || t->format->format == SDL_PIXELFORMAT_BGRA4444
        || t->format->format == SDL_PIXELFORMAT_ABGR1555
        || t->format->format == SDL_PIXELFORMAT_BGRA5551
        || t->format->format == SDL_PIXELFORMAT_ARGB8888
        || t->format->format == SDL_PIXELFORMAT_ABGR8888
        || t->format->format == SDL_PIXELFORMAT_BGRA8888
        //|| t->format->format == SDL_PIXELFORMAT_RGBX8888
        || t->format->format == SDL_PIXELFORMAT_RGBA8888){
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->pixels);
    }else{ glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels); }
}
    
    
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t->w, t->h, 0, GL_RGB,GL_UNSIGNED_BYTE, t->pixels);
    
    glGenerateMipmap(GL_TEXTURE_2D);    

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso); // anisotropie

    glBindTexture(GL_TEXTURE_2D, 0);
    SDL_FreeSurface(t);
    return textureID;
    }

//////////////////////////////////////// 


// conversion matrice assimp -> glm
glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4* from)
{
    glm::mat4 to;

    to[0][0] = (GLfloat)from->a1; to[0][1] = (GLfloat)from->b1;  to[0][2] = (GLfloat)from->c1; to[0][3] = (GLfloat)from->d1;
    to[1][0] = (GLfloat)from->a2; to[1][1] = (GLfloat)from->b2;  to[1][2] = (GLfloat)from->c2; to[1][3] = (GLfloat)from->d2;
    to[2][0] = (GLfloat)from->a3; to[2][1] = (GLfloat)from->b3;  to[2][2] = (GLfloat)from->c3; to[2][3] = (GLfloat)from->d3;
    to[3][0] = (GLfloat)from->a4; to[3][1] = (GLfloat)from->b4;  to[3][2] = (GLfloat)from->c4; to[3][3] = (GLfloat)from->d4;

    return to;
}

class Texture_to_skinning
    {
    public:
    //Texture_to_skinning(GLenum TextureTarget, const std::string& FileName);
        Texture_to_skinning(GLenum TextureTarget, const std::string& FileName)
        {
            m_textureTarget = TextureTarget;
            m_fileName      = FileName;
        }


        bool Load()
        {

            t = NULL;
            t = IMG_Load(m_fileName.c_str());


/*            printf("try image load = %s\n",m_fileName.c_str());

            if(!t)
                printf("image null\n");*/


            glGenTextures(1, &m_textureObj);
            glBindTexture(m_textureTarget, m_textureObj);


            if(t->format->format == SDL_PIXELFORMAT_RGB332
                || t->format->format == SDL_PIXELFORMAT_RGB444
                || t->format->format == SDL_PIXELFORMAT_RGB555
                || t->format->format == SDL_PIXELFORMAT_RGB565
                || t->format->format == SDL_PIXELFORMAT_RGB24
                || t->format->format == SDL_PIXELFORMAT_RGB888
              //|| t->format->format == SDL_PIXELFORMAT_RGBX8888
                || t->format->format == SDL_PIXELFORMAT_RGB565
                || t->format->format == SDL_PIXELFORMAT_BGR555
                || t->format->format == SDL_PIXELFORMAT_BGR565
                || t->format->format == SDL_PIXELFORMAT_BGR24
                || t->format->format == SDL_PIXELFORMAT_BGR888){
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels);
        }else{

            if(t->format->format == SDL_PIXELFORMAT_RGBA4444
                || t->format->format == SDL_PIXELFORMAT_RGBA5551
                || t->format->format == SDL_PIXELFORMAT_ARGB4444
                || t->format->format == SDL_PIXELFORMAT_ABGR4444
                || t->format->format == SDL_PIXELFORMAT_BGRA4444
                || t->format->format == SDL_PIXELFORMAT_ABGR1555
                || t->format->format == SDL_PIXELFORMAT_BGRA5551
                || t->format->format == SDL_PIXELFORMAT_ARGB8888
                || t->format->format == SDL_PIXELFORMAT_ABGR8888
                || t->format->format == SDL_PIXELFORMAT_BGRA8888
              //|| t->format->format == SDL_PIXELFORMAT_RGBX8888
                || t->format->format == SDL_PIXELFORMAT_RGBA8888){

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->pixels);
               }else{ glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels); }
           }


        //glTexImage2D(m_textureTarget, 0, GL_RGBA, m_image.columns(), m_image.rows(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_blob.data());  
        //glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        //glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    

           glGenerateMipmap(m_textureTarget);    
           glTexParameteri( m_textureTarget, GL_TEXTURE_WRAP_S, GL_REPEAT );
           glTexParameteri( m_textureTarget, GL_TEXTURE_WRAP_T, GL_REPEAT );
           glTexParameteri( m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
           glTexParameteri( m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
           glBindTexture(m_textureTarget, 0);

           return true;
   }
    
    
    void Bind(GLenum TextureUnit)
    {
        glActiveTexture(TextureUnit);
        glBindTexture(GL_TEXTURE_2D, m_textureObj);
    }
    
    private:
    std::string m_fileName;
    GLenum m_textureTarget;
    GLuint m_textureObj;
    /*Magick::Image m_image;
    Magick::Blob m_blob;*/
    SDL_Surface * t;
};

// fonction qui renvoi le nombre de bones max assigné par vertice d'un mesh animé
int max_bones(const aiMesh *mesh)
{
    int *counts = (int*)calloc(mesh->mNumVertices, sizeof(int));
    for (uint i = 0; i < mesh->mNumBones; i++) {
        const aiBone *bone = mesh->mBones[i];
        for (uint j = 0; j < bone->mNumWeights; j++) {
            const aiVertexWeight *weight = &bone->mWeights[j];
            counts[weight->mVertexId]++;
        }
    }
    int max = 0;
    for (uint i = 0; i < mesh->mNumVertices; i++) {
        if (max < counts[i])
            max = counts[i];
    }
    return max;
}


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
class SkinnedMesh_animation
{
public:

   const aiScene* m_pScene;
   uint m_NumBones;
   vector<BoneInfo> m_BoneInfo;



    SkinnedMesh_animation()
    {
        m_VAO = 0;

        ZERO_MEM(m_Buffers);
     
        m_NumBones = 0;
       
        m_pScene = NULL;
       
    }
    
    ~SkinnedMesh_animation()
    {
        Clear();
    }


    bool LoadMesh(const string& Filename)
    {
    // Release the previously loaded mesh (if it exists)
        Clear();

    // Create the VAO
        glGenVertexArrays(1, &m_VAO);   
        glBindVertexArray(m_VAO);

    // Create the buffers for the vertices attributes
    
        //std::cout << "test val = " << ARRAY_SIZE_IN_ELEMENTS(m_Buffers) << std::endl;
        glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);

        bool Ret = false;    

        m_pScene = m_Importer.ReadFile(Filename.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);


     /*   aiString bla;   
        m_Importer.GetExtensionList (bla);
        cout << bla.C_Str() << endl;
*/

        if (m_pScene){  

            /*if(m_pScene->mAnimations[0])
                printf("scene chargée\n");*/

            //m_GlobalInverseTransform = m_pScene->mRootNode->mTransformation;
                m_GlobalInverseTransform = aiMatrix4x4ToGlm(&m_pScene->mRootNode->mTransformation);
            //std::cout<<glm::to_string(m_GlobalInverseTransform)<<std::endl;
                m_GlobalInverseTransform = glm::inverse(m_GlobalInverseTransform);
            //std::cout<<glm::to_string(m_GlobalInverseTransform)<<std::endl;
                Ret = InitFromScene(m_pScene, Filename);

        }
        else {
            printf("Error parsing '%s': '%s'\n", Filename.c_str(), m_Importer.GetErrorString());
        }

    // Make sure the VAO is not changed from the outside
        glBindVertexArray(0);   

        return Ret;
    }



    void Render()
    {
        glBindVertexArray(m_VAO);

        for (uint i = 0 ; i < m_Entries.size() ; i++) {
        //    const uint MaterialIndex = m_Entries[i].MaterialIndex;

          //  assert(MaterialIndex < m_Textures.size());

            //if (m_Textures[MaterialIndex]) {
                m_Textures[0]->Bind(GL_TEXTURE0);
                m_Textures[1]->Bind(GL_TEXTURE2);
            
            //}


                //glActiveTexture(GL_TEXTURE0);
                //glBindTexture(GL_TEXTURE_2D, m_Textures[0]);
                //glActiveTexture(GL_TEXTURE1);
                //glBindTexture(GL_TEXTURE_2D, tex_depthMap);


            glDrawElementsBaseVertex(GL_TRIANGLES, m_Entries[i].NumIndices, GL_UNSIGNED_INT, (void*)(sizeof(uint) * m_Entries[i].BaseIndex), m_Entries[i].BaseVertex);
        }

    // Make sure the VAO is not changed from the outside    
        glBindVertexArray(0);
    }


    
    uint NumBones() const
    {
        return m_NumBones;
    }
    

    void Print_info_model(){


        std::cout << "\nSKINNED MODEL:\nnb mesh(s) = " << m_Entries.size() << std::endl;
        
        for(uint i = 0; i < m_Entries.size(); i++){
            std::cout << "mesh " << i << " = " << m_Entries[i].NumIndices << " vertice(s)" << std::endl;
        }

        std::cout << "nb bone(s) = " << m_NumBones << std::endl;
        
        std::cout << "nb tex(s) = " << m_Textures.size() << std::endl;
        
        std::cout << "num max bones = " << num_bones_per_vertex << std::endl;        


        printf("\n");

    }
    

    void BoneTransform(float TimeInSeconds, vector<glm::mat4>& Transforms/*,const aiScene* Model_scene, uint Model_num_bones, vector<BoneInfo> Model_bone_info*/)
    {
        glm::mat4 Identity;
        Identity = glm::mat4(1.0);
        //Identity.InitIdentity();

        float TicksPerSecond = (float)(m_pScene/*Model_scene*/->mAnimations[0]->mTicksPerSecond != 0 ? m_pScene/*Model_scene*/->mAnimations[0]->mTicksPerSecond : 25.0f);
        float TimeInTicks = TimeInSeconds * TicksPerSecond;
        float AnimationTime = fmod(TimeInTicks, (float)m_pScene/*Model_scene*/->mAnimations[0]->mDuration);

        ReadNodeHeirarchy(AnimationTime, m_pScene/*Model_scene*/->mRootNode, Identity/*, Model_scene*/);

        Transforms.resize(m_NumBones/*Model_num_bones*/);


        for (uint i = 0 ; i < m_NumBones/*Model_num_bones*/ ; i++) {
            Transforms[i] = /*Model_bone_info[i].FinalTransformation*/ m_BoneInfo[i].FinalTransformation /*glm::mat4(1.0)*/;
        }
    }



//private:

    
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



    void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
    {
        if (pNodeAnim->mNumScalingKeys == 1) {
            Out = pNodeAnim->mScalingKeys[0].mValue;
            return;
        }

        uint ScalingIndex = FindScaling(AnimationTime, pNodeAnim);
        uint NextScalingIndex = (ScalingIndex + 1);
        assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
        float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
        float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
        assert(Factor >= 0.0f && Factor <= 1.0f);
        const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
        const aiVector3D& End   = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
        aiVector3D Delta = End - Start;
        Out = Start + Factor * Delta;
    }

    

    void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
    {
    // we need at least two values to interpolate...
        if (pNodeAnim->mNumRotationKeys == 1) {
            Out = pNodeAnim->mRotationKeys[0].mValue;
            return;
        }
    
        uint RotationIndex = FindRotation(AnimationTime, pNodeAnim);
        uint NextRotationIndex = (RotationIndex + 1);
        assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
        float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
        float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
        assert(Factor >= 0.0f && Factor <= 1.0f);
        const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
        const aiQuaternion& EndRotationQ   = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;    
        aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
        Out = Out.Normalize();
    }
    

    
    void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
    {
        if (pNodeAnim->mNumPositionKeys == 1) {
            Out = pNodeAnim->mPositionKeys[0].mValue;
            return;
        }

        uint PositionIndex = FindPosition(AnimationTime, pNodeAnim);
        uint NextPositionIndex = (PositionIndex + 1);
        assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
        float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
        float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
        assert(Factor >= 0.0f && Factor <= 1.0f);
        const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
        const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
        aiVector3D Delta = End - Start;
        Out = Start + Factor * Delta;
    }


    uint FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
    {
        assert(pNodeAnim->mNumScalingKeys > 0);

        for (uint i = 0 ; i < pNodeAnim->mNumScalingKeys - 1 ; i++) {
            if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
                return i;
            }
        }

        assert(0);

        return 0;
    }


    uint FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
    {
        assert(pNodeAnim->mNumRotationKeys > 0);

        for (uint i = 0 ; i < pNodeAnim->mNumRotationKeys - 1 ; i++) {
            if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
                return i;
            }
        }

        assert(0);

        return 0;
    }

    uint FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
    {    
        for (uint i = 0 ; i < pNodeAnim->mNumPositionKeys - 1 ; i++) {
            if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
                return i;
            }
        }

        assert(0);

        return 0;
    }

    

    const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const string NodeName)
    {
        for (uint i = 0 ; i < pAnimation->mNumChannels ; i++) {
            const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

            if (string(pNodeAnim->mNodeName.data) == NodeName) {
            return pNodeAnim;
        }
    }
    
    return NULL;
    }


    
    void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform/*, const aiScene* Model_scene*/)
    {    
        string NodeName(pNode->mName.data);

        const aiAnimation* pAnimation = m_pScene/*Model_scene*/->mAnimations[0];
        
        glm::mat4 NodeTransformation = aiMatrix4x4ToGlm(&pNode->mTransformation);


        const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnimation, NodeName);

        if (pNodeAnim) {
        // Interpolate scaling and generate scaling transformation matrix
            aiVector3D Scaling;
            CalcInterpolatedScaling(Scaling, AnimationTime, pNodeAnim);
            glm::mat4 ScalingM = glm::mat4(1.0);
            ScalingM = glm::scale(ScalingM, glm::vec3(Scaling.x, Scaling.y, Scaling.z));
            //ScalingM.InitScaleTransform(Scaling.x, Scaling.y, Scaling.z);

        // Interpolate rotation and generate rotation transformation matrix
            aiQuaternion RotationQ;
            CalcInterpolatedRotation(RotationQ, AnimationTime, pNodeAnim);        
            aiMatrix4x4 temp = aiMatrix4x4(RotationQ.GetMatrix());
            glm::mat4 RotationM = aiMatrix4x4ToGlm(&temp);

        // Interpolate translation and generate translation transformation matrix
            aiVector3D Translation;
            CalcInterpolatedPosition(Translation, AnimationTime, pNodeAnim);
            glm::mat4 TranslationM = glm::mat4(1.0);
            TranslationM = glm::translate(TranslationM, glm::vec3(Translation.x, Translation.y, Translation.z));
            //TranslationM.InitTranslationTransform(Translation.x, Translation.y, Translation.z);

        // Combine the above transformations
            NodeTransformation = TranslationM * RotationM * ScalingM;
        }

        glm::mat4 GlobalTransformation = ParentTransform * NodeTransformation;

        if (m_BoneMapping.find(NodeName) != m_BoneMapping.end()) {
            uint BoneIndex = m_BoneMapping[NodeName];
            m_BoneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation * m_BoneInfo[BoneIndex].BoneOffset;
        }

        for (uint i = 0 ; i < pNode->mNumChildren ; i++) {
            ReadNodeHeirarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation/*, Model_scene*/);
        }
    }

    
    
bool InitFromScene(const aiScene* pScene, const string& Filename)
{  
    m_Entries.resize(pScene->mNumMeshes);
    m_Textures.resize((pScene->mNumMaterials)+1);

    num_bones_per_vertex = 0;

    //std::cout << "\nNumMaterials = " << pScene->mNumMaterials << std::endl;        


    vector<glm::vec3> Positions;
    vector<glm::vec3> Normals;
    vector<glm::vec2> TexCoords;
    vector<VertexBoneData> Bones;
    vector<uint> Indices;
       
    uint NumVertices = 0;
    uint NumIndices = 0;
    
    // Count the number of vertices and indices
    for (uint i = 0 ; i < m_Entries.size() ; i++) {
        m_Entries[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;        
        m_Entries[i].NumIndices    = pScene->mMeshes[i]->mNumFaces * 3;
        m_Entries[i].BaseVertex    = NumVertices;
        m_Entries[i].BaseIndex     = NumIndices;
        
        NumVertices += pScene->mMeshes[i]->mNumVertices;
        NumIndices  += m_Entries[i].NumIndices;
    }
    
    // Reserve space in the vectors for the vertex attributes and indices
    Positions.reserve(NumVertices);
    Normals.reserve(NumVertices);
    TexCoords.reserve(NumVertices);
    Bones.resize(NumVertices);
    Indices.reserve(NumIndices);
        
    // Initialize the meshes in the scene one by one
    for (uint i = 0 ; i < m_Entries.size() ; i++) {

        //std::cout << "\nmax bones = " << max_bones(pScene->mMeshes[i]) << std::endl;        

        const aiMesh* paiMesh = pScene->mMeshes[i];
        InitMesh(i, paiMesh, Positions, Normals, TexCoords, Bones, Indices);
    }


    /*if (!InitMaterials(pScene, Filename)) {
        return false;
    }*/
 

    // Generate and populate the buffers with vertex attributes and the indices
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[POS_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Positions[0]) * Positions.size(), &Positions[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(POSITION_LOCATION);
    glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);    

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[TEXCOORD_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoords[0]) * TexCoords.size(), &TexCoords[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(TEX_COORD_LOCATION);
    glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[NORMAL_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Normals[0]) * Normals.size(), &Normals[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(NORMAL_LOCATION);
    glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[BONE_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Bones[0]) * Bones.size(), &Bones[0], GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(BONE_ID_LOCATION);
    glVertexAttribIPointer(BONE_ID_LOCATION, 4, GL_INT, sizeof(VertexBoneData), (const GLvoid*)0);
    glEnableVertexAttribArray(BONE_ID_LOCATION+1);
    glVertexAttribIPointer(BONE_ID_LOCATION+1, 2, GL_INT, sizeof(VertexBoneData), (const void *)(4*(sizeof(int))));
   

    glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);    
    glVertexAttribPointer(BONE_WEIGHT_LOCATION, /*NUM_BONES_PER_VEREX*/4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData),
     (const void *)(6*(sizeof(int))) /*(const GLvoid*)16*/ /*(const GLvoid*)(NUM_BONES_PER_VEREX * NUM_BONES_PER_VEREX)*/);
    
    glEnableVertexAttribArray(BONE_WEIGHT_LOCATION+1);    
    glVertexAttribPointer(BONE_WEIGHT_LOCATION+1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData),
     (const void *)((6*(sizeof(int)))+(4*(sizeof(float)))));
    

/*
    glVertexAttribDivisor(BONE_ID_LOCATION, 1);
    glVertexAttribDivisor(BONE_ID_LOCATION+1, 1);
    glVertexAttribDivisor(BONE_ID_LOCATION+2, 1);
    glVertexAttribDivisor(BONE_ID_LOCATION+3, 1);
    
    glVertexAttribDivisor(BONE_WEIGHT_LOCATION, 1);
    glVertexAttribDivisor(BONE_WEIGHT_LOCATION+1, 1);
    glVertexAttribDivisor(BONE_WEIGHT_LOCATION+2, 1);
    glVertexAttribDivisor(BONE_WEIGHT_LOCATION+3, 1);*/



    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices[0]) * Indices.size(), &Indices[0], GL_STATIC_DRAW);

    return GLCheckError();
}


    
void InitMesh(uint MeshIndex, const aiMesh* paiMesh,
                    vector<glm::vec3>& Positions,
                    vector<glm::vec3>& Normals,
                    vector<glm::vec2>& TexCoords,
                    vector<VertexBoneData>& Bones,
                    vector<uint>& Indices) {
    
    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
    
    // Populate the vertex attribute vectors
    for (uint i = 0 ; i < paiMesh->mNumVertices ; i++) {
        const aiVector3D* pPos      = &(paiMesh->mVertices[i]);
        const aiVector3D* pNormal   = &(paiMesh->mNormals[i]);
        const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;

        Positions.push_back(glm::vec3(pPos->x, pPos->y, pPos->z));
        Normals.push_back(glm::vec3(pNormal->x, pNormal->y, pNormal->z));
        TexCoords.push_back(glm::vec2(pTexCoord->x, pTexCoord->y));        
    }
    
    LoadBones(MeshIndex, paiMesh, Bones);
    
    // Populate the index buffer
    for (uint i = 0 ; i < paiMesh->mNumFaces ; i++) {
        const aiFace& Face = paiMesh->mFaces[i];
        assert(Face.mNumIndices == 3);
        Indices.push_back(Face.mIndices[0]);
        Indices.push_back(Face.mIndices[1]);
        Indices.push_back(Face.mIndices[2]);
    }
}


    
void LoadBones(uint MeshIndex, const aiMesh* pMesh, vector<VertexBoneData>& Bones)
{
    for (uint i = 0 ; i < pMesh->mNumBones ; i++) {                
        uint BoneIndex = 0;        
        string BoneName(pMesh->mBones[i]->mName.data);
        
        if (m_BoneMapping.find(BoneName) == m_BoneMapping.end()) {
            // Allocate an index for a new bone
            BoneIndex = m_NumBones;
            m_NumBones++;            
            BoneInfo bi;            
            m_BoneInfo.push_back(bi);
            m_BoneInfo[BoneIndex].BoneOffset = aiMatrix4x4ToGlm(&pMesh->mBones[i]->mOffsetMatrix);            
            m_BoneMapping[BoneName] = BoneIndex;
        }
        else {
            BoneIndex = m_BoneMapping[BoneName];
        }                      
        
        for (uint j = 0 ; j < pMesh->mBones[i]->mNumWeights ; j++) {
            uint VertexID = m_Entries[MeshIndex].BaseVertex + pMesh->mBones[i]->mWeights[j].mVertexId;
            float Weight  = pMesh->mBones[i]->mWeights[j].mWeight;                   
            Bones[VertexID].AddBoneData(BoneIndex, Weight);
        }
    }    
}



bool InitMaterials(const aiScene* pScene, const string& Filename)
{
    // Extract the directory part from the file name
    string::size_type SlashIndex = Filename.find_last_of("/");
    string Dir;

    if (SlashIndex == string::npos) {
        Dir = ".";
    }
    else if (SlashIndex == 0) {
        Dir = "/";
    }
    else {
        Dir = Filename.substr(0, SlashIndex);
    }

    bool Ret = true;

    // init des tex
    for (uint i = 0 ; i < pScene->mNumMaterials; i++) {
        const aiMaterial* pMaterial = pScene->mMaterials[i];

        m_Textures[i] = NULL;

        if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            aiString Path;

            if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                string p(Path.data);
                
                if (p.substr(0, 2) == ".\\") {                    
                    p = p.substr(2, p.size() - 2);
                }

                string FullPath = Dir + "/" + p;

                m_Textures[i] = new Texture_to_skinning(GL_TEXTURE_2D, FullPath.c_str());

                if (!m_Textures[i]->Load()) {
                    printf("Error loading texture '%s'\n", FullPath.c_str());
                    delete m_Textures[i];
                    m_Textures[i] = NULL;
                    Ret = false;
                }
                else {
                    //printf("texture %d load : '%s'\n", i, FullPath.c_str());
                }
            }
        }
    }


    string FullPath = Dir + "/" + "../../../../../mixamo/data/skins/skeletonzombie_t_avelange/skeletonzombie_t_avelange.fbm/skeletonZombie_specular.png";
    m_Textures[1] = new Texture_to_skinning(GL_TEXTURE_2D, FullPath.c_str());
    if (!m_Textures[1]->Load()) {
        printf("Error loading texture '%s'\n", FullPath.c_str());
        delete m_Textures[1];
        m_Textures[1] = NULL;
        Ret = false;
    }
    else {
        //printf("texture %d load : '%s'\n", 1, FullPath.c_str());
    }


    return Ret;
}



void Clear()
{
    for (uint i = 0 ; i < m_Textures.size() ; i++) {
        SAFE_DELETE(m_Textures[i]);
    }

    if (m_Buffers[0] != 0) {
        glDeleteBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);
    }
       
    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
}



    
enum VB_TYPES {
    INDEX_BUFFER,
    POS_VB,
    NORMAL_VB,
    TEXCOORD_VB,
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


// classe utilisé pour chargé les mesh animé
class SkinnedMesh
{
public:
   
   const aiScene* m_pScene;
   uint m_NumBones;
   vector<BoneInfo> m_BoneInfo;



    SkinnedMesh()
    {
        m_VAO = 0;

        ZERO_MEM(m_Buffers);
     
        m_NumBones = 0;
       
        m_pScene = NULL;
       
    }
    
    ~SkinnedMesh()
    {
        Clear();
    }


    bool LoadMesh(const string& Filename)
    {
    // Release the previously loaded mesh (if it exists)
        Clear();

    // Create the VAO
        glGenVertexArrays(1, &m_VAO);   
        glBindVertexArray(m_VAO);

    // Create the buffers for the vertices attributes
    
        //std::cout << "test val = " << ARRAY_SIZE_IN_ELEMENTS(m_Buffers) << std::endl;
        glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);

        bool Ret = false;    

        //std::cout << "TEST" << std::endl;
        m_pScene = m_Importer.ReadFile(Filename.c_str(), aiProcess_Triangulate | /*aiProcess_GenSmoothNormals |*/ aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

        //std::cout << "TEST2" << std::endl;

     /*   aiString bla;   
        m_Importer.GetExtensionList (bla);
        cout << bla.C_Str() << endl;*/


        if (m_pScene){  


            if(m_pScene->mAnimations[0])
                //printf("scene chargée\n");



            //m_GlobalInverseTransform = m_pScene->mRootNode->mTransformation;
                m_GlobalInverseTransform = aiMatrix4x4ToGlm(&m_pScene->mRootNode->mTransformation);

            //std::cout<<glm::to_string(m_GlobalInverseTransform)<<std::endl;
                m_GlobalInverseTransform = glm::inverse(m_GlobalInverseTransform);

            //std::cout<<glm::to_string(m_GlobalInverseTransform)<<std::endl;
                Ret = InitFromScene(m_pScene, Filename);

        }
        else {
            printf("Error parsing '%s': '%s'\n", Filename.c_str(), m_Importer.GetErrorString());
        }

    // Make sure the VAO is not changed from the outside
        glBindVertexArray(0);   

        return Ret;
    }



    void Render()
    {
        glBindVertexArray(m_VAO);

        for (uint i = 0 ; i < m_Entries.size() ; i++) {
        //    const uint MaterialIndex = m_Entries[i].MaterialIndex;

          //  assert(MaterialIndex < m_Textures.size());

            //if (m_Textures[MaterialIndex]) {
                m_Textures[0]->Bind(GL_TEXTURE0);
                m_Textures[1]->Bind(GL_TEXTURE1);
                m_Textures[2]->Bind(GL_TEXTURE2);
            
            //}

                //glActiveTexture(GL_TEXTURE0);
                //glBindTexture(GL_TEXTURE_2D, m_Textures[0]);
                //glActiveTexture(GL_TEXTURE1);
                //glBindTexture(GL_TEXTURE_2D, tex_depthMap);

                if(i == 1 || i == 0)
                    glDrawElementsBaseVertex(GL_TRIANGLES, m_Entries[i].NumIndices, GL_UNSIGNED_INT, (void*)(sizeof(uint) * m_Entries[i].BaseIndex), m_Entries[i].BaseVertex);
        }

    // Make sure the VAO is not changed from the outside    
        glBindVertexArray(0);
    }


    
    uint NumBones() const
    {
        return m_NumBones;
    }
    

    void Print_info_model(){


        std::cout << "\nSKINNED MODEL:\nnb mesh(s) = " << m_Entries.size() << std::endl;
        
        for(uint i = 0; i < m_Entries.size(); i++){
            std::cout << "mesh " << i << " = " << m_Entries[i].NumIndices << " vertice(s)" << std::endl;
        }

        std::cout << "nb bone(s) = " << m_NumBones << std::endl;
        
        std::cout << "nb tex(s) = " << m_Textures.size() << std::endl;
        
        std::cout << "num max bones = " << num_bones_per_vertex << std::endl;        

        printf("\n");

    }
    

    void BoneTransform(float TimeInSeconds, vector<glm::mat4>& Transforms, SkinnedMesh_animation Model /*const aiScene* Model_scene, uint Model_num_bones, vector<BoneInfo> Model_bone_info*/)
    {
        glm::mat4 Identity;
        Identity = glm::mat4(1.0);
        //Identity.InitIdentity();

        float TicksPerSecond = (float)(/*m_pScene*/Model.m_pScene->mAnimations[0]->mTicksPerSecond != 0 ? /*m_pScene*/Model.m_pScene->mAnimations[0]->mTicksPerSecond : 25.0f);
        float TimeInTicks = TimeInSeconds * TicksPerSecond;
        float AnimationTime = fmod(TimeInTicks, (float)/*m_pScene*/Model.m_pScene->mAnimations[0]->mDuration);

        ReadNodeHeirarchy(AnimationTime, Model.m_pScene->mRootNode, Identity, Model);
        

        Transforms.resize(m_NumBones /*Model_num_bones*/);


        for (uint i = 0 ; i < m_NumBones/*Model_num_bones*/ ; i++) {
            Transforms[i] = m_BoneInfo[i].FinalTransformation /*glm::mat4(1.0)*/;
        //print_mat4(Model.m_BoneInfo[i].FinalTransformation);

        }
    }



private:

    
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



    void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
    {
        if (pNodeAnim->mNumScalingKeys == 1) {
            Out = pNodeAnim->mScalingKeys[0].mValue;
            return;
        }

        uint ScalingIndex = FindScaling(AnimationTime, pNodeAnim);
        uint NextScalingIndex = (ScalingIndex + 1);
        assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
        float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
        float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
        assert(Factor >= 0.0f && Factor <= 1.0f);
        const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
        const aiVector3D& End   = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
        aiVector3D Delta = End - Start;
        Out = Start + Factor * Delta;
    }

    

    void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
    {
    // we need at least two values to interpolate...
        if (pNodeAnim->mNumRotationKeys == 1) {
            Out = pNodeAnim->mRotationKeys[0].mValue;
            return;
        }
    
        uint RotationIndex = FindRotation(AnimationTime, pNodeAnim);
        uint NextRotationIndex = (RotationIndex + 1);
        assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
        float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
        float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
        assert(Factor >= 0.0f && Factor <= 1.0f);
        const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
        const aiQuaternion& EndRotationQ   = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;    
        aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
        Out = Out.Normalize();
    }
    

    
    void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
    {
        if (pNodeAnim->mNumPositionKeys == 1) {
            Out = pNodeAnim->mPositionKeys[0].mValue;
            return;
        }

        uint PositionIndex = FindPosition(AnimationTime, pNodeAnim);
        uint NextPositionIndex = (PositionIndex + 1);
        assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
        float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
        float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
        assert(Factor >= 0.0f && Factor <= 1.0f);
        const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
        const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
        aiVector3D Delta = End - Start;
        Out = Start + Factor * Delta;
    }


    uint FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
    {
        assert(pNodeAnim->mNumScalingKeys > 0);

        for (uint i = 0 ; i < pNodeAnim->mNumScalingKeys - 1 ; i++) {
            if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
                return i;
            }
        }

        assert(0);

        return 0;
    }


    uint FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
    {
        assert(pNodeAnim->mNumRotationKeys > 0);

        for (uint i = 0 ; i < pNodeAnim->mNumRotationKeys - 1 ; i++) {
            if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
                return i;
            }
        }

        assert(0);

        return 0;
    }

    uint FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
    {    
        for (uint i = 0 ; i < pNodeAnim->mNumPositionKeys - 1 ; i++) {
            if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
                return i;
            }
        }

        assert(0);

        return 0;
    }

    

    const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const string NodeName)
    {
        for (uint i = 0 ; i < pAnimation->mNumChannels ; i++) {
            const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

            if (string(pNodeAnim->mNodeName.data) == NodeName) {
            return pNodeAnim;
        }
    }
    
    return NULL;
    }


    
    void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform, SkinnedMesh_animation Model)
    {    
        string NodeName(pNode->mName.data);

        const aiAnimation* pAnimation = /*m_pScene*/Model.m_pScene->mAnimations[0];
        
        glm::mat4 NodeTransformation = aiMatrix4x4ToGlm(&pNode->mTransformation);


        const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnimation, NodeName);

        if (pNodeAnim) {
        // Interpolate scaling and generate scaling transformation matrix
            aiVector3D Scaling;
            CalcInterpolatedScaling(Scaling, AnimationTime, pNodeAnim);
            glm::mat4 ScalingM = glm::mat4(1.0);
            ScalingM = glm::scale(ScalingM, glm::vec3(Scaling.x, Scaling.y, Scaling.z));
            //ScalingM.InitScaleTransform(Scaling.x, Scaling.y, Scaling.z);

        // Interpolate rotation and generate rotation transformation matrix
            aiQuaternion RotationQ;
            CalcInterpolatedRotation(RotationQ, AnimationTime, pNodeAnim);        
            aiMatrix4x4 temp = aiMatrix4x4(RotationQ.GetMatrix());
            glm::mat4 RotationM = aiMatrix4x4ToGlm(&temp);

        // Interpolate translation and generate translation transformation matrix
            aiVector3D Translation;
            CalcInterpolatedPosition(Translation, AnimationTime, pNodeAnim);
            glm::mat4 TranslationM = glm::mat4(1.0);
            TranslationM = glm::translate(TranslationM, glm::vec3(Translation.x, Translation.y, Translation.z));
            //TranslationM.InitTranslationTransform(Translation.x, Translation.y, Translation.z);

        // Combine the above transformations
            NodeTransformation = TranslationM * RotationM * ScalingM;
        }

        glm::mat4 GlobalTransformation = ParentTransform * NodeTransformation;

        if (m_BoneMapping.find(NodeName) != m_BoneMapping.end()) {
            uint BoneIndex = m_BoneMapping[NodeName];
            m_BoneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation * m_BoneInfo[BoneIndex].BoneOffset;
        }

        for (uint i = 0 ; i < pNode->mNumChildren ; i++) {
            ReadNodeHeirarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation, Model);
        }
    }

    
    
bool InitFromScene(const aiScene* pScene, const string& Filename)
{  
    m_Entries.resize(pScene->mNumMeshes);
    m_Textures.resize((pScene->mNumMaterials)+1);

    num_bones_per_vertex = 0;

    //std::cout << "\nNumMaterials = " << pScene->mNumMaterials << std::endl;        


    vector<glm::vec3> Positions;
    vector<glm::vec3> Normals;
    vector<glm::vec2> TexCoords;
    vector<glm::vec3> Tangents;
    vector<VertexBoneData> Bones;
    vector<uint> Indices;
       
    uint NumVertices = 0;
    uint NumIndices = 0;
    
    // Count the number of vertices and indices
    for (uint i = 0 ; i < m_Entries.size() ; i++) {
        m_Entries[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;        
        m_Entries[i].NumIndices    = pScene->mMeshes[i]->mNumFaces * 3;
        m_Entries[i].BaseVertex    = NumVertices;
        m_Entries[i].BaseIndex     = NumIndices;
        
        NumVertices += pScene->mMeshes[i]->mNumVertices;
        NumIndices  += m_Entries[i].NumIndices;
    }
    
    // Reserve space in the vectors for the vertex attributes and indices
    Positions.reserve(NumVertices);
    Normals.reserve(NumVertices);
    TexCoords.reserve(NumVertices);
    Tangents.reserve(NumVertices);
    Bones.resize(NumVertices);
    Indices.reserve(NumIndices);
        
    // Initialize the meshes in the scene one by one
    for (uint i = 0 ; i < m_Entries.size() ; i++) {

        //std::cout << "\nmax bones = " << max_bones(pScene->mMeshes[i]) << std::endl;        

        if(max_bones(pScene->mMeshes[i]) > num_bones_per_vertex){
            num_bones_per_vertex = max_bones(pScene->mMeshes[i]);
        }
        
        const aiMesh* paiMesh = pScene->mMeshes[i];
        InitMesh(i, paiMesh, Positions, Normals, TexCoords, Tangents, Bones, Indices);

    }


    if (!InitMaterials(pScene, Filename)) {
        return false;
    }
 


    // Generate and populate the buffers with vertex attributes and the indices
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[POS_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Positions[0]) * Positions.size(), &Positions[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(POSITION_LOCATION);
    glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);    

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[TEXCOORD_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoords[0]) * TexCoords.size(), &TexCoords[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(TEX_COORD_LOCATION);
    glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[NORMAL_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Normals[0]) * Normals.size(), &Normals[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(NORMAL_LOCATION);
    glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[TANGENT_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Tangents[0]) * Tangents.size(), &Tangents[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(TANGENT_LOCATION);
    glVertexAttribPointer(TANGENT_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);


    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[BONE_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Bones[0]) * Bones.size(), &Bones[0], GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(BONE_ID_LOCATION);
    glVertexAttribIPointer(BONE_ID_LOCATION, 4, GL_INT, sizeof(VertexBoneData), (const GLvoid*)0);
    glEnableVertexAttribArray(BONE_ID_LOCATION+1);
    //glVertexAttribIPointer(BONE_ID_LOCATION+1, 2, GL_INT, sizeof(VertexBoneData), (const void *)(4*(sizeof(int))));
    glVertexAttribIPointer(BONE_ID_LOCATION+1, 1, GL_INT, sizeof(VertexBoneData), (const void *)(4*(sizeof(int))));
   

    glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);    
    //glVertexAttribPointer(BONE_WEIGHT_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const void *)(6*(sizeof(int))));
    glVertexAttribPointer(BONE_WEIGHT_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const void *)(5*(sizeof(int))));
    
    glEnableVertexAttribArray(BONE_WEIGHT_LOCATION+1);    
    //glVertexAttribPointer(BONE_WEIGHT_LOCATION+1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const void *)((6*(sizeof(int)))+(4*(sizeof(float)))));
    glVertexAttribPointer(BONE_WEIGHT_LOCATION+1, 1, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const void *)((5*(sizeof(int)))+(4*(sizeof(float)))));
    


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices[0]) * Indices.size(), &Indices[0], GL_STATIC_DRAW);

    return GLCheckError();
}


    
void InitMesh(uint MeshIndex, const aiMesh* paiMesh,
                    vector<glm::vec3>& Positions,
                    vector<glm::vec3>& Normals,
                    vector<glm::vec2>& TexCoords,
                    vector<glm::vec3>& Tangents,
                    vector<VertexBoneData>& Bones,
                    vector<uint>& Indices) {
    
    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
    
    // Populate the vertex attribute vectors
    for (uint i = 0 ; i < paiMesh->mNumVertices ; i++) {
        const aiVector3D* pPos      = &(paiMesh->mVertices[i]);
        const aiVector3D* pNormal   = &(paiMesh->mNormals[i]);
        const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;
        const aiVector3D* pTangent = &(paiMesh->mTangents[i]);

        Positions.push_back(glm::vec3(pPos->x, pPos->y, pPos->z));
        Normals.push_back(glm::vec3(pNormal->x, pNormal->y, pNormal->z));
        TexCoords.push_back(glm::vec2(pTexCoord->x, pTexCoord->y));
        Tangents.push_back(glm::vec3(pTangent->x, pTangent->y, pTangent->z));        
    }
    
    LoadBones(MeshIndex, paiMesh, Bones);
    
    // Populate the index buffer
    for (uint i = 0 ; i < paiMesh->mNumFaces ; i++) {
        const aiFace& Face = paiMesh->mFaces[i];
        assert(Face.mNumIndices == 3);
        Indices.push_back(Face.mIndices[0]);
        Indices.push_back(Face.mIndices[1]);
        Indices.push_back(Face.mIndices[2]);
    }
}


    
void LoadBones(uint MeshIndex, const aiMesh* pMesh, vector<VertexBoneData>& Bones)
{
    for (uint i = 0 ; i < pMesh->mNumBones ; i++) {                
        uint BoneIndex = 0;        
        string BoneName(pMesh->mBones[i]->mName.data);
        
        if (m_BoneMapping.find(BoneName) == m_BoneMapping.end()) {
            // Allocate an index for a new bone
            BoneIndex = m_NumBones;
            m_NumBones++;            
            BoneInfo bi;            
            m_BoneInfo.push_back(bi);
            m_BoneInfo[BoneIndex].BoneOffset = aiMatrix4x4ToGlm(&pMesh->mBones[i]->mOffsetMatrix);            
            m_BoneMapping[BoneName] = BoneIndex;
        }
        else {
            BoneIndex = m_BoneMapping[BoneName];
        }                      
        
        for (uint j = 0 ; j < pMesh->mBones[i]->mNumWeights ; j++) {
            uint VertexID = m_Entries[MeshIndex].BaseVertex + pMesh->mBones[i]->mWeights[j].mVertexId;
            float Weight  = pMesh->mBones[i]->mWeights[j].mWeight;                   
            Bones[VertexID].AddBoneData(BoneIndex, Weight);
        }
    }    
}



bool InitMaterials(const aiScene* pScene, const string& Filename)
{
    // Extract the directory part from the file name
    string::size_type SlashIndex = Filename.find_last_of("/");
    string Dir;

    if (SlashIndex == string::npos) {
        Dir = ".";
    }
    else if (SlashIndex == 0) {
        Dir = "/";
    }
    else {
        Dir = Filename.substr(0, SlashIndex);
    }

    bool Ret = true;


    // init des tex
    for (uint i = 0 ; i < pScene->mNumMaterials; i++) {
        const aiMaterial* pMaterial = pScene->mMaterials[i];

        //std::cout << "TEST = " << pScene->mMaterials[0]. << std::endl;



        m_Textures[i] = NULL;

        if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            aiString Path;


            if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                string p(Path.data);

                
                if (p.substr(0, 2) == ".\\") {                    
                    p = p.substr(2, p.size() - 2);
                }

                string FullPath = Dir + "/" + p;


                m_Textures[i] = new Texture_to_skinning(GL_TEXTURE_2D, FullPath.c_str());


                if (!m_Textures[i]->Load()) {

                    printf("Error loading texture '%s'\n", FullPath.c_str());
                    delete m_Textures[i];
                    m_Textures[i] = NULL;
                    Ret = false;
                }
                else {
                    //printf("texture %d load : '%s'\n", i, FullPath.c_str());
                }
            }
        }
    }



    string FullPath = Dir + "/" + "../../../../../mixamo/data/skins/paladin_prop_j_nordstrom/paladin_prop_j_nordstrom.fbm/Paladin_specular.png";
    m_Textures[1] = new Texture_to_skinning(GL_TEXTURE_2D, FullPath.c_str());
    if (!m_Textures[1]->Load()) {
        printf("Error loading texture '%s'\n", FullPath.c_str());
        delete m_Textures[1];
        m_Textures[1] = NULL;
        Ret = false;
    }
    else {
        //printf("texture %d load : '%s'\n", 1, FullPath.c_str());
    }

    FullPath = Dir + "/" + "../../../../../mixamo/data/skins/paladin_prop_j_nordstrom/paladin_prop_j_nordstrom.fbm/Paladin_normal.png";
    m_Textures[2] = new Texture_to_skinning(GL_TEXTURE_2D, FullPath.c_str());
    if (!m_Textures[2]->Load()) {
        printf("Error loading texture '%s'\n", FullPath.c_str());
        delete m_Textures[2];
        m_Textures[2] = NULL;
        Ret = false;
    }
    else {
        //printf("texture %d load : '%s'\n", 1, FullPath.c_str());
    }


    return Ret;
}



void Clear()
{
    for (uint i = 0 ; i < m_Textures.size() ; i++) {
        SAFE_DELETE(m_Textures[i]);
    }

    if (m_Buffers[0] != 0) {
        glDeleteBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);
    }
       
    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
}



    
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




// CLASS PARTICLE GENERATOR
class ParticleGenerator
{
public:

    float time_acc,time_acc2;
    int nb_frame, atlas_size, wind_up, wind_down;
    float wind_str, blend_factor, reflect_angle;
    glm::mat4 billboard_mat;

    // Constructor
   ParticleGenerator(Shader shader,GLuint amount)
   : shader(shader), amount(amount)
   {
    this->init();
    }

    // update wind velocity
    void update_wind(float dt){

        static float wind_speed =  0.001;

        if(this->wind_up == 1){
            this->wind_str += dt * wind_speed /*0.001*/;
        }

        if(this->wind_down == 1){
            this->wind_str -= dt * wind_speed /*0.001*/;
        }

        if(this->wind_str >= 0.15 && this->wind_up == 1){
            this->wind_str = 0.15;
            this->wind_up = 0;
            this->wind_down = 1;
            
            if(wind_speed < 0.0015){
                wind_speed = rand_FloatRange(0.0007, 0.002);
            }else{
                wind_speed = rand_FloatRange(0.0007, 0.0009);
            }
            //std::cout << "test1 = " << wind_speed << std::endl;
        }

        if(this->wind_str <= -0.15 && this->wind_down == 1){
            this->wind_str = - 0.15;
            this->wind_down = 0;
            this->wind_up = 1;
            
            if(wind_speed < 0.0015){
                wind_speed = rand_FloatRange(0.0007, 0.002);
            }else{
                wind_speed = rand_FloatRange(0.0007, 0.0009);
            }
            //std::cout << "test2 = " << wind_speed << std::endl;
        }

    }


    // Update all particles
    void Update(GLfloat dt, float limit_time, glm::vec2 offset)
    {

        // frame calculation
        this->time_acc2 += dt;
        if(this->time_acc2 >= 0.01){
            for (GLuint i = 0; i < this->amount; ++i)
            {
                Particle &p = this->particles[i];

                p.actual_frame += 1;
                this->time_acc2 = 0.0;

                if(p.actual_frame > this->nb_frame)
                    p.actual_frame = 0;

            }
        }

        // search if new particle have to spawn
        this->time_acc+= dt;
        GLuint nb_new_particle = 0;
        if(this->time_acc > limit_time){
            while(this->time_acc >= limit_time){
                this->time_acc -= limit_time;
                nb_new_particle++;
            }

            //std::cout << "test = " << nb_new_particle << std::endl;    
     
            // Add new particles 
            for (GLuint i = 0; i < nb_new_particle; ++i)
            {
                int unusedParticle = this->firstUnusedParticle();
                //printf("test = %d\n", unusedParticle);
                this->respawnParticle(this->particles[unusedParticle], offset);
            }

            // Update all particles
            for (GLuint i = 0; i < this->amount; ++i)
            {
                Particle &p = this->particles[i];
                p.Life -= dt*3.0 /*2.0*/; // reduce life

                // particle is alive, then update
                if (p.Life > 0.0f)
                {   
                   
                    p.Position += p.Velocity * dt; 
                    p.Color.a -= dt * 0.15;

                    // WIND EFFECT
                    update_wind(dt);
                    p.Velocity.x += dt * this->wind_str;
                    //std::cout << "wind_str = " << this->wind_str << std::endl;


                    // SMOKE
                    /*if(p.Position.y > 0.2){
                        //p.scale *= 0.995;
                        p.Velocity *= 1.003;
                    }*/
                    if(p.Position.y > /*0.15*/ 0.075){

                        //std::cout << "pos x = " << p.Position.x << std::endl;
                        /*p.Color.r = 0.0;
                        p.Color.g = 1.0;
                        p.Color.b = 0.0;*/  

                        if(p.Position.x > /*0.03*/ 0.02)
                            p.Position.x -= dt * /*0.04*/ 0.06;

                        if(p.Position.x < /*-0.03*/ -0.02)
                            p.Position.x += dt * /*0.04*/ 0.06;

                    }
                    if(p.Position.y > 0.25){
                        p.Color.r = 0.0;
                        p.Color.g = 0.0;
                        p.Color.b = 0.0;
                    }
                }

            }
        }

    }

// Render all particles
void Draw(bool depth_test, bool reflection_render)
{
    float x_num;
    float y_num;

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
    if(depth_test)
        glDepthMask(0);
    
    this->shader.Use();
    for (Particle particle : this->particles)
    {
        if (particle.Life > 0.0f)
        {

            if(depth_test){
                x_num = particle.actual_frame % this->atlas_size;
                y_num = particle.actual_frame / this->atlas_size;

                glUniform1f(glGetUniformLocation(this->shader.Program, "x_num"), x_num);
                glUniform1f(glGetUniformLocation(this->shader.Program, "y_num"), y_num);
                glUniform1f(glGetUniformLocation(this->shader.Program, "atlas_size"), this->atlas_size);
                glUniform1f(glGetUniformLocation(this->shader.Program, "sens"), particle.sens);
            }
            
            glm::mat4 Msend = glm::mat4(1.0);

            if(reflection_render){
                Msend = glm::rotate(Msend, 0.3f, glm::vec3(0.0, 0.0 , 1.0));
                Msend = glm::rotate(Msend, this->reflect_angle, glm::vec3(0.0, 1.0 , 0.0));
            }

            Msend = glm::translate(Msend, glm::vec3(particle.Position.x,particle.Position.y,0.0));

            Msend = glm::scale(Msend, glm::vec3(particle.scale,particle.scale,1.0)); 

            glUniformMatrix4fv(glGetUniformLocation(this->shader.Program, "offset"), 1, GL_FALSE, glm::value_ptr(Msend));
            glUniform4f(glGetUniformLocation(this->shader.Program, "ParticleColor"), particle.Color.x,particle.Color.y,particle.Color.z,particle.Color.w);

            glBindVertexArray(this->VAO);
            
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindVertexArray(0);
        }
    }
    
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    if(depth_test)
        glDepthMask(1);
    
    glDisable(GL_BLEND);
    
    
    
}



//private:
    // State
    std::vector<Particle> particles;
    GLuint amount;
    Shader shader;
    // Render state
    GLuint VAO;
  
    // Initializes buffer and vertex attributes

    void init()
    {
    // Set up mesh and attribute properties
        GLuint VBO;
        GLfloat particle_quad[] = {
           /* 0.0f, 1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 1.0f, 1.0f,
            0.0f, 0.0f, 0.0f, 1.0f,

            0.0f, 1.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 0.0f,
            1.0f, 0.0f, 1.0f, 1.0f*/

            /*-1.0,1.0, 0.0f, 0.0f,
            -0.4,1.0, 1.0f, 0.0f,
            -1.0,0.2, 0.0f, 1.0f,
            -0.4,0.2, 1.0f, 1.0f*/

            -0.5,1.0, 0.0f, 0.0f,
            0.5,1.0, 1.0f, 0.0f,
            -0.5,0.0, 0.0f, 1.0f,
            0.5,0.0, 1.0f, 1.0f

        }; 
        glGenVertexArrays(1, &this->VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(this->VAO);
    // Fill mesh buffer
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);
    // Set mesh attributes
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
        glBindVertexArray(0);

    // Create this->amount default particle instances
        for (GLuint i = 0; i < this->amount; ++i){
            this->particles.push_back(Particle());

        }
        this->time_acc = 0.0;
        this->time_acc2 = 0.0;
        this->nb_frame = /*32*/ 64 /*16*/ - 1;
        this-> atlas_size = /*4*/ 8;
        this-> wind_str = 0.0;
        this->wind_up = 0;
        this->wind_down = 1;
        this->blend_factor = 0.106;
        this->reflect_angle = 6.61;

    }
    

    // Returns the first Particle index that's currently unused e.g. Life <= 0.0f or 0 if no particle is currently inactive
    GLuint lastUsedParticle = 0;
    GLuint firstUnusedParticle()
    {
        //printf("test = %d\n", lastUsedParticle);
    // First search from last used particle, this will usually return almost instantly
        for (GLuint i = lastUsedParticle; i < this->amount; ++i){
            if (this->particles[i].Life <= 0.0f){
                lastUsedParticle = i;
                return i;
            }
        }
    // Otherwise, do a linear search
        for (GLuint i = 0; i < lastUsedParticle; ++i){
            if (this->particles[i].Life <= 0.0f){
                lastUsedParticle = i;
                return i;
            }
        }
    // All particles are taken, override the first one (note that if it repeatedly hits this case, more particles should be reserved)
        lastUsedParticle = 0;
        return 0;
    }


    // Respawns particle
    void respawnParticle(Particle &particle, glm::vec2 offset)
    {
        GLfloat random = /*((rand() % 100) - 50) / 10.0f*/rand_FloatRange(-1.3, 1.3);
        GLfloat rColor = /*0.5 + ((rand() % 100) / 100.0f)*/0.12;
        particle.Position = glm::vec2((random)*0.06, -0.1) ;
        particle.Color = glm::vec4((255.0/255.0)*0.4, (140.0/255.0)*0.4, (80.0/255.0)*0.4, 0.5f);
        //particle.Color = glm::vec4((255.0/255.0)*0.3, (140.0/255.0)*0.3, (80.0/255.0)*0.3, 0.5f);
        particle.Life = 8.0f;
        particle.Velocity = glm::vec2(0.0,rand_FloatRange(0.5,2.0)) * 0.1f;
        particle.actual_frame = rand_FloatRange(0,this->nb_frame);
        particle.sens = rand_FloatRange(0,2);
        particle.scale = 0.2;
        
    }



};
