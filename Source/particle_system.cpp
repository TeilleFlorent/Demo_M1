#include "particle_system.hpp"


    // update wind velocity
    void ParticleGenerator::update_wind(float dt){

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

    
    float ParticleGenerator::rand_FloatRange(float a, float b)
    {
        return ((b-a)*((float)rand()/RAND_MAX))+a;
    }



    // Update all particles
    void ParticleGenerator::Update(GLfloat dt, float limit_time, glm::vec2 offset)
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


                    if(p.Position.y > /*0.15*/ 0.075){


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
void ParticleGenerator::Draw(bool depth_test, bool reflection_render)
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


    void ParticleGenerator::init()
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
    GLuint ParticleGenerator::firstUnusedParticle()
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
    void ParticleGenerator::respawnParticle(Particle &particle, glm::vec2 offset)
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

