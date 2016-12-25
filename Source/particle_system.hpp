#ifndef SHADER_HPP
#define SHADER_HPP
#include "shader.hpp"
#endif

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


#define myPI 3.141593
#define myPI_2 1.570796




struct Particle {
    glm::vec2 Position, Velocity;
    glm::vec4 Color;
    GLfloat Life;
    int actual_frame;
    int sens;
    float scale;
    Particle() : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f) { }
};



// CLASS PARTICLE GENERATOR
class ParticleGenerator
{

public:

    float time_acc,time_acc2;
    int nb_frame, atlas_size, wind_up, wind_down;
    float wind_str, blend_factor, reflect_angle;
    glm::mat4 billboard_mat;

    std::vector<Particle> particles;
    GLuint amount;
    Shader shader;
    GLuint VAO;
    GLuint lastUsedParticle;

    glm::vec3 particules_pos; 
    float fire_intensity;
  

    ParticleGenerator(Shader shader,GLuint amount): shader(shader), amount(amount)
    {
        this->init();
        lastUsedParticle = 0;
        particules_pos = glm::vec3(-1.27, 4.961, 0.03);
        fire_intensity = 1.0f;
    }


    float rand_FloatRange(float a, float b);

    // update wind velocity
    void update_wind(float dt);

    // Update all particles
    void Update(GLfloat dt, float limit_time, glm::vec2 offset);

    // Render all particles
    void Draw(bool depth_test, bool reflection_render);

    void init();

    GLuint firstUnusedParticle();
    
    // Respawns particle
    void respawnParticle(Particle &particle, glm::vec2 offset);



};
