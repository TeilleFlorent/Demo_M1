#include "train.hpp"
#include "ground.cpp"

static SDL_Window * _win = NULL;
static SDL_GLContext _oglContext = NULL;

  // SHADERS
Shader basic_shader;
Shader skybox_shader;
Shader lamp_shader;
Shader skinning_shader;
Shader particule_shader;
Shader screen_shader;
Shader blur_shader;


// MODELS
Model feu_model;
Model paladin_model;
Model house_model;
Model sword_model;
Model shield_model;
Model tree1_model;
Model rock1_model;
SkinnedMesh paladin_skinned;
SkinnedMesh_animation paladin_sitting_idle;
SkinnedMesh_animation paladin_standing_up;
SkinnedMesh_animation paladin_breathing_idle;
SkinnedMesh_animation paladin_warrior_idle;
vector<glm::mat4> Transforms;



static GLint m_boneLocation[MAX_BONES];
static GLint m_boneLocation2[MAX_BONES];


// HEIGHT MAP    
CMultiLayeredHeightmap MyMap;

// VAOs
static GLuint skyboxVAO = 0;
static GLuint lampVAO = 0;
static GLuint groundVAO = 0;
static GLuint screenVAO = 0;


// VBOs
static GLuint skyboxVBO = 0;
static GLuint lampVBO = 0;
static GLuint groundVBO = 0;
static GLuint screenVBO = 0;

// FBO
GLuint feu_depth_FBO; // FBO lié a la texture depth map
GLuint particle_render_FBO;
GLuint particle_depth_FBO;
GLuint house_depth_FBO;
GLuint reflection_cubeMap_FBO;
GLuint reflection_cubeMap_RBO;
GLuint VL_FBO[1];
GLuint VL_RBO[1];
GLuint shadow_cubeMap_FBO;
GLuint pingpongFBO[2];
GLuint pingpongRBO[2];
GLuint final_FBO[2];
GLuint final_RBO[2];

// identifiant du (futur) identifiant de texture
static GLuint tex_cube_map = 0;
static GLuint tex_ground_color = 0;
static GLuint tex_ground_normal = 0;
static GLuint tex_ground_AO = 0;
static GLuint tex_particule = 0;
static GLuint tex_pre_rendu_feu = 0;
static GLuint tex_depth_feu = 0; 
static GLuint tex_depth_particle = 0;
static GLuint tex_pre_rendu_feu2 = 0;
static GLuint tex_depth_feu2 = 0; 
static GLuint tex_depth_particle2 = 0;
static GLuint reflection_cubeMap = 0;
static GLuint tex_depth_house = 0;
static GLuint tex_color_VL[1];
static GLuint tex_shadow_cubeMap;
static GLuint pingpongColorbuffers[2];
static GLuint tex_final_color[2];

float depth_map_res_seed = /*2048.0*/ 1024.0;
float depth_map_res_x, depth_map_res_y, depth_map_res_x_house, depth_map_res_y_house;

float reflection_cubeMap_res = /*2048.0*/ 512;
float tex_VL_res_seed = 2048.0;
float tex_VL_res_x, tex_VL_res_y;



//dimension fenetre SDL
static int w = 800 * 1.5;
static int h = 600 * 1.5;
static int final_w = w;
static int final_h = h;


static glm::vec3 cameraPos   = glm::vec3(-0.591501,5.29917,0.0557512);
static glm::vec3 cameraFront = glm::vec3(0.95, 0.0, -0.3);
static glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
static float walk_speed = 0.1;
static float taille = 0.8f; // en mode fps

static float yaw = -18;
static float pitch = -1.6;

// LIGHTS
static light * lights;

// All objets
static objet * ground;
static objet Feu;
static objet paladin;
static objet house;
static objet sword;
static objet shield;
static objet * trees;
static int nb_tree = 3;
static objet * rocks;
static int nb_rock = 3;

//sphere para
static int longi = 10;
static int lati = 10;
static int nbVerticesSphere;

// camera 
int cameraZ = 0;
int cameraD = 0;
int cameraQ = 0;
int cameraS = 0;

//FLY
GLboolean fly_state = true;

// data cube map texture
std::vector<const GLchar*> faces;


// PARTICULE
ParticleGenerator * Particles;
glm::vec3 particules_pos(-1.27, 4.961, 0.03); 
static float fire_intensity = 1.0f;

// bias shadow
static float send_bias = 0.002;

// FOG PARA
static float skybox_alpha = 0.99;
float fog_density = 0.04f; //0.04f;
float fog_Start = 2.0f;
float fog_End = 5.0f;
glm::vec4 fog_color = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
float fog_equation = 2.0; 

// VL intensity
static float VL_intensity_max = 1.3;
static float VL_intensity = 0.0;
static float VL_offset_factor_max = 0.9;
static float VL_offset_factor = VL_offset_factor_max;

static int shadow_point_light = 0;

// SOUNDS
static Mix_Music * S_main_music = NULL;
static Mix_Chunk * S_wind_lowland = NULL; 
static Mix_Chunk * S_fire = NULL; 

// SCRIPT PARA
static bool script_on = true;
static int step = 0;
static float output_factor = 1.0;

/////////////////////////////////////////////////////////


// FONCTION MAIN
int main() {

  srand(time(NULL));

 
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    fprintf(stderr, "Erreur lors de l'initialisation de SDL :  %s", SDL_GetError());
    return -1;
  }
  atexit(SDL_Quit);


  initAudio();

  load_audio();


  if((_win = initWindow(w,h, &_oglContext))) {

   SDL_SetRelativeMouseMode(SDL_TRUE);

    initGL(_win);

    initData();

    // compilation des shaders
    basic_shader.set_shader("../shaders/basic.vs","../shaders/basic.fs");
    skybox_shader.set_shader("../shaders/skybox.vs","../shaders/skybox.fs");
    lamp_shader.set_shader("../shaders/lamp.vs","../shaders/lamp.fs");
    particule_shader.set_shader("../shaders/particule.vs", "../shaders/particule.fs");
    screen_shader.set_shader("../shaders/screen.vs", "../shaders/screen.fs");
    skinning_shader.set_shader("../shaders/skinning.vs", "../shaders/skinning.fs");
    blur_shader.set_shader("../shaders/blur.vs", "../shaders/blur.fs");


    // Set texture samples
    basic_shader.Use();
    glUniform1i(glGetUniformLocation(basic_shader.Program, "texture_diffuse1"), 0);
    glUniform1i(glGetUniformLocation(basic_shader.Program, "texture_specular1"), 1);
    glUniform1i(glGetUniformLocation(basic_shader.Program, "texture_normal1"), 2);
    glUniform1i(glGetUniformLocation(basic_shader.Program, "depth_map_particle"), 3);
    glUniform1i(glGetUniformLocation(basic_shader.Program, "tex_render_particle"), 4);
    glUniform1i(glGetUniformLocation(basic_shader.Program, "reflection_cubeMap"), 5);
    glUniform1i(glGetUniformLocation(basic_shader.Program, "shadow_map1"), 6);
    glUniform1i(glGetUniformLocation(basic_shader.Program, "texture_metalness"), 7);
    glUniform1i(glGetUniformLocation(basic_shader.Program, "shadow_cube_map"), 8);
    glUseProgram(0);

    skinning_shader.Use();
    glUniform1i(glGetUniformLocation(skinning_shader.Program, "texture_diffuse1"), 0);
    glUniform1i(glGetUniformLocation(skinning_shader.Program, "texture_specular1"), 1);
    glUniform1i(glGetUniformLocation(skinning_shader.Program, "texture_normal1"), 2);
    glUniform1i(glGetUniformLocation(skinning_shader.Program, "reflection_cubeMap"), 5);
    glUniform1i(glGetUniformLocation(skinning_shader.Program, "shadow_map1"), 3);
    glUniform1i(glGetUniformLocation(skinning_shader.Program, "shadow_cube_map"), 6);
    glUseProgram(0);

   
    screen_shader.Use();
    glUniform1i(glGetUniformLocation(screen_shader.Program, "depth_map_feu"), 0);
    glUniform1i(glGetUniformLocation(screen_shader.Program, "tex_particle"), 1);
    glUseProgram(0);

    particule_shader.Use();
    glUniform1i(glGetUniformLocation(particule_shader.Program, "MyTex"), 0);
    glUniform1i(glGetUniformLocation(particule_shader.Program, "depth_map_feu"), 1);
    glUniform1i(glGetUniformLocation(particule_shader.Program, "particles_pre_render"), 2);
    glUseProgram(0);

    blur_shader.Use();
    glUniform1i(glGetUniformLocation(blur_shader.Program, "image"), 0);
    glUseProgram(0);
    


    ///////////////// Load les models && les animations  
    feu_model.Load_Model("../Models/feu/3ds/fireplace.3ds", 4);
    //feu_model.Print_info_model();

    sword_model.Load_Model("../Models/sword3/Longsword_LP.obj", 3);    
    //sword_model.Print_info_model();

    shield_model.Load_Model("../Models/shield/Shield_MedPoly.obj", 5);    
    //shield_model.Print_info_model();

    //tree1_model.Load_Model("../Models/tree1/DeadTree1.obj", 6);    
    //tree1_model.Print_info_model();

   /* tree1_model.Load_Model("../Models/tree2/Tree_Dry_1.obj", 6);    
    tree1_model.Print_info_model();*/

   /* rock1_model.Load_Model("../Models/rock2/Rock_10.obj", 7);    
    rock1_model.Print_info_model();*/

    house_model.Load_Model("../Models/house2/Abandoned house.obj", 2);
    //house_model.Print_info_model();

    paladin_skinned.LoadMesh("../Models/paladin_skinned/paladin/paladin/paladin/paladin/paladin/Paladin_w_Prop_J_Nordstrom.fbx");
    //paladin_skinned.Print_info_model();

    paladin_sitting_idle.LoadMesh("../Models/paladin_skinned/paladin/paladin/paladin/paladin/paladin/sitting_idle.fbx");
    paladin_standing_up.LoadMesh("../Models/paladin_skinned/paladin/paladin/paladin/paladin/paladin/standing_up.fbx");
    paladin_breathing_idle.LoadMesh("../Models/paladin_skinned/paladin/paladin/paladin/paladin/paladin/breathing_idle.fbx");
    paladin_warrior_idle.LoadMesh("../Models/paladin_skinned/paladin/paladin/paladin/paladin/paladin/warrior_idle.fbx");
    
    // gen le tab de uniform location du tab de bone dans les shader
    skinning_shader.Use();
    for (unsigned int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_boneLocation) ; i++) {
      char Name[128];
      memset(Name, 0, sizeof(Name));
      SNPRINTF(Name, sizeof(Name), "gBones[%d]", i);

      m_boneLocation[i] = glGetUniformLocation(skinning_shader.Program, Name);    

    }
    glUseProgram(0);

    /*depth_map_shader.Use();
    for (uint i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_boneLocation2) ; i++) {
      char Name[128];
      memset(Name, 0, sizeof(Name));
      SNPRINTF(Name, sizeof(Name), "gBones[%d]", i);

      //std::cout << temp_name << std::endl;

      m_boneLocation2[i] = glGetUniformLocation(depth_map_shader.Program, Name);    

      //std::cout << "bone location " << i << "= " << m_boneLocation[i] << std::endl;    
    }
    glUseProgram(0);
*/

    // generation du ground via une heigh map 
    MyMap.LoadHeightMapFromImage("../Textures/height_map3.jpeg");
  
    if(MyMap.bLoaded)
      printf("Map generé\n\n");    


    Particles = new ParticleGenerator(particule_shader, 1000);
  

    loop(_win);

  }
  return 0;
}


static void quit(void) {

  if(tex_ground_color)
    glDeleteTextures(1, &tex_ground_color);  
  if(tex_ground_normal)
    glDeleteTextures(1, &tex_ground_normal);  
  if(tex_ground_AO)
    glDeleteTextures(1, &tex_ground_AO);  
  if(tex_cube_map)
    glDeleteTextures(1, &tex_cube_map);
  if(tex_particule)
    glDeleteTextures(1, &tex_particule);
  if(tex_pre_rendu_feu)
    glDeleteTextures(1, &tex_pre_rendu_feu);
  if(tex_depth_feu)
    glDeleteTextures(1, &tex_depth_feu);
  if(tex_depth_particle)
    glDeleteTextures(1, &tex_depth_particle);
  if(tex_pre_rendu_feu2)
    glDeleteTextures(1, &tex_pre_rendu_feu2);
  if(tex_depth_feu2)
    glDeleteTextures(1, &tex_depth_feu2);
  if(tex_depth_particle2)
    glDeleteTextures(1, &tex_depth_particle2);
  if(reflection_cubeMap)
    glDeleteTextures(1, &reflection_cubeMap);
  if(tex_depth_house)
    glDeleteTextures(1, &tex_depth_house);
  if(tex_color_VL[0])
    glDeleteTextures(1, &tex_color_VL[0]);
  if(tex_shadow_cubeMap)
    glDeleteTextures(1, &tex_shadow_cubeMap);
  if(pingpongColorbuffers[0])
    glDeleteTextures(1, &pingpongColorbuffers[0]);
  if(pingpongColorbuffers[1])
    glDeleteTextures(1, &pingpongColorbuffers[1]);
  


  if(lampVBO)
    glDeleteBuffers(1, &lampVBO);
  if(groundVBO)
    glDeleteBuffers(1, &groundVBO);
  if(skyboxVBO)
    glDeleteBuffers(1, &skyboxVBO);
  if(feu_depth_FBO)
    glDeleteFramebuffers(1, &feu_depth_FBO);
  if(particle_render_FBO)
    glDeleteFramebuffers(1, &particle_render_FBO);
  if(particle_depth_FBO)
    glDeleteFramebuffers(1, &particle_depth_FBO);
  if(house_depth_FBO)
    glDeleteFramebuffers(1, &house_depth_FBO);
  


  if(_oglContext)
    SDL_GL_DeleteContext(_oglContext);

  if(_win)
    SDL_DestroyWindow(_win);


}


// fonction qui paramettre la fenetre SDL
static SDL_Window * initWindow(int w, int h, SDL_GLContext * poglContext) {
  SDL_Window * win = NULL;
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);


  if( (win = SDL_CreateWindow("Train", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
    w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | 
        SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN)) == NULL )
    return NULL;
    if( (*poglContext = SDL_GL_CreateContext(win)) == NULL ) {
      SDL_DestroyWindow(win);
      return NULL;
    }


    glewExperimental = GL_TRUE;
    GLenum err = glewInit();

    if (err != GLEW_OK)
    exit(1); // or handle the error in a nicer way
    if (!GLEW_VERSION_2_1)  // check that the machine supports the 2.1 API.
    exit(1); // or handle the error in a nicer way


  fprintf(stderr, "Version d'OpenGL : %s\n", glGetString(GL_VERSION));
  fprintf(stderr, "Version de shaders supportes : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));  
  atexit(quit);
  return win;
}




void initAudio() {

  int mixFlags = MIX_INIT_MP3 | MIX_INIT_OGG, res;
  res = Mix_Init(mixFlags);
  if( (res & mixFlags) != mixFlags ) {
    fprintf(stderr, "Mix_Init: Erreur lors de l'initialisation de la bibliothèque SDL_Mixer\n");
    fprintf(stderr, "Mix_Init: %s\n", Mix_GetError());
  }

  if(Mix_OpenAudio(44100  /*22050*/ , /*AUDIO_S16LSB*/ MIX_DEFAULT_FORMAT, 2, 1024) < 0){
    printf("BUG init audio\n");  
  //exit(-4);
  }

  Mix_VolumeMusic(MIX_MAX_VOLUME/3);

  Mix_AllocateChannels(10);

}

void load_audio(){

  if(!(S_wind_lowland = Mix_LoadWAV("../Sounds/wind.wav"))){ 
    fprintf(stderr, "BUG : %s\n", Mix_GetError());
  }

  if(!(S_fire = Mix_LoadWAV("../Sounds/fire.wav"))){ 
    fprintf(stderr, "BUG : %s\n", Mix_GetError());
  }


  if(!(S_main_music = Mix_LoadMUS("../Sounds/Blonde Redhead - For the Damaged Coda.mp3"))) {
     fprintf(stderr, "BUG : %s\n", Mix_GetError());
  }  



}



// set des paramettre liés à openGL
static void initGL(SDL_Window * win) {


  glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
  //glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glClearDepth(1.0);

  glDepthFunc(GL_LESS); 
  
  resizeGL(win);

  //glEnable(GL_BLEND);
  //glBlendEquation(GL_FUNC_ADD);
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 
  glEnable(GL_MULTISAMPLE); // active anti aliasing 
  
  //glFrontFace(GL_CCW);
  //glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
  
}



// fonction qui recupere et paramettre les data
static void initData(void) {

 float aniso = 0.0f; 

 glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso); // get la valeur pour l'aniso


  SDL_Surface * t = NULL;


 GLfloat skyboxVertices[] = {

  -1.0f,  1.0f, -1.0f,
  -1.0f, -1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,
  1.0f,  1.0f, -1.0f,
  -1.0f,  1.0f, -1.0f,

  -1.0f, -1.0f,  1.0f,
  -1.0f, -1.0f, -1.0f,
  -1.0f,  1.0f, -1.0f,
  -1.0f,  1.0f, -1.0f,
  -1.0f,  1.0f,  1.0f,
  -1.0f, -1.0f,  1.0f,

  1.0f, -1.0f, -1.0f,
  1.0f, -1.0f,  1.0f,
  1.0f,  1.0f,  1.0f,
  1.0f,  1.0f,  1.0f,
  1.0f,  1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,

  -1.0f, -1.0f,  1.0f,
  -1.0f,  1.0f,  1.0f,
  1.0f,  1.0f,  1.0f,
  1.0f,  1.0f,  1.0f,
  1.0f, -1.0f,  1.0f,
  -1.0f, -1.0f,  1.0f,

  -1.0f,  1.0f, -1.0f,
  1.0f,  1.0f, -1.0f,
  1.0f,  1.0f,  1.0f,
  1.0f,  1.0f,  1.0f,
  -1.0f,  1.0f,  1.0f,
  -1.0f,  1.0f, -1.0f,

  -1.0f, -1.0f, -1.0f,
  -1.0f, -1.0f,  1.0f,
  1.0f, -1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,
  -1.0f, -1.0f,  1.0f,
  1.0f, -1.0f,  1.0f

};


GLfloat screen[] ={

  /*-1.0,1.0,0.0,
  -0.4,1.0,0.0,
  -1.0,0.2,0.0,  
  -0.4,0.2,0.0,*/
  -1.0,1.0,0.0,
  1.0,1.0,0.0,
  -1.0,-1.0,0.0,  
  1.0,-1.0,0.0,
  
  ///////////////
    
/*  0.0f, 0.0f, 1.0f, 0.0f,      
  0.0f, 1.0f, 1.0f, 1.0f*/

  0.0f, 0.0f, 1.0f, 0.0f, 
  0.0f, 1.0f, 1.0f, 1.0f     
  

};



//SHPERE LAMP
GLfloat * dataLamp = buildSphere(longi, lati);
nbVerticesSphere = (6*3*longi*lati);



//GEN LES VAO
glGenVertexArrays(1, &skyboxVAO);
glGenVertexArrays(1, &lampVAO);
glGenVertexArrays(1, &groundVAO);
glGenVertexArrays(1, &screenVAO);


// GEN LES FBO
glGenFramebuffers(1, &feu_depth_FBO);  // FBO depth map particle
glGenFramebuffers(1, &particle_render_FBO);
glGenFramebuffers(1, &particle_depth_FBO);
glGenFramebuffers(1, &reflection_cubeMap_FBO);
glGenRenderbuffers(1, &reflection_cubeMap_RBO); // RBO du FBO
glGenFramebuffers(1, &house_depth_FBO);
glGenFramebuffers(1, &VL_FBO[0]);
glGenRenderbuffers(1, &VL_RBO[0]);
glGenFramebuffers(1, &shadow_cubeMap_FBO);
glGenFramebuffers(1, &final_FBO[0]);
glGenRenderbuffers(1, &final_RBO[0]);
glGenFramebuffers(1, &final_FBO[1]);
glGenRenderbuffers(1, &final_RBO[1]);


//////////////////////////////


// skybox VAO
glGenBuffers(1, &skyboxVBO);
glBindVertexArray(skyboxVAO);
glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
glBindVertexArray(0);



 //skybox texture
faces.push_back("../skybox/s2/front.png");
faces.push_back("../skybox/s2/back.png");
faces.push_back("../skybox/s2/top.png");
faces.push_back("../skybox/s2/bottom.png");
faces.push_back("../skybox/s2/right.png");
faces.push_back("../skybox/s2/left.png");


tex_cube_map = loadCubemap(faces);


// screen VAO
glBindVertexArray(screenVAO);
glEnableVertexAttribArray(0);
glEnableVertexAttribArray(1);
  
glGenBuffers(1, &screenVBO);
glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
glBufferData(GL_ARRAY_BUFFER, sizeof screen, screen, GL_STATIC_DRAW);
  
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0 ,(const void *)(0*(sizeof(float))));  
glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0 ,(const void *)(12*(sizeof(float))));

glBindBuffer(GL_ARRAY_BUFFER, 0);
glBindVertexArray(0);


// lamp VAO
glBindVertexArray(lampVAO);
glEnableVertexAttribArray(0);
glEnableVertexAttribArray(1);
glGenBuffers(1, &lampVBO);
glBindBuffer(GL_ARRAY_BUFFER, lampVBO);
glBufferData(GL_ARRAY_BUFFER,((6 * 6 * longi * lati)) * (sizeof(float)), dataLamp, GL_STATIC_DRAW);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*(sizeof(float)), (const void *)0);
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*(sizeof(float)), /*3*(sizeof(float))*/(const void *)0);
glBindBuffer(GL_ARRAY_BUFFER, 0);
glBindVertexArray(0);


// texture ground base color
glGenTextures(1, &tex_ground_color);
glBindTexture(GL_TEXTURE_2D, tex_ground_color);

if( (t = IMG_Load("../Textures/sol1/rock_sliced_Base_Color.png")) != NULL ) {
  //if( (t = IMG_Load("../Textures/sol2/rockface_Base_Color.png")) != NULL ) {
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->pixels);
  SDL_FreeSurface(t);
} else {
  fprintf(stderr, "Erreur lors du chargement de la texture\n");
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
}


glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso); // anisotropie

glGenerateMipmap(GL_TEXTURE_2D);
glBindTexture(GL_TEXTURE_2D, 0);


// texture ground normal
glGenTextures(1, &tex_ground_AO);
glBindTexture(GL_TEXTURE_2D, tex_ground_AO);
  
if( (t = IMG_Load("../Textures/sol1/rock_sliced_Ambient_Occlusion.png")) != NULL ) {
  //if( (t = IMG_Load("../Textures/sol2/rockface_Ambient_Occlusion.png")) != NULL ) {
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels);
  SDL_FreeSurface(t);
} else {
  fprintf(stderr, "Erreur lors du chargement de la texture\n");
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
}


glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso); // anisotropie

glGenerateMipmap(GL_TEXTURE_2D);
glBindTexture(GL_TEXTURE_2D, 0);


// texture ground normal
glGenTextures(1, &tex_ground_normal);
glBindTexture(GL_TEXTURE_2D, tex_ground_normal);
  
if( (t = IMG_Load("../Textures/sol1/rock_sliced_Normal.png")) != NULL ) {
  //if( (t = IMG_Load("../Textures/sol2/rockface_Normal.png")) != NULL ) {
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->pixels);
  SDL_FreeSurface(t);
} else {
  fprintf(stderr, "Erreur lors du chargement de la texture\n");
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
}

glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso); // anisotropie

glGenerateMipmap(GL_TEXTURE_2D);
glBindTexture(GL_TEXTURE_2D, 0);



// TEX PARTICULE
 glGenTextures(1, &tex_particule);
  glBindTexture(GL_TEXTURE_2D, tex_particule);
 
  t = IMG_Load("../Textures/particle_atlas2.png");


  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->pixels);
  

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso); // anisotropie


  glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, 0);


  // TEX DEPTH MAP FEU
  //std::cout << "RATIO = " <<  ((float)h/(float)w)  << std::endl;
  depth_map_res_x = depth_map_res_seed;
  depth_map_res_y = depth_map_res_x * ((float)h/(float)w);
  //std::cout << "res y = " <<  depth_map_res_y  << std::endl;

  glGenTextures(1, &tex_depth_feu);
  glBindTexture(GL_TEXTURE_2D, tex_depth_feu);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, depth_map_res_x, depth_map_res_y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, /*GL_NEAREST*/ GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, /*GL_NEAREST*/GL_LINEAR);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  //GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
  //glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
  //glBindTexture(GL_TEXTURE_2D, 0);

  // attach tex au FBO 
  glBindFramebuffer(GL_FRAMEBUFFER, feu_depth_FBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex_depth_feu, 0);
  //glDrawBuffer(GL_NONE);
  //glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // TEX DEPTH MAP PARTICLE
  depth_map_res_x = depth_map_res_seed;
  depth_map_res_y = depth_map_res_x * ((float)h/(float)w);
 
  glGenTextures(1, &tex_depth_particle);
  glBindTexture(GL_TEXTURE_2D, tex_depth_particle);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, depth_map_res_x, depth_map_res_y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, /*GL_NEAREST*/ GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, /*GL_NEAREST*/GL_LINEAR);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  //GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
  //glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
  //glBindTexture(GL_TEXTURE_2D, 0);

  // attach tex au FBO 
  glBindFramebuffer(GL_FRAMEBUFFER, particle_depth_FBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex_depth_particle, 0);
  //glDrawBuffer(GL_NONE);
  //glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

   

  // TEX PRE RENDU PARTICLES
  depth_map_res_x = depth_map_res_seed;
  depth_map_res_y = depth_map_res_x * ((float)h/(float)w);

  glGenTextures(1, &tex_pre_rendu_feu);
  glBindTexture(GL_TEXTURE_2D, tex_pre_rendu_feu);  
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, depth_map_res_x, depth_map_res_y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR /*GL_NEAREST*/);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR /*GL_NEAREST*/);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glBindTexture(GL_TEXTURE_2D, 0);

  // attach la tex au FBO
  glBindFramebuffer(GL_FRAMEBUFFER, particle_render_FBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_pre_rendu_feu, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  
  GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (Status != GL_FRAMEBUFFER_COMPLETE) {
    printf("FBO BUUUG, status: 0x%x\n", Status);
  } 
  glBindFramebuffer(GL_FRAMEBUFFER, 0);


  // TEX REFLECTION CUBEMAP
  glGenTextures(1, &reflection_cubeMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, reflection_cubeMap);
  for (int i = 0; i < 6; ++i)
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, reflection_cubeMap_res, reflection_cubeMap_res, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  /*glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


  // RBO & FBO attach
  glBindFramebuffer(GL_FRAMEBUFFER, reflection_cubeMap_FBO);
  glBindRenderbuffer(GL_RENDERBUFFER, reflection_cubeMap_RBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, reflection_cubeMap_res, reflection_cubeMap_res);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, reflection_cubeMap_RBO);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  
  Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (Status != GL_FRAMEBUFFER_COMPLETE) {
    printf("FBO BUUUG, status: 0x%x\n", Status);
  } 
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    // TEX DEPTH MAP FEU 2
  glGenTextures(1, &tex_depth_feu2);
  glBindTexture(GL_TEXTURE_2D, tex_depth_feu2);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, reflection_cubeMap_res, reflection_cubeMap_res, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, /*GL_NEAREST*/ GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, /*GL_NEAREST*/GL_LINEAR);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  //GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
  //glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
  //glBindTexture(GL_TEXTURE_2D, 0);


  // TEX DEPTH MAP PARTICLE 2
  glGenTextures(1, &tex_depth_particle2);
  glBindTexture(GL_TEXTURE_2D, tex_depth_particle2);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, reflection_cubeMap_res, reflection_cubeMap_res, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, /*GL_NEAREST*/ GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, /*GL_NEAREST*/GL_LINEAR);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  //GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
  //glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
  //glBindTexture(GL_TEXTURE_2D, 0);


  // TEX PRE RENDU PARTICLES 2
  glGenTextures(1, &tex_pre_rendu_feu2);
  glBindTexture(GL_TEXTURE_2D, tex_pre_rendu_feu2);  
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, reflection_cubeMap_res, reflection_cubeMap_res, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR /*GL_NEAREST*/);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR /*GL_NEAREST*/);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glBindTexture(GL_TEXTURE_2D, 0);


 // TEX DEPTH MAP HOUSE
  depth_map_res_x_house = depth_map_res_seed * 6;
  depth_map_res_y_house = depth_map_res_x_house * ((float)h/(float)w);

  glGenTextures(1, &tex_depth_house);
  glBindTexture(GL_TEXTURE_2D, tex_depth_house);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, depth_map_res_x_house, depth_map_res_y_house, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, /*GL_NEAREST*/ GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, /*GL_NEAREST*/ GL_LINEAR);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  //GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
  //glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
  
  // attach tex au FBO 
  glBindFramebuffer(GL_FRAMEBUFFER, house_depth_FBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex_depth_house, 0);
  //glDrawBuffer(GL_NONE);
  //glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);



  ///////////// TEX COLOR VL 
  tex_VL_res_x = tex_VL_res_seed;  
  tex_VL_res_y = tex_VL_res_x * ((float)h/(float)w);

  tex_VL_res_x = w;
  tex_VL_res_y = h;
  //tex_VL_res_y = tex_VL_res_x * ((float)h/(float)w);

  for(int i = 0; i < 1; i++){
  glGenTextures(1, &tex_color_VL[i]);
  //glBindTexture(GL_TEXTURE_2D, tex_color_VL[i]);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex_color_VL[i]);

  //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_VL_res_x, tex_VL_res_y, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4 , GL_RGBA, tex_VL_res_x, tex_VL_res_y, GL_TRUE);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  
  /*glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);*/
 
  // RBO & FBO attach
  glBindFramebuffer(GL_FRAMEBUFFER, VL_FBO[i]);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tex_color_VL[i], 0);
  //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D , tex_color_VL[i], 0);
  glBindRenderbuffer(GL_RENDERBUFFER, VL_RBO[i]);
  //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, tex_VL_res_x, tex_VL_res_y);
  glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, tex_VL_res_x, tex_VL_res_y); 
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT /*GL_DEPTH_STENCIL_ATTACHMENT*/, GL_RENDERBUFFER, VL_RBO[i]);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  

  Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (Status != GL_FRAMEBUFFER_COMPLETE) {
    printf("FBO BUUUG, status: 0x%x\n", Status);
  } 
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  //glBindTexture(GL_TEXTURE_2D, 0);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
  }

  ///////////// TEX FINLA OUTPUT
  
  tex_VL_res_x = w;
  tex_VL_res_y = h;
  
  for(int i = 0; i < 2; i++){
  glGenTextures(1, &tex_final_color[i]);
  glBindTexture(GL_TEXTURE_2D, tex_final_color[i]);
  
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_VL_res_x, tex_VL_res_y, 0, GL_RGBA, GL_FLOAT, NULL);
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  
  /*glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);*/
 
  // RBO & FBO attach
  glBindFramebuffer(GL_FRAMEBUFFER, final_FBO[i]);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D , tex_final_color[i], 0);
  glBindRenderbuffer(GL_RENDERBUFFER, final_RBO[i]);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, tex_VL_res_x, tex_VL_res_y);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT , GL_RENDERBUFFER, final_RBO[i]);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  

  Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (Status != GL_FRAMEBUFFER_COMPLETE) {
    printf("FBO BUUUG, status: 0x%x\n", Status);
  } 
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  }


  ///// TEX SHADOW CUBEMAP
  glGenTextures(1, &tex_shadow_cubeMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, tex_shadow_cubeMap);
  for (int i = 0; i < 6; ++i)
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, reflection_cubeMap_res * 2.0, reflection_cubeMap_res * 2.0, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  
  Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (Status != GL_FRAMEBUFFER_COMPLETE) {
    printf("FBO BUUUG, status: 0x%x\n", Status);
  } 
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

  // TEX & FBO PING PONG 1
  glGenFramebuffers(1, &pingpongFBO[0]);
  glGenRenderbuffers(1, &pingpongRBO[0]); // RBO du FBO
  glGenTextures(1, &pingpongColorbuffers[0]);
  
  //glGenFramebuffers(2, pingpongFBO);
  //glGenRenderbuffers(2, pingpongRBO);
  //glGenTextures(2, pingpongColorbuffers);
  //for (GLuint i = 0; i < 2; i++){
    glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[0]);
    glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, reflection_cubeMap_res, reflection_cubeMap_res, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[0], 0);
    // Also check if framebuffers are complete (no need for depth buffer)
 
    glBindRenderbuffer(GL_RENDERBUFFER, pingpongRBO[0]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, reflection_cubeMap_res, reflection_cubeMap_res);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pingpongRBO[0]);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      std::cout << "Framebuffer not complete!" << std::endl;
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  //}

  // TEX & FBO PING PONG 2
    glGenFramebuffers(1, &pingpongFBO[1]);
  glGenRenderbuffers(1, &pingpongRBO[1]); // RBO du FBO
  glGenTextures(1, &pingpongColorbuffers[1]);
  
  //glGenFramebuffers(2, pingpongFBO);
  //glGenRenderbuffers(2, pingpongRBO);
  //glGenTextures(2, pingpongColorbuffers);
  //for (GLuint i = 0; i < 2; i++){
    glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[1]);
    glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, reflection_cubeMap_res, reflection_cubeMap_res, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[1], 0);
    // Also check if framebuffers are complete (no need for depth buffer)
 
    glBindRenderbuffer(GL_RENDERBUFFER, pingpongRBO[1]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, reflection_cubeMap_res, reflection_cubeMap_res);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pingpongRBO[1]);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      std::cout << "Framebuffer not complete!" << std::endl;
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  //}
  
  // OLD PING PONG INIT
  /*glGenFramebuffers(2, pingpongFBO);
  glGenRenderbuffers(2, pingpongRBO);
  glGenTextures(2, pingpongColorbuffers);
  for (GLuint i = 0; i < 2; i++)
  {
    glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
    glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, reflection_cubeMap_res, reflection_cubeMap_res, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
    // Also check if framebuffers are complete (no need for depth buffer)
 
    glBindRenderbuffer(GL_RENDERBUFFER, pingpongRBO[i]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, reflection_cubeMap_res, reflection_cubeMap_res);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pingpongRBO[i]);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      std::cout << "Framebuffer not complete!" << std::endl;
    //glBindRenderbuffer(GL_RENDERBUFFER, 0);
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  }*/

//////////////////////////////
// LIGHT INIT
lights = new light[3];

// sun
lights[0].lightColor = glm::vec3(1.0,1.0,1.0);
lights[0].lightSpecularColor = glm::vec3(1.0,1.0,1.0);
lights[0].lightPos = glm::vec3(29.8, 135, -81);
lights[0].lightColor*= 3.0;
lights[0].lightSpecularColor*= 3.0;
 
// fire
lights[1].lightColor = glm::vec3(255.0/255.0, (/*147.0*/(197)/255.0), ((143)/255.0));
lights[1].lightSpecularColor = glm::vec3(/*1.0,1.0,1.0*/ 255.0/255.0, ((200.0)/255.0), ((180.0)/255.0)) * 1.0f;
lights[1].lightPos = glm::vec3(particules_pos.x,particules_pos.y + 0.11f,particules_pos.z);
lights[1].save_lightPos = lights[1].lightPos;

// fake light pos use for shadow house && VL;
lights[2].lightPos = glm::vec3(-0.0680922, 11.9514, -5.3171);

// OBJECTS INIT
ground = new objet[1];
trees = new objet[nb_tree];
rocks = new objet[nb_rock];
/////////////////////

  
ground->AmbientStr = 0.1;
ground->DiffuseStr = 0.4;
ground->SpecularStr = 0.15;
ground->ShiniStr = 4; // 4 8 16 ... 256 
 
ground->angle=0.01;
ground->acca=0.0;
ground->var=0.0;
ground->scale=100.0;
ground->alpha=1.0;

ground->x=0.0;
ground->y=0.0;
ground->z=0.0; 

ground->start=0.0;
ground->dt=0.0;
ground->bouge=0;
ground->t=0.0;
ground->t0=0.0;
ground->shadow_darkness = 0.65;

////////////////////////
  
 Feu.AmbientStr = 0.3;
 Feu.DiffuseStr = 0.55;
 Feu.SpecularStr = 0.0;
 Feu.ShiniStr = 1.0; // 4 8 16 ... 256 
 Feu.angle=0.5;
 Feu.acca=0.0;
 Feu.var=4.0;
 Feu.scale=0.05 /*2.0*/;

 Feu.alpha = 1.0;

 Feu.x=particules_pos.x /*+ 1.0*/;
 Feu.y= particules_pos.y - 0.11;
 Feu.z=particules_pos.z;

 Feu.start=0.0;
 Feu.dt=0.0;
 Feu.bouge=0;
 Feu.t=0.0;
 Feu.t0=0.0;
 Feu.shadow_darkness = 0.75;

////////////////////////
  
 paladin.AmbientStr = 0.25;
 paladin.DiffuseStr = 0.8;
 paladin.SpecularStr = 0.2;
 paladin.ShiniStr = 256.0; // 4 8 16 ... 256 
 paladin.angle = 3.635;
 paladin.acca=0.105;
 paladin.var=1.0;
 paladin.scale=0.0055 /*2.0*/;

 paladin.alpha = 1.0;

 paladin.x = -0.77;
 paladin.y = 4.906;
 paladin.z = 0.97;
 
 paladin.start=0.0;
 paladin.dt=0.0;
 paladin.bouge=0;
 paladin.t=0.0;
 paladin.t0=0.0;
 paladin.shadow_darkness = 0.75;

 //////////////////////
 
 house.AmbientStr = 0.5;
 house.DiffuseStr = 0.3;
 house.SpecularStr = 0.2;
 house.ShiniStr = 8; // 4 8 16 ... 256 
 house.angle=2.47;
 house.acca=0.105;
 house.var=2.0;
 house.scale = 0.02;

 house.alpha = 1.0;

 house.x = -0.8;
 house.y = 4.881;
 house.z = -0.3;
 
 house.start=0.0;
 house.dt=0.0;
 house.bouge=0;
 house.t=0.0;
 house.t0=0.0;

 house.shadow_darkness = 0.75;

 /////////////////////
 
 sword.AmbientStr = 0.6;
 sword.DiffuseStr = 0.6;
 sword.SpecularStr = 0.2;
 sword.ShiniStr = 8; // 4 8 16 ... 256 
 sword.angle=-0.2;
 sword.acca=0.105;
 sword.var=3.0;
 sword.scale = 0.011;

 sword.alpha = 1.0;

 sword.x = 0.2179;
 sword.y = 5.009;
 sword.z = -5.631;
 
 sword.start=0.0;
 sword.dt=0.0;
 sword.bouge=0;
 sword.t=0.0;
 sword.t0=0.0;

 sword.shadow_darkness = 0.75;

 /////////////////////

 shield.AmbientStr = 0.6;
 shield.DiffuseStr = 0.8;
 shield.SpecularStr = 0.8;
 shield.ShiniStr = 16; // 4 8 16 ... 256 
 shield.angle=0.67;
 shield.acca=0.105;
 shield.var=5.0;
 shield.scale = 0.004;

 shield.alpha = 1.0;

 shield.x = -1.67;
 shield.y = 5.04;
 shield.z = -1.03;
 
 shield.start=0.0;
 shield.dt=0.0;
 shield.bouge=0;
 shield.t=0.0;
 shield.t0=0.0;

 shield.shadow_darkness = 0.75;

 ///////////////////////
 
 for(int i = 0; i < nb_tree; i++){
 
  trees[i].AmbientStr = 0.5;
  trees[i].DiffuseStr = 0.5;
  trees[i].SpecularStr = 0.3;
  trees[i].ShiniStr = 4; // 4 8 16 ... 256 
  trees[i].angle=0.67;
  trees[i].acca=0.105;
  trees[i].var=6.0;
  trees[i].scale = 0.0009;

  trees[i].alpha = 1.0;

  trees[i].x = 2.0;
  trees[i].y = 6.0;
  trees[i].z = 0.0;

  trees[i].start=0.0;
  trees[i].dt=0.0;
  trees[i].bouge=0;
  trees[i].t=0.0;
  trees[i].t0=0.0;

  trees[i].shadow_darkness = 0.75;
 
 }

 ////////////////////////
 
 for(int i = 0; i < nb_rock; i++){
 
  rocks[i].AmbientStr = 0.5;
  rocks[i].DiffuseStr = 0.5;
  rocks[i].SpecularStr = 0.3;
  rocks[i].ShiniStr = 4; // 4 8 16 ... 256 
  rocks[i].angle=0.67;
  rocks[i].acca=0.105;
  rocks[i].var=7.0;
  rocks[i].scale = 1.0;

  rocks[i].alpha = 1.0;

  rocks[i].x = 2.0;
  rocks[i].y = 6.0;
  rocks[i].z = 0.0;

  rocks[i].start=0.0;
  rocks[i].dt=0.0;
  rocks[i].bouge=0;
  rocks[i].t=0.0;
  rocks[i].t0=0.0;

  rocks[i].shadow_darkness = 0.75;
 
 }

}


 static void resizeGL(SDL_Window * win) {

  SDL_GetWindowSize(win, &w, &h);
  //glViewport(0, 0, w, h);

  std::cout << "W = " << w << ", H = " << h << std::endl;

  SDL_WarpMouseInWindow(win,w/2.0,h/2.0);

}


 static void loop(SDL_Window * win) {


  /*Uint32 t;
  t = SDL_GetTicks();
*/

  SDL_GL_SetSwapInterval(1);

  
  for(;;) {


    manageEvents(win);

    mobile_move(ground,1);

    draw();
    printFPS();

    Particles->Update(ground->dt*-1.0f, 0.01, glm::vec2(0.0,0.0));

    fire_script();

    if(script_on)
      camera_script();

    SDL_GL_SwapWindow(win);


  /*  printf("1cameraX = %f, cameraY = %f, cameraZ = %f\n",cameraPos.x,cameraPos.y, cameraPos.z); 
    printf("2cameraX = %f, cameraY = %f, cameraZ = %f\n",cameraFront.x,cameraFront.y, cameraFront.z);
    printf("yaw = %f, pitch = %f\n", yaw, pitch); */

    //printf("lightX = %f, Y = %f, Z = %f\n", lights[1].lightPos.x,  lights[1].lightPos.y,  lights[1].lightPos.z);
   
    //lights[1].lightPos = glm::vec3(particules_pos.x,particules_pos.y + 0.08f,particules_pos.z);
    /*Feu.x = particules_pos.x;
    Feu.y = particules_pos.y - 0.11;
    Feu.z = particules_pos.z;*/

    }

  }

void audio_script(int step, double acc){


  /*if(!Mix_PlayingMusic())
    Mix_FadeInMusic(S_main_music, 1, 8000);*/
  
  if(step == 0 && acc < 0.5){
    if(!Mix_Playing(0)){   
      Mix_Volume(0,MIX_MAX_VOLUME/6);
      Mix_FadeInChannel(0, S_wind_lowland, 0, 4000);
    }
  }else{
    Mix_FadeOutChannel(0,5000);
    if(!Mix_PlayingMusic() && step == 0)
      Mix_FadeInMusic(S_main_music, 1, 6000);    
  }  

  if(step == 8 && acc < 0.4){
    if(!Mix_Playing(1)){   
      Mix_Volume(1,MIX_MAX_VOLUME/3);
      Mix_FadeInChannel(1, S_fire, 0, 100);
    }
  }else{
    Mix_FadeOutChannel(1,1000);
  }


}


void camera_script(){

  static float speed = 0.073;


  static double acc = 0.0;
  glm::vec3 res;
  glm::vec3 tab_bezier_quad_coord[50][4];
  tab_bezier_quad_coord[0][0] = glm::vec3(23.73,6.58,-23.01);
  tab_bezier_quad_coord[0][1] = glm::vec3(16.58,5.54,-25.92);
  tab_bezier_quad_coord[0][2] = glm::vec3(6.26,5.76,-28.14);
  tab_bezier_quad_coord[0][3] = glm::vec3(-10.12,5.28,-24.26);
  tab_bezier_quad_coord[1][0] = glm::vec3(179,-59,0.0);
  tab_bezier_quad_coord[1][1] = glm::vec3(201,-36,-0.26);
  tab_bezier_quad_coord[1][2] = glm::vec3(190,-20,0.0);
  tab_bezier_quad_coord[1][3] = glm::vec3(183,-26,0.0);

  tab_bezier_quad_coord[2][0] = glm::vec3(-8.34,5.32,-22.98);
  tab_bezier_quad_coord[2][1] = glm::vec3(-13.72,5.74,-16.76);
  tab_bezier_quad_coord[2][2] = glm::vec3(-3.71,5.72,-14.18);
  tab_bezier_quad_coord[2][3] = glm::vec3(-0.58,5.6,-6.82);
  tab_bezier_quad_coord[3][0] = glm::vec3(-259,-36,-23.01);
  tab_bezier_quad_coord[3][1] = glm::vec3(-286,-34,-25.92);
  tab_bezier_quad_coord[3][2] = glm::vec3(-317,-9,-28.14);
  tab_bezier_quad_coord[3][3] = glm::vec3(-314,-27,-24.26);

  tab_bezier_quad_coord[4][0] = glm::vec3(-0.48,5.49,-5.1);
  tab_bezier_quad_coord[4][1] = glm::vec3(-0.55,5.41,-4.64);
  tab_bezier_quad_coord[4][2] = glm::vec3(1.78,5.55,-4.36);
  tab_bezier_quad_coord[4][3] = glm::vec3(0.67,5.44,-5.9);
  tab_bezier_quad_coord[5][0] = glm::vec3(-797,-18,-23.01);
  tab_bezier_quad_coord[5][1] = glm::vec3(-790,-7,-25.92);
  tab_bezier_quad_coord[5][2] = glm::vec3(-864, -10,-28.14);
  tab_bezier_quad_coord[5][3] = glm::vec3(-940,-19,-24.26);

  tab_bezier_quad_coord[6][0] = glm::vec3(-4.61,6.09,0.27);
  tab_bezier_quad_coord[6][1] = glm::vec3(-2.27,9.21,0.42);
  tab_bezier_quad_coord[6][2] = glm::vec3(2.47,8.35,-1.22);
  tab_bezier_quad_coord[6][3] = glm::vec3(0.55,5.52,-1.65);
  tab_bezier_quad_coord[7][0] = glm::vec3(-1081,-11,-23.01);
  tab_bezier_quad_coord[7][1] = glm::vec3(-1106,-58,-25.92);
  tab_bezier_quad_coord[7][2] = glm::vec3(-1259, -59,-28.14);
  tab_bezier_quad_coord[7][3] = glm::vec3(-1263,-57,-24.26);

  tab_bezier_quad_coord[8][0] = glm::vec3(-1.30,5.12,-0.82);
  tab_bezier_quad_coord[8][1] = glm::vec3(-1.25,5.16,-0.77);
  tab_bezier_quad_coord[8][2] = glm::vec3(-1.46,5.08,-0.61);
  tab_bezier_quad_coord[8][3] = glm::vec3(-1.71,5.06,-0.43);
  tab_bezier_quad_coord[9][0] = glm::vec3(-1172,-36,-23.01);
  tab_bezier_quad_coord[9][1] = glm::vec3(-1229,-14,-25.92);
  tab_bezier_quad_coord[9][2] = glm::vec3(-1203,-10,-28.14);
  tab_bezier_quad_coord[9][3] = glm::vec3(-1172,-11,-24.26);

  tab_bezier_quad_coord[10][0] = glm::vec3(-1.79,5.22,0.27);
  tab_bezier_quad_coord[10][1] = glm::vec3(-1.75,5.41,1.05);
  tab_bezier_quad_coord[10][2] = glm::vec3(-1.07,5.38,1.008);
  tab_bezier_quad_coord[10][3] = glm::vec3(-0.73,5.27,0.40);
  tab_bezier_quad_coord[11][0] = glm::vec3(330,-31,-23.01);
  tab_bezier_quad_coord[11][1] = glm::vec3(282,-26,-25.92);
  tab_bezier_quad_coord[11][2] = glm::vec3(244,-27,-28.14);
  tab_bezier_quad_coord[11][3] = glm::vec3(215,-31,-24.26);

  tab_bezier_quad_coord[12][0] = glm::vec3(-0.52,5.24,0.26);
  tab_bezier_quad_coord[12][1] = glm::vec3(-0.88,5.35,0.186);
  tab_bezier_quad_coord[12][2] = glm::vec3(-1.23,5.36,0.41);
  tab_bezier_quad_coord[12][3] = glm::vec3(-1.08,5.3,0.76);
  tab_bezier_quad_coord[13][0] = glm::vec3(57,-38,-23.01);
  tab_bezier_quad_coord[13][1] = glm::vec3(72,-22,-25.92);
  tab_bezier_quad_coord[13][2] = glm::vec3(47,-23,-28.14);
  tab_bezier_quad_coord[13][3] = glm::vec3(22,-9.84,-24.26);

  tab_bezier_quad_coord[14][0] = glm::vec3(-1.39,5.39,1.11);
  tab_bezier_quad_coord[14][1] = glm::vec3(-0.90,5.51*0.95,1.72);
  tab_bezier_quad_coord[14][2] = glm::vec3(-0.25,5.61*0.95,1.26);
  tab_bezier_quad_coord[14][3] = glm::vec3(-0.27,5.33,0.003);
  tab_bezier_quad_coord[15][0] = glm::vec3(-30,-27,-23.01);
  tab_bezier_quad_coord[15][1] = glm::vec3(-133,-33,-25.92);
  tab_bezier_quad_coord[15][2] = glm::vec3(-120,-30,-28.14);
  tab_bezier_quad_coord[15][3] = glm::vec3(-211,-22,-24.26);

  tab_bezier_quad_coord[16][0] = glm::vec3(-0.14,6.3,-0.49);
  tab_bezier_quad_coord[16][1] = glm::vec3(-0.46,5.89,-0.15);
  tab_bezier_quad_coord[16][2] = glm::vec3(-0.80,5.5,0.19);
  tab_bezier_quad_coord[16][3] = glm::vec3(-1.3,5.3,0.56);
  tab_bezier_quad_coord[17][0] = glm::vec3(467,-41,-23.01);
  tab_bezier_quad_coord[17][1] = glm::vec3(464,-30,-25.92);
  tab_bezier_quad_coord[17][2] = glm::vec3(446,-15,-28.14);
  tab_bezier_quad_coord[17][3] = glm::vec3(386,35,-24.26);

  tab_bezier_quad_coord[18][0] = glm::vec3(-0.67,5.56,0.06);
  tab_bezier_quad_coord[18][1] = glm::vec3(-1.08,5.6,-0.09);
  tab_bezier_quad_coord[18][2] = glm::vec3(-1.43,5.6,0.3);
  tab_bezier_quad_coord[18][3] = glm::vec3(-1.45,5.6,0.8);
  tab_bezier_quad_coord[19][0] = glm::vec3(105,-10,-23.01);
  tab_bezier_quad_coord[19][1] = glm::vec3(75,-9,-25.92);
  tab_bezier_quad_coord[19][2] = glm::vec3(42,-2.3,-28.14);
  tab_bezier_quad_coord[19][3] = glm::vec3(20,-7.3,-24.26);

  tab_bezier_quad_coord[20][0] = glm::vec3(-0.46,5.18,1.03);
  tab_bezier_quad_coord[20][1] = glm::vec3(-0.03,5.18,-0.1);
  tab_bezier_quad_coord[20][2] = glm::vec3(0.38,5.14,-0.05);
  tab_bezier_quad_coord[20][3] = glm::vec3(0.89,5.11,-0.64);
  tab_bezier_quad_coord[21][0] = glm::vec3(-112,7,-23.01);
  tab_bezier_quad_coord[21][1] = glm::vec3(-132,-0.7,-25.92);
  tab_bezier_quad_coord[21][2] = glm::vec3(-100,-9,-28.14);
  tab_bezier_quad_coord[21][3] = glm::vec3(-144,-11,-24.26);

  tab_bezier_quad_coord[22][0] = glm::vec3(-1.17,5.4,-1.15);
  tab_bezier_quad_coord[22][1] = glm::vec3(-0.75,5.50,-1.7);
  tab_bezier_quad_coord[22][2] = glm::vec3(0.36,5.26,-1.47);
  tab_bezier_quad_coord[22][3] = glm::vec3(0.025,5.24,0.15);
  tab_bezier_quad_coord[23][0] = glm::vec3(-315,-27,-23.01);
  tab_bezier_quad_coord[23][1] = glm::vec3(-282,-22,-25.92);
  tab_bezier_quad_coord[23][2] = glm::vec3(-230,-11,-28.14);
  tab_bezier_quad_coord[23][3] = glm::vec3(-128,-4,-24.26);

  tab_bezier_quad_coord[24][0] = glm::vec3(-2.11,5.66,0.44);
  tab_bezier_quad_coord[24][1] = glm::vec3(-1.17,6.00,0.29);
  tab_bezier_quad_coord[24][2] = glm::vec3(-0.57,6.15,-0.49);
  tab_bezier_quad_coord[24][3] = glm::vec3(-0.70,5.89,-1.30);
  tab_bezier_quad_coord[25][0] = glm::vec3(-83,54,-23.01);
  tab_bezier_quad_coord[25][1] = glm::vec3(-83,46,-25.92);
  tab_bezier_quad_coord[25][2] = glm::vec3(-90,49,-28.14);
  tab_bezier_quad_coord[25][3] = glm::vec3(-93,59.89,-24.26);


  /////////////////////////

  if(acc < 1.0){
    acc += ground->dt * -speed;
    //std::cout << "acc = " << acc << std::endl;
  }else{
    if(step < 24){
      step +=2;
      acc = 0.0;
    }

    if(step == 2)
      speed = 0.073;
    
    if(step == 4)
      speed = 0.15;

    if(step == 6)
      speed = 0.15;

    if(step == 8){
      speed = 0.09;
      shadow_point_light = 1.0;
    }
 
    if(step == 10){
      speed = 0.08;
    }

    if(step == 12){
      speed = 0.1;
    }

    if(step == 14){
      speed = 0.1;
    }

    if(step == 16){
      speed = 0.13;
    }

    if(step == 18){
      speed = 0.06;
    }

    if(step == 20){
      speed = 0.1;
    }

    if(step == 22){
      speed = 0.1;
    }

    if(step == 24){
      speed = 0.075;
    }


  }

  if(step >= 20 && (VL_intensity < VL_intensity_max)){
    VL_intensity += ground->dt * -1.0 * 0.25;
  }

  if(step == 24 && acc > 0.99){
    //if(output_factor > 0.0)
    output_factor += ground->dt * -1.0 * 0.5;
    Mix_FadeOutMusic(5000);
  }

  //std::cout << "VL_intensity = " << VL_intensity << std::endl;
  //std::cout << "step = " << step << std::endl;


  res.x = bezier(tab_bezier_quad_coord[step][0].x,tab_bezier_quad_coord[step][1].x,tab_bezier_quad_coord[step][2].x,tab_bezier_quad_coord[step][3].x, acc);
  res.y = bezier(tab_bezier_quad_coord[step][0].y,tab_bezier_quad_coord[step][1].y,tab_bezier_quad_coord[step][2].y,tab_bezier_quad_coord[step][3].y, acc);
  res.z = bezier(tab_bezier_quad_coord[step][0].z,tab_bezier_quad_coord[step][1].z,tab_bezier_quad_coord[step][2].z,tab_bezier_quad_coord[step][3].z, acc);

  yaw = bezier(tab_bezier_quad_coord[step+1][0].x,tab_bezier_quad_coord[step+1][1].x,tab_bezier_quad_coord[step+1][2].x,tab_bezier_quad_coord[step+1][3].x, acc);
  pitch = bezier(tab_bezier_quad_coord[step+1][0].y,tab_bezier_quad_coord[step+1][1].y,tab_bezier_quad_coord[step+1][2].y,tab_bezier_quad_coord[step+1][3].y, acc);
  
  cameraPos = res;

  audio_script(step, acc);

}


double bezier(double A,  // Start value
              double B,  // First control value
              double C,  // Second control value
              double D,  // Ending value
              double t)  // Parameter 0 <= t <= 1
{
    /*double s = 1 - t;
    double AB = A*s + B*t;
    double BC = B*s + C*t;
    double CD = C*s + D*t;
    double ABC = AB*s + BC*t;
    double BCD = BC*s + CD*t;
    return ABC*s + BCD*t;*/
     double s = 1 - t;
    double AB = A*s + B*t;
    double BC = B*s + C*t;
    double CD = C*s + D*t;
    double ABC = AB*s + CD*t;
    double BCD = BC*s + CD*t;
    return ABC*s + BCD*t;
}


/*!\brief Cette fonction permet de gérer les évènements clavier et
 * souris via la bibliothèque SDL et pour la fenêtre pointée par \a
 * win.
 *
 * \param win le pointeur vers la fenêtre SDL pour laquelle nous avons
 * attaché le contexte OpenGL.
 */
 static void manageEvents(SDL_Window * win) {

  SDL_Event event;
  glm::vec3 front;


  GLfloat camera_speed = walk_speed*20;


  if(fly_state == true)
    camera_speed = walk_speed*10;

  //printf("camera_speed = %f\n", camera_speed);


  if(cameraZ == 1){

     cameraPos -= (camera_speed*ground->dt) * cameraFront;

   }

   if(cameraS == 1){

     cameraPos += (camera_speed*ground->dt) * cameraFront;

   }

   if(cameraQ == 1){

     cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * (camera_speed*ground->dt);

   }

   if(cameraD == 1){

     cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * (camera_speed*ground->dt); 

   }



//////////////////////////////////////

/*if(cameraM == 1){

  yaw -= 80.0*ground->dt;
  front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch)) ;
  front.y = sin(glm::radians(pitch));
  front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  cameraFront = glm::normalize(front); 

}

if(cameraK == 1){
  yaw += 80.0*ground->dt;
  front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  front.y = sin(glm::radians(pitch));
  front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  cameraFront = glm::normalize(front);

}

if(cameraO == 1){
  if(pitch < 89.0){
    pitch -= 80.0*ground->dt;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
  }
}

if(cameraL == 1){
  if(pitch > -89.0){
    pitch += 80.0*ground->dt;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
  }
}*/


while(SDL_PollEvent(&event))

  switch (event.type) {

    case SDL_KEYDOWN:

    switch(event.key.keysym.sym) {

     case SDLK_ESCAPE:
     printf("\ntime = %f\n", ground->t);
     std::cout << "x = " << cameraPos.x << ", y = " <<  cameraPos.y << ", z = " <<  cameraPos.z << std::endl;
     std::cout << "pos = " << trees[0].x << " " << trees[0].y << " " << trees[0].z << std::endl;
     std::cout << "angle = " << shield.angle << std::endl;
     exit(0);


     case 'z' :
     cameraZ = 1;
     break;

     case 'q' :
     cameraQ = 1;
     break;

     case 's' :
     cameraS = 1;
     break;

     case 'd' :
     cameraD=1;
     break;

     case 'a' :
     if(script_on){
      script_on = false;
     }else{
      script_on = true;
     }
    /* VL_intensity += 0.01;
     std::cout << "VL_intensity = " <<  VL_intensity << std::endl;*/
     break;

     case 'e' :
     VL_intensity += 0.01;
     std::cout << "VL_intensity = " <<  VL_intensity << std::endl;
     break;

     case 't' :
     if(shadow_point_light == 1){
      shadow_point_light = 0;
     }else{
      shadow_point_light = 1;
     }
     break;

     case 'y' :
     trees[0].z -= 0.2;
     break;

     case 'w' :
     VL_offset_factor += 0.01;
     std::cout << "blur offset = " <<  VL_offset_factor << std::endl;
     break;

     case 'x' :
     VL_offset_factor -= 0.01;
     std::cout << "blur offset = " <<  VL_offset_factor << std::endl;    
     break;

    /* case 'm' :

     cameraM = 1;

     break;

     case 'k' :

     cameraK = 1;

     break;

     case 'o' :
     cameraO = 1;
     break;

     case 'l' :
     cameraL = 1;
     break;*/

     
     case 'v' :
     //std::cout << "test = " <<  ((float)h/(float)w)  << std::endl ;

     if(walk_speed < 0.2)
      walk_speed = 0.22;
     else
       if(walk_speed >= 0.2)
        walk_speed = 0.1;     
     break;


    case 'r' :    
     if(fly_state == true)
      fly_state = false;
    else
     if(fly_state == false)
      fly_state = true;
    break;


    default:
    fprintf(stderr, "La touche %s a ete pressee\n",
      SDL_GetKeyName(event.key.keysym.sym));
    break;
  }
  break;

  case SDL_KEYUP:

  switch(event.key.keysym.sym) {

    case 'z' :
    cameraZ = 0;
    break;

    case 'q' :
    cameraQ = 0;
    break;

    case 's' :
    cameraS = 0;
    break;

    case 'd' :
    cameraD = 0;
    break;


    /*case 'm' :

    cameraM = 0;
    break;

    case 'k' :

    cameraK = 0;
    break;

    case 'o' :

    cameraO = 0;
    break;

    case 'l' :
    cameraL = 0;
    break;*/

  }
  break;




  case SDL_WINDOWEVENT:
  if(event.window.windowID == SDL_GetWindowID(win)) {
    switch (event.window.event)  {
      case SDL_WINDOWEVENT_RESIZED:
      SDL_GetWindowSize(win,&w,&h);
      resizeGL(win);    
      initData();
      
      //SDL_WarpMouseInWindow(_win,w/2.0,h/2.0);

      break;
      case SDL_WINDOWEVENT_CLOSE:
      event.type = SDL_QUIT;
      SDL_PushEvent(&event);
      break;
    }
  }
  break;
  case SDL_QUIT:
  exit(0);

}


/////////////////////////////////////// MOUSE

int x,y;                      

if(RUN_MODE == 0 || RUN_MODE == 2){

SDL_GetRelativeMouseState(&x,&y);

//printf("x = %d, y = %d\n", x, y);


    GLfloat xoffset = (float)x;
    GLfloat yoffset = (float)y*-1; // Reversed since y-coordinates go from bottom to left
    
    GLfloat sensitivity = 0.05; // Change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;


    yaw   += xoffset;
    pitch += yoffset;

     if (pitch > 89.0f)
        pitch = 89.0f;
      if (pitch < -89.0f)
        pitch = -89.0f;

      
      //glm::vec3 front;
      front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
      front.y = sin(glm::radians(pitch));
      front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
      cameraFront = glm::normalize(front);
    }

}


static void draw() {


  glm::mat4 projectionM,projectionM2, projectionM3,Msend,viewMatrix;

  projectionM = glm::perspective(45.0f, /* 4.0f/3.0f */(float)w/(float)h, 0.1f, 1000.0f); // rendu de base
  projectionM2 = glm::perspective(45.0f, (float)depth_map_res_x/(float)depth_map_res_y, 0.1f, 1000.0f); // pre rendu dans depth tex
  projectionM3 = glm::perspective(45.0f, (float)depth_map_res_x_house/(float)depth_map_res_y_house, 0.1f, 1000.0f); // pre rendu dans depth tex HOUSE
  
  
  viewMatrix=glm::lookAt(cameraPos, (cameraPos) + cameraFront, cameraUp); 


 ////////////////////////////
   

 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 



 //DRAW SCREEN
 /*glViewport(0, 0, w, h);
 screen_shader.Use();
 glActiveTexture(GL_TEXTURE0);
 glBindTexture(GL_TEXTURE_2D, tex_color_VL);

 glBindVertexArray(screenVAO);

 glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

 glBindVertexArray(0);
 glUseProgram(0);*/
 


 // pre rendu feu
 glViewport(0, 0, depth_map_res_x, depth_map_res_y);
 Pre_rendu_feu(projectionM2, viewMatrix, -1.0f);
 
 //pre rendu reflection cube map
 glViewport(0, 0, reflection_cubeMap_res, reflection_cubeMap_res);
 Pre_rendu_cubeMap();

 //pre rendu shadow
 glViewport(0, 0, depth_map_res_x_house, depth_map_res_y_house);
 Pre_rendu_shadow_house(projectionM3, viewMatrix);


 // pre rendu shadow cubemap
 if(shadow_point_light == 1){
   glViewport(0, 0, reflection_cubeMap_res * 2.0, reflection_cubeMap_res * 2.0);
   Pre_rendu_shadow_cubeMap();
 }

  // rendu scene pour la VL
 glViewport(0, 0, tex_VL_res_x, tex_VL_res_y);
 RenderShadowedObjects(true);


 glBindFramebuffer(GL_READ_FRAMEBUFFER, VL_FBO[0]);
 glBindFramebuffer(GL_DRAW_FRAMEBUFFER, final_FBO[0]);        
 glBlitFramebuffer(0, 0, tex_VL_res_x, tex_VL_res_y, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);     
 glBindFramebuffer(GL_FRAMEBUFFER, 0);   


 if(VL_intensity > 0.0){
   VL_blur_apply();
 }


// FINAL OUTPUT
 glViewport(0, 0, w, h);
 screen_shader.Use();
 glActiveTexture(GL_TEXTURE0);
 glBindTexture(GL_TEXTURE_2D, tex_final_color[0]);

 glBindVertexArray(screenVAO);

 glUniform1f(glGetUniformLocation(screen_shader.Program, "output_factor"), output_factor);
   
 glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

 glBindVertexArray(0);
 glUseProgram(0);


 // rendu scene normal        
/* glViewport(0, 0, w, h);
 RenderShadowedObjects(false);
*/

 glUseProgram(0);
        

}

void VL_blur_apply(){

  glBindFramebuffer(GL_FRAMEBUFFER, 0);   

  int horizontal = 1; 
  GLuint amount = 2;
  blur_shader.Use();
  for (GLuint i = 0; i < amount; i++){

   glBindFramebuffer(GL_FRAMEBUFFER, final_FBO[horizontal]); 
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   if(horizontal == 0){
    horizontal = 1;
   }else{ horizontal = 0; }

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, tex_final_color[horizontal]);  // bind texture of other framebuffer (or scene if first iteration)

   glUniform1f(glGetUniformLocation(blur_shader.Program, "horizontal"), horizontal);
   glUniform1f(glGetUniformLocation(blur_shader.Program, "offset_factor"), VL_offset_factor);
   glUniform1i(glGetUniformLocation(blur_shader.Program, "is_reflection_blur"), 0);

   RenderQuad();
   glBindFramebuffer(GL_FRAMEBUFFER, 0);     

 }
 glUseProgram(0);


}


void Pre_rendu_feu(glm::mat4 projectionMatrix, glm::mat4 viewMatrix, float face_cube){


  glm::mat4 projectionM = projectionMatrix;
  glm::mat4 viewM = viewMatrix; 

  glm:: mat4 Msend;


  /////////////////////////// DRAW DEPTH PARTICULES     
  glBindFramebuffer(GL_FRAMEBUFFER, particle_depth_FBO);
  if(face_cube != -1){
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex_depth_particle2, 0);
  }else{
     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex_depth_particle, 0);
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  particule_shader.Use();   

  Msend = glm::mat4(1.0);
  Msend = glm::translate(Msend, glm::vec3(particules_pos.x,particules_pos.y,particules_pos.z));     

  glUniformMatrix4fv(glGetUniformLocation(particule_shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projectionM));
  glUniformMatrix4fv(glGetUniformLocation(particule_shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(viewM));
  glUniformMatrix4fv(glGetUniformLocation(particule_shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(Msend));
  glUniform1f(glGetUniformLocation(particule_shader.Program, "test"), 2.0);
  glUniform1f(glGetUniformLocation(particule_shader.Program, "face_cube"), face_cube);


  Particles -> Draw(false,(face_cube != -1));

  glUseProgram(0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);        

  ////////////////////// DRAW DEPTH FEU DE CAMP
  glBindFramebuffer(GL_FRAMEBUFFER, feu_depth_FBO);
  if(face_cube != -1){
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex_depth_feu2, 0);
  }else{
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex_depth_feu, 0);
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  basic_shader.Use();

  Msend = glm::mat4();
  Msend = glm::translate(Msend, glm::vec3(Feu.x,Feu.y,Feu.z));
  Msend = glm::rotate(Msend, (float)myPI_2, glm::vec3(-1.0, 0.0 , 0.0));
  Msend = glm::scale(Msend, glm::vec3(Feu.scale)); 

  glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewM));
  glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(Msend));
  glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionM));

  glUniform1f(glGetUniformLocation(basic_shader.Program, "var"), Feu.var);     
  glUniform1f(glGetUniformLocation(basic_shader.Program, "depth_test"), 1.0);    
  glUniform1f(glGetUniformLocation(basic_shader.Program, "face_cube"), face_cube);     
  
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  feu_model.Draw(basic_shader, glm::mat4(1.0), false);  
  glDisable(GL_CULL_FACE);

  glBindVertexArray(0);
  glUseProgram(0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);        
  
  /////////////////////////// DRAW PARTICULES     
  glBindFramebuffer(GL_FRAMEBUFFER, particle_render_FBO);
  if(face_cube != -1){
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_pre_rendu_feu2, 0);
  }else{
     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_pre_rendu_feu, 0);
  }

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


  particule_shader.Use();   

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_particule);
  

  Msend = glm::mat4(1.0);
  Msend = glm::translate(Msend, glm::vec3(particules_pos.x,particules_pos.y,particules_pos.z));    

  glUniformMatrix4fv(glGetUniformLocation(particule_shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projectionM));
  glUniformMatrix4fv(glGetUniformLocation(particule_shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(viewM));
  glUniformMatrix4fv(glGetUniformLocation(particule_shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(Msend));
  glUniform1f(glGetUniformLocation(particule_shader.Program, "test"), 0.0);
  glUniform1f(glGetUniformLocation(particule_shader.Program, "face_cube"), face_cube);


  Particles -> Draw(true,(face_cube != -1));

  glUseProgram(0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);        
  glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

///////////////////////
  
  //glViewport(0, 0, w, h);

}

void Pre_rendu_cubeMap(){

  
  glm::mat4 projectionM,Msend;

  std::vector<glm::mat4> cubeMap_viewMatrices;
  projectionM = glm::perspective(89.5373f, (float)reflection_cubeMap_res/(float)reflection_cubeMap_res, 0.1f, 1000.0f);
  glm::vec3 paladin_pos = glm::vec3(paladin.x,paladin.y+0.45,paladin.z);
 
  
  /////////////

  /*cubeMap_viewMatrices.push_back(glm::lookAt(glm::vec3(paladin_pos), glm::vec3(paladin_pos) + glm::vec3( 1.0,  0.0,  0.0), glm::vec3(0.0, -1.0,  0.0)));
  cubeMap_viewMatrices.push_back(glm::lookAt(glm::vec3(paladin_pos), glm::vec3(paladin_pos) + glm::vec3(-1.0,  0.0,  0.0), glm::vec3(0.0, -1.0,  0.0)));
  cubeMap_viewMatrices.push_back(glm::lookAt(glm::vec3(paladin_pos), glm::vec3(paladin_pos) + glm::vec3( 0.0,  1.0,  0.0), glm::vec3(0.0,  0.0,  1.0)));
  cubeMap_viewMatrices.push_back(glm::lookAt(glm::vec3(paladin_pos), glm::vec3(paladin_pos) + glm::vec3( 0.0, -1.0,  0.0), glm::vec3(0.0,  0.0, -1.0)));
  cubeMap_viewMatrices.push_back(glm::lookAt(glm::vec3(paladin_pos), glm::vec3(paladin_pos) + glm::vec3( 0.0,  0.0,  1.0), glm::vec3(0.0, -1.0,  0.0)));
  cubeMap_viewMatrices.push_back(glm::lookAt(glm::vec3(paladin_pos), glm::vec3(paladin_pos) + glm::vec3( 0.0,  0.0, -1.0), glm::vec3(0.0, -1.0,  0.0)));*/

  cubeMap_viewMatrices.push_back(glm::lookAt(glm::vec3(paladin_pos), glm::vec3(paladin_pos) + glm::vec3( 1.0,  0.0,  0.0), glm::vec3(0.0, -1.0, 0.0)));
  cubeMap_viewMatrices.push_back(glm::lookAt(glm::vec3(paladin_pos), glm::vec3(paladin_pos) + glm::vec3(-1.0,  0.0,  0.0), glm::vec3(0.0, -1.0, 0.0)));
  cubeMap_viewMatrices.push_back(glm::lookAt(glm::vec3(paladin_pos), glm::vec3(paladin_pos) + glm::vec3( 0.0,  1.0,  0.0), glm::vec3(0.0, 0.0, 1.0)));
  cubeMap_viewMatrices.push_back(glm::lookAt(glm::vec3(paladin_pos), glm::vec3(paladin_pos) + glm::vec3( 0.0, -1.0,  0.0), glm::vec3(0.0, 0.0, -1.0)));
  cubeMap_viewMatrices.push_back(glm::lookAt(glm::vec3(paladin_pos), glm::vec3(paladin_pos) + glm::vec3( 0.0,  0.0,  1.0), glm::vec3(0.0, -1.0, 0.0)));
  cubeMap_viewMatrices.push_back(glm::lookAt(glm::vec3(paladin_pos), glm::vec3(paladin_pos) + glm::vec3( 0.0,  0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

  

  for (int i = 0; i < 6; ++i){
   
    glBindFramebuffer(GL_FRAMEBUFFER, 0);      
    
    if(i == 5){
      Pre_rendu_feu(projectionM, cubeMap_viewMatrices[i], i);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, reflection_cubeMap_FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i , reflection_cubeMap, 0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);      
   
    glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[0]);
   
   

    /*if(i == 5){
      glClearColor(0.0f, 0.0f, 0.8f, 1.0f);
    }else{ glClearColor(0.3f, 0.3f, 0.3f, 1.0f); }*/

      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



     // DRAW SKYBOX 
    glDepthMask(GL_FALSE); // desactivé juste pour draw la skybox
    skybox_shader.Use();   

    glm::mat4 SkyboxViewMatrix = glm::mat4(glm::mat3(cubeMap_viewMatrices[i]));  // Remove any translation component of the view matrix
    glUniformMatrix4fv(glGetUniformLocation(skybox_shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(SkyboxViewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(skybox_shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projectionM));
    glUniform1f(glGetUniformLocation(skybox_shader.Program, "alpha"), 1.0);
    glUniform1f(glGetUniformLocation(skybox_shader.Program, "is_foggy"), 0.0);
    glUniform1f(glGetUniformLocation(skybox_shader.Program, "is_volum_light"), 0.0);


    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(skybox_shader.Program, "skybox"), 0); // envoi du sampler cube 
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex_cube_map); // bind les 6 textures du cube map
 
    glEnable(GL_BLEND);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDisable(GL_BLEND);

    glBindVertexArray(0);
    glDepthMask(GL_TRUE);  
    glUseProgram(0);



    /////////////////////////////////// DRAW FEU DE CAMP
    if(i == 5){
      basic_shader.Use();

      glActiveTexture(GL_TEXTURE3);
      glBindTexture(GL_TEXTURE_2D, tex_depth_particle2);
      glActiveTexture(GL_TEXTURE4);
      glBindTexture(GL_TEXTURE_2D, tex_pre_rendu_feu2);


      Msend = glm::mat4();

      Msend = glm::translate(Msend, glm::vec3(Feu.x,Feu.y,Feu.z));
      Msend = glm::rotate(Msend, (float)myPI_2, glm::vec3(-1.0, 0.0 , 0.0));
      Msend = glm::scale(Msend, glm::vec3(Feu.scale)); 

      glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, ("cubeMap_viewMatrices[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, glm::value_ptr(cubeMap_viewMatrices[i]));            
      glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(Msend));
      glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionM));


      glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightPos[0]"),1, &lights[0].lightPos[0]);
      glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightColor[0]"),1, &lights[0].lightColor[0]);
      glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightSpecularColor[0]"),1, &lights[0].lightSpecularColor[0]);
      glUniform1f(glGetUniformLocation(basic_shader.Program, "constant[0]"),1.0f);
      glUniform1f(glGetUniformLocation(basic_shader.Program, "linear[0]"),  0.007);
      glUniform1f(glGetUniformLocation(basic_shader.Program, "quadratic[0]"), 0.0002);


      glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightPos[1]"),1, &lights[1].lightPos[0]);
      glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightColor[1]"),1, &lights[1].lightColor[0]);
      glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightSpecularColor[1]"),1, &lights[1].lightSpecularColor[0]);
      glUniform1f(glGetUniformLocation(basic_shader.Program, "constant[1]"),1.0f);
      glUniform1f(glGetUniformLocation(basic_shader.Program, "linear[1]"),  0.7);
      glUniform1f(glGetUniformLocation(basic_shader.Program, "quadratic[1]"), 1.8);
      glUniform1f(glGetUniformLocation(basic_shader.Program, "fire_intensity"), fire_intensity);


      glUniform1f(glGetUniformLocation(basic_shader.Program, "ambientSTR"), Feu.AmbientStr);
      glUniform1f(glGetUniformLocation(basic_shader.Program, "diffuseSTR"), Feu.DiffuseStr);
      glUniform1f(glGetUniformLocation(basic_shader.Program, "specularSTR"), Feu.SpecularStr);
      glUniform1f(glGetUniformLocation(basic_shader.Program, "ShiniSTR"), Feu.ShiniStr);

      glUniform3fv(glGetUniformLocation(basic_shader.Program, "viewPos"), 1, &paladin_pos[0]);

      glUniform1f(glGetUniformLocation(basic_shader.Program, "alpha"), Feu.alpha);
      glUniform1f(glGetUniformLocation(basic_shader.Program, "var"), Feu.var);    
      glUniform1f(glGetUniformLocation(basic_shader.Program, "depth_test"), 0.0);    
      glUniform1f(glGetUniformLocation(basic_shader.Program, "face_cube"), i);    

      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);
      feu_model.Draw(basic_shader, glm::mat4(1.0), false);
      glDisable(GL_CULL_FACE);
      

      glBindVertexArray(0);
      glUseProgram(0);
    }


    
    ///////////////////////////////////////////////// DRAW PARTICULES      

    if(i == 5){
      particule_shader.Use();   
  
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, tex_particule);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, tex_depth_feu2);
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, tex_pre_rendu_feu2);


      Msend = glm::mat4(1.0);
      Msend = glm::translate(Msend, glm::vec3(particules_pos.x,particules_pos.y,particules_pos.z));
     

      glUniformMatrix4fv(glGetUniformLocation(particule_shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projectionM));
      //glUniformMatrix4fv(glGetUniformLocation(particule_shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
      glUniformMatrix4fv(glGetUniformLocation(particule_shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(cubeMap_viewMatrices[i]));            
      glUniformMatrix4fv(glGetUniformLocation(particule_shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(Msend));
      glUniform1f(glGetUniformLocation(particule_shader.Program, "test"), 1.0);
      glUniform1f(glGetUniformLocation(particule_shader.Program, "blend_factor"), Particles -> blend_factor);
      glUniform1f(glGetUniformLocation(particule_shader.Program, "face_cube"), i);


      Particles -> Draw(true,true);

      glUseProgram(0);  
    }


    if(/*i == 5*/ true){

      glBindFramebuffer(GL_FRAMEBUFFER, 0);      

    // BLURING
      int horizontal = 1; 
      GLuint amount = /*6*/ 2;
      blur_shader.Use();
      for (GLuint i = 0; i < amount; i++)
      {
     
     //std::cout << "horizontal = " << horizontal << std::endl;
       glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]); 
       glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     
     
       if(horizontal == 0){
        horizontal = 1;
       }else{ horizontal = 0; }
     
       glActiveTexture(GL_TEXTURE0);
       glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
     
       glUniform1f(glGetUniformLocation(blur_shader.Program, "horizontal"), horizontal);
       glUniform1i(glGetUniformLocation(blur_shader.Program, "is_reflection_blur"), 1);

       RenderQuad();
       glBindFramebuffer(GL_FRAMEBUFFER, 0);     
    
     

     }
     glUseProgram(0);

     glBindFramebuffer(GL_READ_FRAMEBUFFER, pingpongFBO[0]); 
     glBindFramebuffer(GL_DRAW_FRAMEBUFFER, reflection_cubeMap_FBO);
     //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i , reflection_cubeMap, 0);
    

     glBlitFramebuffer(0, 0, reflection_cubeMap_res, reflection_cubeMap_res,             
                  0, 0, reflection_cubeMap_res, reflection_cubeMap_res,             
                  GL_COLOR_BUFFER_BIT,             
                  GL_LINEAR);    


    }


  }
 

  ////////////////////////
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);      
  glViewport(0, 0, w, h);
  //glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
  

}


void Pre_rendu_shadow_house(glm::mat4 projectionMatrix, glm::mat4 viewMatrix){

  static int test = 0;

  if(test == 0){


    test = 1;
  // DRAW DEPTH HOUSE     
    glBindFramebuffer(GL_FRAMEBUFFER, house_depth_FBO);
 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glm::mat4 lightProjection, lightView;
    GLfloat far =  glm::distance(lights[2].lightPos, glm::vec3(house.x,house.y,house.z)) * 1.2f;
    lightProjection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 1.0f, far);
    lightView = glm::lookAt(lights[2].lightPos, glm::vec3(house.x,house.y,house.z) , glm::vec3(0.0,1.0,0.0));


    basic_shader.Use();

    glm::mat4 Msend;
    Msend = glm::mat4();

    Msend = glm::translate(Msend, glm::vec3(house.x,house.y,house.z));
    Msend = glm::rotate(Msend, house.angle, glm::vec3(0.0, 1.0 , 0.0));
    Msend = glm::scale(Msend, glm::vec3(house.scale)); 

    glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(lightView));
    glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(Msend));
    glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(lightProjection));

    glUniform3fv(glGetUniformLocation(basic_shader.Program, "viewPos"), 1, &cameraPos[0]);

    glUniform1f(glGetUniformLocation(basic_shader.Program, "alpha"), house.alpha);
    glUniform1f(glGetUniformLocation(basic_shader.Program, "var"), house.var);    
    glUniform1f(glGetUniformLocation(basic_shader.Program, "depth_test"), 1.0);    
    glUniform1f(glGetUniformLocation(basic_shader.Program, "face_cube"), -1.0);     
  
  
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    house_model.Draw(basic_shader, glm::mat4(1.0), false);
    glDisable(GL_CULL_FACE);

    glBindVertexArray(0);
    glUseProgram(0);


   // DRAW SWORD 
    basic_shader.Use();

    Msend = glm::mat4();
    Msend = glm::translate(Msend, glm::vec3(sword.x,sword.y,sword.z));
    Msend = glm::rotate(Msend, sword.angle, glm::vec3(0.0, 0.0 , 1.0));
    Msend = glm::scale(Msend, glm::vec3(sword.scale)); 

    glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(lightView));
    glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(Msend));
    glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(lightProjection));

    glUniform3fv(glGetUniformLocation(basic_shader.Program, "viewPos"), 1, &cameraPos[0]);

    glUniform1f(glGetUniformLocation(basic_shader.Program, "alpha"), sword.alpha);
    glUniform1f(glGetUniformLocation(basic_shader.Program, "var"), sword.var);    
    glUniform1f(glGetUniformLocation(basic_shader.Program, "depth_test"), 1.0);    
    glUniform1f(glGetUniformLocation(basic_shader.Program, "face_cube"), -1.0);     

    sword_model.Draw(basic_shader, glm::mat4(1.0), false);
    

    glBindVertexArray(0);
    glUseProgram(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);   


  }

}

void Pre_rendu_shadow_cubeMap(){

  glm::mat4 projectionM,Msend;

  std::vector<glm::mat4> cubeMap_viewMatrices;
  projectionM = glm::perspective(89.5373f, (float)reflection_cubeMap_res/(float)reflection_cubeMap_res, 0.12f, 1000.0f);
  
  
  /////////////
  
  cubeMap_viewMatrices.push_back(glm::lookAt(glm::vec3(lights[1].lightPos), glm::vec3(lights[1].lightPos) + glm::vec3( 1.0,  0.0,  0.0), glm::vec3(0.0, -1.0, 0.0)));
  cubeMap_viewMatrices.push_back(glm::lookAt(glm::vec3(lights[1].lightPos), glm::vec3(lights[1].lightPos) + glm::vec3(-1.0,  0.0,  0.0), glm::vec3(0.0, -1.0, 0.0)));
  cubeMap_viewMatrices.push_back(glm::lookAt(glm::vec3(lights[1].lightPos), glm::vec3(lights[1].lightPos) + glm::vec3( 0.0,  1.0,  0.0), glm::vec3(0.0, 0.0, 1.0)));
  cubeMap_viewMatrices.push_back(glm::lookAt(glm::vec3(lights[1].lightPos), glm::vec3(lights[1].lightPos) + glm::vec3( 0.0, -1.0,  0.0), glm::vec3(0.0, 0.0, -1.0)));
  cubeMap_viewMatrices.push_back(glm::lookAt(glm::vec3(lights[1].lightPos), glm::vec3(lights[1].lightPos) + glm::vec3( 0.0,  0.0,  1.0), glm::vec3(0.0, -1.0, 0.0)));
  cubeMap_viewMatrices.push_back(glm::lookAt(glm::vec3(lights[1].lightPos), glm::vec3(lights[1].lightPos) + glm::vec3( 0.0,  0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));


  for (int i = 0; i < 6; ++i){
   
    glBindFramebuffer(GL_FRAMEBUFFER, 0);      
    
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_cubeMap_FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i , tex_shadow_cubeMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // DRAW FEU DE CAMP
    if(i != 2){
      basic_shader.Use();

      Msend = glm::mat4();

      Msend = glm::translate(Msend, glm::vec3(Feu.x,Feu.y,Feu.z));
      Msend = glm::rotate(Msend, (float)myPI_2, glm::vec3(-1.0, 0.0 , 0.0));
      Msend = glm::scale(Msend, glm::vec3(Feu.scale)); 

      glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, ("cubeMap_viewMatrices[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, glm::value_ptr(cubeMap_viewMatrices[i]));            
      glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(Msend));
      glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionM));

      glUniform3fv(glGetUniformLocation(basic_shader.Program, "viewPos"), 1, &lights[1].lightPos[0]);

      glUniform1f(glGetUniformLocation(basic_shader.Program, "alpha"), Feu.alpha);
      glUniform1f(glGetUniformLocation(basic_shader.Program, "var"), Feu.var);    
      glUniform1f(glGetUniformLocation(basic_shader.Program, "depth_test"), 1.0);    
      glUniform1f(glGetUniformLocation(basic_shader.Program, "face_cube"), i);    

      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK); 
      feu_model.Draw(basic_shader, glm::mat4(1.0), false);
      glDisable(GL_CULL_FACE);

      glBindVertexArray(0);
      glUseProgram(0);
    }

    // DRAW SKINNED PALADIN
    if(i ==  0 || i == 4){
      skinning_shader.Use();

      for (uint i = 0 ; i < Transforms.size() ; i++) {
        SetBoneTransform(i, Transforms[i],1);
      }

      Msend = glm::mat4();

      Msend = glm::translate(Msend, glm::vec3(paladin.x,paladin.y,paladin.z));
      Msend = glm::rotate(Msend, paladin.angle, glm::vec3(0.0 , 1.0 , 0.0));
      Msend = glm::rotate(Msend, paladin.acca, glm::vec3(1.0 , 0.0 , 0.0));
      Msend = glm::scale(Msend, glm::vec3(paladin.scale)); 


      glUniformMatrix4fv(glGetUniformLocation(skinning_shader.Program, ("cubeMap_viewMatrices[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, glm::value_ptr(cubeMap_viewMatrices[i]));            
      glUniformMatrix4fv(glGetUniformLocation(skinning_shader.Program, "modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(Msend));
      glUniformMatrix4fv(glGetUniformLocation(skinning_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionM));

      glUniform3fv(glGetUniformLocation(skinning_shader.Program, "viewPos"), 1, &lights[1].lightPos[0]);

      glUniform1f(glGetUniformLocation(skinning_shader.Program, "alpha"), paladin.alpha);
      glUniform1f(glGetUniformLocation(skinning_shader.Program, "var"), paladin.var);    
      glUniform1f(glGetUniformLocation(skinning_shader.Program, "depth_test"), 1.0);   
      glUniform1f(glGetUniformLocation(skinning_shader.Program, "face_cube"), i);     


      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK); 
      paladin_skinned.Render(); 
      glDisable(GL_CULL_FACE);

      glBindVertexArray(0);
      glUseProgram(0);
    }

    // DRAW HOUSE
    if(i != 1 || i != 3){
      basic_shader.Use();

      Msend = glm::mat4();

      Msend = glm::translate(Msend, glm::vec3(house.x,house.y,house.z));
      Msend = glm::rotate(Msend, house.angle, glm::vec3(0.0, 1.0 , 0.0));
      Msend = glm::scale(Msend, glm::vec3(house.scale)); 

      glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, ("cubeMap_viewMatrices[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, glm::value_ptr(cubeMap_viewMatrices[i])); 
      glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(Msend));
      glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionM));

      glUniform3fv(glGetUniformLocation(basic_shader.Program, "viewPos"), 1, &lights[1].lightPos[0]);

      glUniform1f(glGetUniformLocation(basic_shader.Program, "alpha"), house.alpha);
      glUniform1f(glGetUniformLocation(basic_shader.Program, "var"), house.var);    
      glUniform1f(glGetUniformLocation(basic_shader.Program, "depth_test"), 1.0);    
      glUniform1f(glGetUniformLocation(basic_shader.Program, "face_cube"), i);     


      glEnable(GL_CULL_FACE);
      glCullFace(GL_FRONT);
      house_model.Draw(basic_shader, glm::mat4(1.0), true);
      glDisable(GL_CULL_FACE);


      glBindVertexArray(0);
      glUseProgram(0);
    }
    // DRAW SHIELD
    if(i == 5){
      basic_shader.Use();

      Msend = glm::mat4();
      Msend = glm::translate(Msend, glm::vec3(shield.x,shield.y,shield.z));
      Msend = glm::rotate(Msend, shield.angle, glm::vec3(0.0, 1.0 , 0.0));
      Msend = glm::rotate(Msend, shield.angle*4.0f, glm::vec3(1.0, 0.0 , 0.0));

      Msend = glm::scale(Msend, glm::vec3(shield.scale)); 

      glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, ("cubeMap_viewMatrices[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, glm::value_ptr(cubeMap_viewMatrices[i])); 
      glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(Msend));
      glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionM));

      glUniform3fv(glGetUniformLocation(basic_shader.Program, "viewPos"), 1, &lights[1].lightPos[0]);

      glUniform1f(glGetUniformLocation(basic_shader.Program, "alpha"), shield.alpha);
      glUniform1f(glGetUniformLocation(basic_shader.Program, "var"), shield.var);    
      glUniform1f(glGetUniformLocation(basic_shader.Program, "depth_test"), 1.0);    
      glUniform1f(glGetUniformLocation(basic_shader.Program, "face_cube"), i);     


      glEnable(GL_CULL_FACE);
      glCullFace(GL_FRONT);
      shield_model.Draw(basic_shader, glm::mat4(1.0), false);
      glDisable(GL_CULL_FACE);


      glBindVertexArray(0);
      glUseProgram(0);
    }



  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);      
  glViewport(0, 0, w, h);
  
}

void RenderShadowedObjects(bool VL_pre_rendering){


 glm::mat4 projectionM,Msend,viewMatrix,Msend2;

 projectionM = glm::perspective(45.0f, /* 4.0f/3.0f */(float)w/(float)h, 0.1f, 1000.0f);
 viewMatrix=glm::lookAt(cameraPos, (cameraPos) + cameraFront, cameraUp); 


 glm::mat4 lightProjection, lightView, light_space_matrix, skybox_light_space_matrix;
 GLfloat far =  glm::distance(lights[2].lightPos, glm::vec3(house.x,house.y,house.z)) * 1.2f;
 lightProjection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 1.0f, far);
 lightView = glm::lookAt(lights[2].lightPos, glm::vec3(house.x,house.y,house.z) , glm::vec3(0.0,1.0,0.0));
 light_space_matrix = lightProjection * lightView;
 glm::vec3 mid_fog_position = glm::vec3(house.x,house.y,house.z);

 skybox_light_space_matrix = lightProjection * glm::mat4(glm::mat3(lightView));


 if(VL_pre_rendering){
  glBindFramebuffer(GL_FRAMEBUFFER, VL_FBO[0]);
  
  //glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 }


 // DRAW SKYBOX 
 //glDepthMask(GL_FALSE); // desactivé juste pour draw la skybox
 skybox_shader.Use();   
 glm::mat4 SkyboxViewMatrix = glm::mat4(glm::mat3(viewMatrix));  // Remove any translation component of the view matrix

 Msend = glm::mat4(1.0f);
 Msend = glm::translate(Msend, glm::vec3(cameraPos.x, cameraPos.y, cameraPos.z));
 Msend = glm::scale(Msend, glm::vec3(100.0f)); 

 glUniformMatrix4fv(glGetUniformLocation(skybox_shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(/*SkyboxViewMatrix*/ viewMatrix));
 glUniformMatrix4fv(glGetUniformLocation(skybox_shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projectionM));
 glUniformMatrix4fv(glGetUniformLocation(skybox_shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(Msend));

 glUniform1f(glGetUniformLocation(skybox_shader.Program, "alpha"), skybox_alpha);
 glUniform1f(glGetUniformLocation(skybox_shader.Program, "is_foggy"), 1.0);
 glUniform1f(glGetUniformLocation(skybox_shader.Program, "is_volum_light"), 1.0);
 glUniform1f(glGetUniformLocation(skybox_shader.Program, "VL_intensity"), VL_intensity); 
 glUniform3fv(glGetUniformLocation(skybox_shader.Program, "viewPos"), 1, &cameraPos[0]);
 glUniformMatrix4fv(glGetUniformLocation(skybox_shader.Program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(light_space_matrix /*skybox_light_space_matrix*/)); 
 glUniform3fv(glGetUniformLocation(skybox_shader.Program, "LightPos[2]"),1, &lights[2].lightPos[0]);


 glBindVertexArray(skyboxVAO);
 glActiveTexture(GL_TEXTURE0);
 glUniform1i(glGetUniformLocation(skybox_shader.Program, "skybox"), 0); // envoi du sampler cube 
 glBindTexture(GL_TEXTURE_CUBE_MAP, tex_cube_map /*reflection_cubeMap*/ /*tex_shadow_cubeMap*/); // bind les 6 textures du cube map
 
 glActiveTexture(GL_TEXTURE1);
 glUniform1i(glGetUniformLocation(skybox_shader.Program, "shadow_map1"), 1);  
 glBindTexture(GL_TEXTURE_2D, tex_depth_house); 
 

 glEnable(GL_BLEND);
 glDrawArrays(GL_TRIANGLES, 0, 36);
 glDisable(GL_BLEND);

 glBindVertexArray(0);
 //glDepthMask(GL_TRUE);  // réactivé pour draw le reste
 glUseProgram(0);


  // DRAW LAMP
 /*lamp_shader.Use();
 
 glBindVertexArray(lampVAO);

 Msend= glm::mat4();
 Msend = glm::translate(Msend, lights[1].lightPos);
 Msend = glm::scale(Msend, glm::vec3(0.07f)); 

 glm::vec3 lampColor(0.2,0.2,0.9);

 glUniformMatrix4fv(glGetUniformLocation(lamp_shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
 glUniformMatrix4fv(glGetUniformLocation(lamp_shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(Msend));
 glUniformMatrix4fv(glGetUniformLocation(lamp_shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projectionM));
 glUniform3f(glGetUniformLocation(lamp_shader.Program, "lampColor"), lampColor.x,lampColor.z,lampColor.z);

 glDrawArrays(GL_TRIANGLES, 0, nbVerticesSphere);

 glBindVertexArray(0);
 glUseProgram(0);*/



 // DRAW ground
 basic_shader.Use();
 
 glActiveTexture(GL_TEXTURE0);
 glBindTexture(GL_TEXTURE_2D, tex_ground_color);
 glActiveTexture(GL_TEXTURE2);
 glBindTexture(GL_TEXTURE_2D, tex_ground_AO);
 glActiveTexture(GL_TEXTURE1);
 glBindTexture(GL_TEXTURE_2D, tex_ground_normal);
 glActiveTexture(GL_TEXTURE6);
 glBindTexture(GL_TEXTURE_2D, tex_depth_house);
 

 Msend = glm::mat4(1.0f);
 Msend = glm::translate(Msend, glm::vec3((ground->x),ground->y,(ground->z)));
 Msend = glm::scale(Msend, glm::vec3(ground->scale)); 
    


 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(Msend));
 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionM));
 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(light_space_matrix));
 glUniform1f(glGetUniformLocation(basic_shader.Program, "send_bias"), send_bias);



 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightPos[0]"),1, &lights[0].lightPos[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightColor[0]"),1, &lights[0].lightColor[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightSpecularColor[0]"),1, &lights[0].lightSpecularColor[0]);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "constant[0]"),1.0f);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "linear[0]"),  0.007);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "quadratic[0]"), 0.0002);
 

 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightPos[1]"),1, &lights[1].lightPos[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightColor[1]"),1, &lights[1].lightColor[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightSpecularColor[1]"),1, &lights[1].lightSpecularColor[0]);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "constant[1]"),1.0f);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "linear[1]"),  0.14);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "quadratic[1]"), 0.07);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fire_intensity"), fire_intensity);

 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightPos[2]"),1, &lights[2].lightPos[0]);

 glUniform1f(glGetUniformLocation(basic_shader.Program, "ambientSTR"), ground->AmbientStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "diffuseSTR"), ground->DiffuseStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "specularSTR"), ground->SpecularStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "ShiniSTR"), ground->ShiniStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "shadow_darkness"), ground->shadow_darkness);
 

 glUniform3fv(glGetUniformLocation(basic_shader.Program, "viewPos"), 1, &cameraPos[0]);
 
 glUniform1f(glGetUniformLocation(basic_shader.Program, "alpha"), 1.0);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "var"), ground->var);    
 glUniform1f(glGetUniformLocation(basic_shader.Program, "depth_test"), 0.0);    
 glUniform1f(glGetUniformLocation(basic_shader.Program, "face_cube"), -1.0);     

 glUniform4fv(glGetUniformLocation(basic_shader.Program, "fog_color"),1, &fog_color[0]);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fog_density"), fog_density);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fog_equation"), fog_equation);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "mid_fog_position"), 1, &mid_fog_position[0]);
 
 glUniform1f(glGetUniformLocation(basic_shader.Program, "VL_intensity"), VL_intensity);
  
 glEnable(GL_CULL_FACE);
 glCullFace(GL_FRONT);
 MyMap.RenderHeightmap();
 glDisable(GL_CULL_FACE);


 glBindVertexArray(0);
 glUseProgram(0);



 /////////////////////////////////// DRAW FEU DE CAMP
 basic_shader.Use();

 glActiveTexture(GL_TEXTURE3);
 glBindTexture(GL_TEXTURE_2D, tex_depth_particle);
 glActiveTexture(GL_TEXTURE4);
 glBindTexture(GL_TEXTURE_2D, tex_pre_rendu_feu);
 glActiveTexture(GL_TEXTURE6);
 glBindTexture(GL_TEXTURE_2D, tex_depth_house);
 

 Msend = glm::mat4();

 Msend = glm::translate(Msend, glm::vec3(Feu.x,Feu.y,Feu.z));
 Msend = glm::rotate(Msend, (float)myPI_2, glm::vec3(-1.0, 0.0 , 0.0));
 Msend = glm::scale(Msend, glm::vec3(Feu.scale)); 

 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(Msend));
 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionM));
 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(light_space_matrix));
 glUniform1f(glGetUniformLocation(basic_shader.Program, "send_bias"), send_bias);


 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightPos[0]"),1, &lights[0].lightPos[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightColor[0]"),1, &lights[0].lightColor[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightSpecularColor[0]"),1, &lights[0].lightSpecularColor[0]);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "constant[0]"),1.0f);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "linear[0]"),  0.007);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "quadratic[0]"), 0.0002);
 

 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightPos[1]"),1, &lights[1].lightPos[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightColor[1]"),1, &lights[1].lightColor[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightSpecularColor[1]"),1, &lights[1].lightSpecularColor[0]);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "constant[1]"),1.0f);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "linear[1]"),  0.7);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "quadratic[1]"), 1.8);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fire_intensity"), fire_intensity);



 glUniform1f(glGetUniformLocation(basic_shader.Program, "ambientSTR"), Feu.AmbientStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "diffuseSTR"), Feu.DiffuseStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "specularSTR"), Feu.SpecularStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "ShiniSTR"), Feu.ShiniStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "shadow_darkness"), Feu.shadow_darkness);
 
 
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "viewPos"), 1, &cameraPos[0]);

 glUniform1f(glGetUniformLocation(basic_shader.Program, "alpha"), Feu.alpha);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "var"), Feu.var);    
 glUniform1f(glGetUniformLocation(basic_shader.Program, "depth_test"), 0.0);    
 glUniform1f(glGetUniformLocation(basic_shader.Program, "face_cube"), -1.0);     
  
 glUniform1f(glGetUniformLocation(basic_shader.Program, "VL_intensity"), VL_intensity);

 glEnable(GL_CULL_FACE);
 glCullFace(GL_BACK); 
 feu_model.Draw(basic_shader, Msend2, false);
 glDisable(GL_CULL_FACE);

 glBindVertexArray(0);
 glUseProgram(0);


 //////////////////////// DRAW PALADIN
/* basic_shader.Use();

 glActiveTexture(GL_TEXTURE5); 
 glBindTexture(GL_TEXTURE_CUBE_MAP, reflection_cubeMap);

 Msend = glm::mat4();

 Msend = glm::translate(Msend, glm::vec3(paladin.x,paladin.y,paladin.z));
 Msend = glm::rotate(Msend, (float)myPI_2, glm::vec3(0.0 , -1.0 , 0.0));
 Msend = glm::scale(Msend, glm::vec3(paladin.scale)); 

 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(Msend));
 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionM));


 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightPos[0]"),1, &lights[0].lightPos[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightColor[0]"),1, &lights[0].lightColor[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightSpecularColor[0]"),1, &lights[0].lightSpecularColor[0]);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "constant[0]"),1.0f);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "linear[0]"),  0.007*0.2);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "quadratic[0]"), 0.0002*0.2);
 

 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightPos[1]"),1, &lights[1].lightPos[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightColor[1]"),1, &lights[1].lightColor[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightSpecularColor[1]"),1, &lights[1].lightSpecularColor[0]);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "constant[1]"),1.0f);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "linear[1]"),  0.35);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "quadratic[1]"), 0.44);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fire_intensity"), fire_intensity);


 glUniform1f(glGetUniformLocation(basic_shader.Program, "ambientSTR"), paladin.AmbientStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "diffuseSTR"), paladin.DiffuseStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "specularSTR"), paladin.SpecularStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "ShiniSTR"), paladin.ShiniStr);
 
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "viewPos"), 1, &cameraPos[0]);

 glUniform1f(glGetUniformLocation(basic_shader.Program, "alpha"), paladin.alpha);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "var"), paladin.var);    
 glUniform1f(glGetUniformLocation(basic_shader.Program, "depth_test"), 0.0);   
 glUniform1f(glGetUniformLocation(basic_shader.Program, "face_cube"), -1.0);     
   
 
 paladin_model.Draw(basic_shader, Msend2, false);

 glBindVertexArray(0);
 glUseProgram(0);
*/


//////////////////////// DRAW SKINNED PALADIN

// CALCUL MATRICES BONES
 float start_anim = 0.0;

//printf("t = %f\n", zombie[0].t);
        
 float RunningTime1, RunningTime2;
 static float anim2_start = -1.0;
 static float anim3_start = -1.0;

 //float RunningTime2 = (float)((double)zombie[1].t - ((double)4600.0f+start_anim))/1000.0f; 
 //float RunningTime3 = ((float)((double)zombie[1].t - ((double)6700.0f+4600.0+start_anim))/1000.0f); 
 //float RunningTime4 = ((float)((double)zombie[1].t - ((double)6700.0f+4600.0+3700.0+start_anim))/1000.0f); 

  //std::cout << "t = " << RunningTime1 << std::endl;    

 /*if(zombie[0].t < start_anim)
  paladin_skinned.BoneTransform(0.0, Transforms, paladin_sitting_idle);*/


 RunningTime1 = (float)((double)ground->t)/1000.0f;
  

  if(step < 16){
    paladin_skinned.BoneTransform(RunningTime1, Transforms, paladin_sitting_idle /*paladin_standing_up*/ /*paladin_breathing_idle*/ /*paladin_warrior_idle*/);
  }

  if(step == 16){
    if(anim2_start == -1.0)
      anim2_start = (float)((double)ground->t)/1000.0f;

    RunningTime2 = (float)((double)ground->t)/1000.0f;
    RunningTime2 = (RunningTime2 - anim2_start) + 3.0f;
    paladin_skinned.BoneTransform(RunningTime2, Transforms, paladin_standing_up);
  }

  if(step > 16){
    if(anim3_start == -1.0)
      anim3_start = (float)((double)ground->t)/1000.0f;

    RunningTime2 = (float)((double)ground->t)/1000.0f;
    RunningTime2 = (RunningTime2 - anim3_start);
    paladin_skinned.BoneTransform(RunningTime2, Transforms, paladin_warrior_idle);
  }

/*  if(RunningTime3 >= 0.0 && RunningTime3 < 4.6+6.7+3.7+(start_anim/1000.0)){

    zombie_model.BoneTransform(RunningTime3+0.28f, Transforms, zombie_scream);
  }

  if(RunningTime4 >= 0.0 ){

    zombie_model.BoneTransform(RunningTime4, Transforms, zombie_run);
  }*/
         
  //std::cout << "size = " << Transforms.size() << std::endl;    



 skinning_shader.Use();

  for (uint i = 0 ; i < Transforms.size() ; i++) {
  SetBoneTransform(i, Transforms[i],1);
 }


 glActiveTexture(GL_TEXTURE5); 
 glBindTexture(GL_TEXTURE_CUBE_MAP, reflection_cubeMap);
 glActiveTexture(GL_TEXTURE3);
 glBindTexture(GL_TEXTURE_2D, tex_depth_house);
 glActiveTexture(GL_TEXTURE6);
 glBindTexture(GL_TEXTURE_CUBE_MAP, tex_shadow_cubeMap);
 

 Msend = glm::mat4();

 Msend = glm::translate(Msend, glm::vec3(paladin.x,paladin.y,paladin.z));
 Msend = glm::rotate(Msend, paladin.angle, glm::vec3(0.0 , 1.0 , 0.0));
 Msend = glm::rotate(Msend, paladin.acca, glm::vec3(1.0 , 0.0 , 0.0));
 Msend = glm::scale(Msend, glm::vec3(paladin.scale)); 


 glUniformMatrix4fv(glGetUniformLocation(skinning_shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
 glUniformMatrix4fv(glGetUniformLocation(skinning_shader.Program, "modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(Msend));
 glUniformMatrix4fv(glGetUniformLocation(skinning_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionM));
 glUniformMatrix4fv(glGetUniformLocation(skinning_shader.Program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(light_space_matrix));
 glUniform1f(glGetUniformLocation(skinning_shader.Program, "send_bias"), send_bias);



 glUniform3fv(glGetUniformLocation(skinning_shader.Program, "LightPos[0]"),1, &lights[0].lightPos[0]);
 glUniform3fv(glGetUniformLocation(skinning_shader.Program, "LightColor[0]"),1, &lights[0].lightColor[0]);
 glUniform3fv(glGetUniformLocation(skinning_shader.Program, "LightSpecularColor[0]"),1, &lights[0].lightSpecularColor[0]);
 glUniform1f(glGetUniformLocation(skinning_shader.Program, "constant[0]"),1.0f);
 glUniform1f(glGetUniformLocation(skinning_shader.Program, "linear[0]"),  0.007*0.2);
 glUniform1f(glGetUniformLocation(skinning_shader.Program, "quadratic[0]"), 0.0002*0.2);
 //glUniform1f(glGetUniformLocation(skinning_shader.Program, "test_bias"), test_bias);
 

 glUniform3fv(glGetUniformLocation(skinning_shader.Program, "LightPos[1]"),1, &lights[1].lightPos[0]);
 glUniform3fv(glGetUniformLocation(skinning_shader.Program, "LightColor[1]"),1, &lights[1].lightColor[0]);
 glUniform3fv(glGetUniformLocation(skinning_shader.Program, "LightSpecularColor[1]"),1, &lights[1].lightSpecularColor[0]);
 glUniform1f(glGetUniformLocation(skinning_shader.Program, "constant[1]"),1.0f);
 glUniform1f(glGetUniformLocation(skinning_shader.Program, "linear[1]"),  0.35);
 glUniform1f(glGetUniformLocation(skinning_shader.Program, "quadratic[1]"), 0.44);
 glUniform1f(glGetUniformLocation(skinning_shader.Program, "fire_intensity"), fire_intensity);


 glUniform1f(glGetUniformLocation(skinning_shader.Program, "ambientSTR"), paladin.AmbientStr);
 glUniform1f(glGetUniformLocation(skinning_shader.Program, "diffuseSTR"), paladin.DiffuseStr);
 glUniform1f(glGetUniformLocation(skinning_shader.Program, "specularSTR"), paladin.SpecularStr);
 glUniform1f(glGetUniformLocation(skinning_shader.Program, "ShiniSTR"), paladin.ShiniStr);
 glUniform1f(glGetUniformLocation(skinning_shader.Program, "shadow_darkness"), paladin.shadow_darkness);
 
 
 glUniform3fv(glGetUniformLocation(skinning_shader.Program, "viewPos"), 1, &cameraPos[0]);

 glUniform1f(glGetUniformLocation(skinning_shader.Program, "alpha"), paladin.alpha);
 glUniform1f(glGetUniformLocation(skinning_shader.Program, "var"), paladin.var);    
 glUniform1f(glGetUniformLocation(skinning_shader.Program, "depth_test"), 0.0);   
 glUniform1f(glGetUniformLocation(skinning_shader.Program, "face_cube"), -1.0);     
 glUniform1i(glGetUniformLocation(skinning_shader.Program, "shadow_point_light"), shadow_point_light);
   
 glUniform1f(glGetUniformLocation(skinning_shader.Program, "VL_intensity"), VL_intensity);

 //glEnable(GL_CULL_FACE);
 //glCullFace(GL_BACK); 
 paladin_skinned.Render(); 
 //glDisable(GL_CULL_FACE);

 glBindVertexArray(0);
 glUseProgram(0);




/////////////////////////////////// DRAW HOUSE
 basic_shader.Use();

 glActiveTexture(GL_TEXTURE6);
 glBindTexture(GL_TEXTURE_2D, tex_depth_house);
 glActiveTexture(GL_TEXTURE8);
 glBindTexture(GL_TEXTURE_CUBE_MAP, tex_shadow_cubeMap); 

 Msend = glm::mat4();

 Msend = glm::translate(Msend, glm::vec3(house.x,house.y,house.z));
 Msend = glm::rotate(Msend, house.angle, glm::vec3(0.0, 1.0 , 0.0));
 Msend = glm::scale(Msend, glm::vec3(house.scale)); 

 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(Msend));
 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionM));
 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(light_space_matrix));
 glUniform1f(glGetUniformLocation(basic_shader.Program, "send_bias"), send_bias);



 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightPos[0]"),1, &lights[0].lightPos[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightColor[0]"),1, &lights[0].lightColor[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightSpecularColor[0]"),1, &lights[0].lightSpecularColor[0]);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "constant[0]"),1.0f);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "linear[0]"),  0.007);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "quadratic[0]"), 0.0002);
 

 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightPos[1]"),1, &lights[1].lightPos[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightColor[1]"),1, &lights[1].lightColor[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightSpecularColor[1]"),1, &lights[1].lightSpecularColor[0]);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "constant[1]"),1.0f);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "linear[1]"),  0.5);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "quadratic[1]"), 0.9);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fire_intensity"), fire_intensity);


 glUniform1f(glGetUniformLocation(basic_shader.Program, "ambientSTR"), house.AmbientStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "diffuseSTR"), house.DiffuseStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "specularSTR"), house.SpecularStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "ShiniSTR"), house.ShiniStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "shadow_darkness"), house.shadow_darkness);
 
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "viewPos"), 1, &cameraPos[0]);

 glUniform1f(glGetUniformLocation(basic_shader.Program, "alpha"), house.alpha);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "var"), house.var);    
 glUniform1f(glGetUniformLocation(basic_shader.Program, "depth_test"), 0.0);    
 glUniform1f(glGetUniformLocation(basic_shader.Program, "face_cube"), -1.0);     
 glUniform1i(glGetUniformLocation(basic_shader.Program, "shadow_point_light"), shadow_point_light);     


 glUniform4fv(glGetUniformLocation(basic_shader.Program, "fog_color"),1, &fog_color[0]);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fog_density"), fog_density);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fog_equation"), fog_equation);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "mid_fog_position"), 1, &mid_fog_position[0]);

 glUniform1f(glGetUniformLocation(basic_shader.Program, "VL_intensity"), VL_intensity);

 glEnable(GL_CULL_FACE);
 glCullFace(GL_BACK);
 house_model.Draw(basic_shader, Msend2, false);
 glDisable(GL_CULL_FACE);
    

 glBindVertexArray(0);
 glUseProgram(0);


 // DRAW SWORD 
 basic_shader.Use();

 glActiveTexture(GL_TEXTURE5); 
 glBindTexture(GL_TEXTURE_CUBE_MAP, tex_cube_map);
 
 Msend = glm::mat4();
 Msend = glm::translate(Msend, glm::vec3(sword.x,sword.y,sword.z));
 Msend = glm::rotate(Msend, sword.angle, glm::vec3(0.0, 0.0 , 1.0));
 Msend = glm::scale(Msend, glm::vec3(sword.scale)); 

 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(Msend));
 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionM));
 //glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(light_space_matrix));
 //glUniform1f(glGetUniformLocation(basic_shader.Program, "send_bias"), send_bias);


 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightPos[0]"),1, &lights[0].lightPos[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightColor[0]"),1, &lights[0].lightColor[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightSpecularColor[0]"),1, &lights[0].lightSpecularColor[0]);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "constant[0]"),1.0f);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "linear[0]"),  0.007);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "quadratic[0]"), 0.0002);
 

 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightPos[1]"),1, &lights[1].lightPos[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightColor[1]"),1, &lights[1].lightColor[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightSpecularColor[1]"),1, &lights[1].lightSpecularColor[0]);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "constant[1]"),1.0f);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "linear[1]"),  0.5);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "quadratic[1]"), 0.9);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fire_intensity"), fire_intensity);


 glUniform1f(glGetUniformLocation(basic_shader.Program, "ambientSTR"), sword.AmbientStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "diffuseSTR"), sword.DiffuseStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "specularSTR"), sword.SpecularStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "ShiniSTR"), sword.ShiniStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "shadow_darkness"), sword.shadow_darkness);
 
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "viewPos"), 1, &cameraPos[0]);

 glUniform1f(glGetUniformLocation(basic_shader.Program, "alpha"), sword.alpha);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "var"), sword.var);    
 glUniform1f(glGetUniformLocation(basic_shader.Program, "depth_test"), 0.0);    
 glUniform1f(glGetUniformLocation(basic_shader.Program, "face_cube"), -1.0);     

 glUniform4fv(glGetUniformLocation(basic_shader.Program, "fog_color"),1, &fog_color[0]);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fog_density"), fog_density);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fog_equation"), fog_equation);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "mid_fog_position"), 1, &mid_fog_position[0]);
 
 glUniform1f(glGetUniformLocation(basic_shader.Program, "VL_intensity"), VL_intensity);

 sword_model.Draw(basic_shader, Msend2, false);   

 glBindVertexArray(0);
 glUseProgram(0);

 ////////// DRAW SHIELD
 basic_shader.Use();

 glActiveTexture(GL_TEXTURE5); 
 glBindTexture(GL_TEXTURE_CUBE_MAP, reflection_cubeMap);
 
 Msend = glm::mat4();
 Msend = glm::translate(Msend, glm::vec3(shield.x,shield.y,shield.z));
 Msend = glm::rotate(Msend, shield.angle, glm::vec3(0.0, 1.0 , 0.0));
 Msend = glm::rotate(Msend, shield.angle*4.0f, glm::vec3(1.0, 0.0 , 0.0));
 
 Msend = glm::scale(Msend, glm::vec3(shield.scale)); 

 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(Msend));
 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionM));
 //glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(light_space_matrix));
 //glUniform1f(glGetUniformLocation(basic_shader.Program, "send_bias"), send_bias);


 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightPos[0]"),1, &lights[0].lightPos[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightColor[0]"),1, &lights[0].lightColor[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightSpecularColor[0]"),1, &lights[0].lightSpecularColor[0]);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "constant[0]"),1.0f);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "linear[0]"),  0.007);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "quadratic[0]"), 0.0002);
 

 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightPos[1]"),1, &lights[1].lightPos[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightColor[1]"),1, &lights[1].lightColor[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightSpecularColor[1]"),1, &lights[1].lightSpecularColor[0]);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "constant[1]"),1.0f);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "linear[1]"),  0.5);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "quadratic[1]"), 0.9);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fire_intensity"), fire_intensity);


 glUniform1f(glGetUniformLocation(basic_shader.Program, "ambientSTR"), shield.AmbientStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "diffuseSTR"), shield.DiffuseStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "specularSTR"), shield.SpecularStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "ShiniSTR"), shield.ShiniStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "shadow_darkness"), shield.shadow_darkness);
 
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "viewPos"), 1, &cameraPos[0]);

 glUniform1f(glGetUniformLocation(basic_shader.Program, "alpha"), shield.alpha);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "var"), shield.var);    
 glUniform1f(glGetUniformLocation(basic_shader.Program, "depth_test"), 0.0);    
 glUniform1f(glGetUniformLocation(basic_shader.Program, "face_cube"), -1.0);     

 glUniform4fv(glGetUniformLocation(basic_shader.Program, "fog_color"),1, &fog_color[0]);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fog_density"), fog_density);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fog_equation"), fog_equation);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "mid_fog_position"), 1, &mid_fog_position[0]);

 glUniform1f(glGetUniformLocation(basic_shader.Program, "VL_intensity"), VL_intensity);
  
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  shield_model.Draw(basic_shader, Msend2, false);
  glDisable(GL_CULL_FACE);
    

 glBindVertexArray(0);
 glUseProgram(0);

 ///////////////////////// DRAW TREE(S)
/* basic_shader.Use();
 
 Msend = glm::mat4();
 Msend = glm::translate(Msend, glm::vec3(trees[0].x,trees[0].y,trees[0].z));
 //Msend = glm::rotate(Msend, trees[0].angle, glm::vec3(0.0, 1.0 , 0.0));
 //Msend = glm::rotate(Msend, trees[0].angle*4.0f, glm::vec3(1.0, 0.0 , 0.0));
 Msend = glm::scale(Msend, glm::vec3(trees[0].scale)); 

 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(Msend));
 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionM));
 //glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(light_space_matrix));
 //glUniform1f(glGetUniformLocation(basic_shader.Program, "send_bias"), send_bias);


 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightPos[0]"),1, &lights[0].lightPos[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightColor[0]"),1, &lights[0].lightColor[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightSpecularColor[0]"),1, &lights[0].lightSpecularColor[0]);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "constant[0]"),1.0f);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "linear[0]"),  0.007);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "quadratic[0]"), 0.0002);
 

 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightPos[1]"),1, &lights[1].lightPos[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightColor[1]"),1, &lights[1].lightColor[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightSpecularColor[1]"),1, &lights[1].lightSpecularColor[0]);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "constant[1]"),1.0f);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "linear[1]"),  0.5);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "quadratic[1]"), 0.9);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fire_intensity"), fire_intensity);


 glUniform1f(glGetUniformLocation(basic_shader.Program, "ambientSTR"), trees[0].AmbientStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "diffuseSTR"), trees[0].DiffuseStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "specularSTR"), trees[0].SpecularStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "ShiniSTR"), trees[0].ShiniStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "shadow_darkness"), trees[0].shadow_darkness);
 
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "viewPos"), 1, &cameraPos[0]);

 glUniform1f(glGetUniformLocation(basic_shader.Program, "alpha"), trees[0].alpha);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "var"), trees[0].var);    
 glUniform1f(glGetUniformLocation(basic_shader.Program, "depth_test"), 0.0);    
 glUniform1f(glGetUniformLocation(basic_shader.Program, "face_cube"), -1.0);     

 glUniform4fv(glGetUniformLocation(basic_shader.Program, "fog_color"),1, &fog_color[0]);
  
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fog_density"), fog_density);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fog_equation"), fog_equation);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "mid_fog_position"), 1, &mid_fog_position[0]);

  
  //glEnable(GL_CULL_FACE);
  //glCullFace(GL_BACK);
  tree1_model.Draw(basic_shader, Msend2, false);
  //glDisable(GL_CULL_FACE);
    

 glBindVertexArray(0);
 glUseProgram(0);
*/

 // DRAW ROCKS
/* basic_shader.Use();
 
 Msend = glm::mat4();
 Msend = glm::translate(Msend, glm::vec3(rocks[0].x,rocks[0].y,rocks[0].z));
 //Msend = glm::rotate(Msend, rocks[0].angle, glm::vec3(0.0, 1.0 , 0.0));
 //Msend = glm::rotate(Msend, rocks[0].angle*4.0f, glm::vec3(1.0, 0.0 , 0.0));
 Msend = glm::scale(Msend, glm::vec3(rocks[0].scale)); 

 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(Msend));
 glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionM));
 //glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(light_space_matrix));
 //glUniform1f(glGetUniformLocation(basic_shader.Program, "send_bias"), send_bias);


 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightPos[0]"),1, &lights[0].lightPos[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightColor[0]"),1, &lights[0].lightColor[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightSpecularColor[0]"),1, &lights[0].lightSpecularColor[0]);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "constant[0]"),1.0f);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "linear[0]"),  0.007);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "quadratic[0]"), 0.0002);
 

 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightPos[1]"),1, &lights[1].lightPos[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightColor[1]"),1, &lights[1].lightColor[0]);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightSpecularColor[1]"),1, &lights[1].lightSpecularColor[0]);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "constant[1]"),1.0f);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "linear[1]"),  0.5);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "quadratic[1]"), 0.9);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fire_intensity"), fire_intensity);


 glUniform1f(glGetUniformLocation(basic_shader.Program, "ambientSTR"), rocks[0].AmbientStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "diffuseSTR"), rocks[0].DiffuseStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "specularSTR"), rocks[0].SpecularStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "ShiniSTR"), rocks[0].ShiniStr);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "shadow_darkness"), rocks[0].shadow_darkness);
 
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "viewPos"), 1, &cameraPos[0]);

 glUniform1f(glGetUniformLocation(basic_shader.Program, "alpha"), rocks[0].alpha);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "var"), rocks[0].var);    
 glUniform1f(glGetUniformLocation(basic_shader.Program, "depth_test"), 0.0);    
 glUniform1f(glGetUniformLocation(basic_shader.Program, "face_cube"), -1.0);     

 glUniform4fv(glGetUniformLocation(basic_shader.Program, "fog_color"),1, &fog_color[0]);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fog_density"), fog_density);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fog_equation"), fog_equation);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "mid_fog_position"), 1, &mid_fog_position[0]);

  
  //glEnable(GL_CULL_FACE);
  //glCullFace(GL_BACK);
  rock1_model.Draw(basic_shader, Msend2, false);
  //glDisable(GL_CULL_FACE);
    

 glBindVertexArray(0);
 glUseProgram(0);*/




  ///////////////////////////////////////////////// DRAW PARTICULES     
  particule_shader.Use();   
  
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_particule);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, tex_depth_feu);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, tex_pre_rendu_feu);


  Msend = glm::mat4(1.0);
  Msend = glm::translate(Msend, glm::vec3(particules_pos.x,particules_pos.y,particules_pos.z));
  

  glm::mat4 temp(1.0f);
  glUniformMatrix4fv(glGetUniformLocation(particule_shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projectionM));
  glUniformMatrix4fv(glGetUniformLocation(particule_shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
  glUniformMatrix4fv(glGetUniformLocation(particule_shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(Msend));
  glUniform1f(glGetUniformLocation(particule_shader.Program, "test"), 1.0);
  glUniform1f(glGetUniformLocation(particule_shader.Program, "distance_camera"), glm::distance(glm::vec3(Feu.x,Feu.y,Feu.z), cameraPos));
  glUniform1f(glGetUniformLocation(particule_shader.Program, "blend_factor"), Particles -> blend_factor);
  glUniform1f(glGetUniformLocation(particule_shader.Program, "face_cube"), -1.0);
  
  Particles -> Draw(true,false);

  glUseProgram(0);  
  ///////////////////////////////////////////////

  if(VL_pre_rendering){
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

}


// fonction draw quad pour post process blur
GLuint quadVAO = 0;
GLuint quadVBO;
void RenderQuad()
{
  if (quadVAO == 0)
  {
    GLfloat quadVertices[] = {
      // Positions        // Texture Coords
      -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
      1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
      1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
    // Setup plane VAO
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
  }
  glBindVertexArray(quadVAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);
}



static void printFPS(void) {
  Uint32 t;
  static Uint32 t0 = 0, f = 0;
  f++;
  t = SDL_GetTicks();
  if(t - t0 > 1000) {
      fprintf(stderr, "%8.2f\n", (1000.0 * f / (t - t0)));
    t0 = t;
    f  = 0;
  }
}



// fonction qui charge la sky box
GLuint loadCubemap(vector<const GLchar*> faces){

  GLuint textureID;
  SDL_Surface * t = NULL;

  glGenTextures(1, &textureID);
  glActiveTexture(GL_TEXTURE0);

  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  for(GLuint i = 0; i < faces.size(); i++)
  {
        //image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
    t = IMG_Load(faces[i]);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels);
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

  return textureID;
}

// fonction qui genere la sphere
static GLfloat * buildSphere(int longitudes, int latitudes) {
  int i, j, k;
  GLfloat theta, phi, r[2], x[2], y[2], z[2], * data;
  GLfloat c2MPI_Long = 2.0 * myPI / longitudes;
  GLfloat cMPI_Lat = myPI / latitudes;
  //data = malloc(((6 * 6 * longitudes * latitudes )) * sizeof *data);
  data = new float[(6 * 6 * longitudes * latitudes )];
  /* assert(data); */
  for(i = 0, k = 0; i < latitudes; i++) {
    phi  = -myPI_2 + i * cMPI_Lat;
    y[0] = sin(phi);
    y[1] = sin(phi + cMPI_Lat);
    r[0] = cos(phi);
    r[1] = cos( phi  + cMPI_Lat);
    for(j = 0; j < longitudes; j++){
      theta = j * c2MPI_Long;
      x[0] = cos(theta);
      x[1] = cos(theta + c2MPI_Long);
      z[0] = sin(theta);
      z[1] = sin(theta + c2MPI_Long);


                  // coordonné de vertex                                                        
      data[k++] = r[0] * x[0]; data[k++] = y[0];  data[k++] = r[0] * z[0];      data[k++] = 0.68;  data[k++] = 0.0;  data[k++] = 0.0;
      data[k++] = r[1] * x[1]; data[k++] = y[1];  data[k++] = r[1] * z[1];      data[k++] = 0.68;  data[k++] = 0.0;  data[k++] = 0.0;
      data[k++] = r[0] * x[1]; data[k++] = y[0];  data[k++] = r[0] * z[1];      data[k++] = 0.68;  data[k++] = 0.0;  data[k++] = 0.0;

      data[k++] = r[0] * x[0]; data[k++] = y[0];  data[k++] = r[0] * z[0];      data[k++] = 0.68;  data[k++] = 0.0;  data[k++] = 0.0;
      data[k++] = r[1] * x[0]; data[k++] = y[1];  data[k++] = r[1] * z[0];      data[k++] = 0.68;  data[k++] = 0.0;  data[k++] = 0.0;
      data[k++] = r[1] * x[1]; data[k++] = y[1];  data[k++] = r[1] * z[1];      data[k++] = 0.68;  data[k++] = 0.0;  data[k++] = 0.0;


    }
  }
  return data;
}



void fire_script(){

static int state = 1;
static int state2 = 1;
static float acc = 0.5;
/*
  if(glm::distance(cameraPos,particules_pos) > 4.0){
    Mix_Volume(3,0);
  }else{
    Mix_Volume(3,(MIX_MAX_VOLUME*0.5)/(glm::distance(cameraPos,particules_pos)*2.0));
  }

  if(!Mix_Playing(3)){  
    Mix_PlayChannel( 3, S_fire, 0);
  }*/

  //////////////////////////
  
  // dynamic intensity
  if(fire_intensity < 1.0 && state == 0){
    fire_intensity += 0.8 * (ground->dt*-1);
  }

  if(fire_intensity > 0.75 && state == 1){
      fire_intensity -= 0.8 * (ground->dt*-1);
  }

  if(fire_intensity <= 0.75){
    state = 0;
  }
  if(fire_intensity >= 1.0){
    state = 1;
  }

  // dynamic pos fire light source
  if(acc <= 0.25){
    state2 = 1;
  }
  if(acc >= 0.75){
    state2 = 0;
  }

  if(state2 == 0){
    acc -= (ground->dt*-1)*2;
    lights[1].lightPos.z -= ((ground->dt*-1)*0.3);  
    lights[1].lightPos.y -= ((ground->dt*-1)*0.15);  
  }

  if(state2 == 1){
    acc += (ground->dt*-1)*2;
    lights[1].lightPos.z += ((ground->dt*-1)*0.3);
    lights[1].lightPos.y += ((ground->dt*-1)*0.15);
  }
  //std::cout << "acc = " << acc << std::endl;


  
}




void SetBoneTransform(uint Index, const glm::mat4& Transform, int val){

    assert(Index < MAX_BONES);
 
    if(val == 1){
       glUniformMatrix4fv(m_boneLocation[Index], 1, /*GL_TRUE*/ GL_FALSE, /*(const GLfloat*)Transform*/ glm::value_ptr(Transform));       
       //printf("index = %d\n", Index);
      //print_mat4(Transform);
    
    }

    if(val == 2){
       glUniformMatrix4fv(m_boneLocation2[Index], 1, /*GL_TRUE*/ GL_FALSE, /*(const GLfloat*)Transform*/ glm::value_ptr(Transform));       
    }
 
}



// fonction qui gere colision fenetre + colision entre objet, + gere la gravité des objet + gere tout les deplacement x/y
static void mobile_move(objet * tabl,int nb) {
  /* static int t0 = 0; */
  static int ft = 0;
  static int start;

  int t, i, j;


  if(ft==0){
    ft=1;
    start = SDL_GetTicks();
  }

  t = (SDL_GetTicks()) - start;                          

  //std::cout << "prog current t = " << t << std::endl;
    

  // boucle gère les variables temporelles de chaque objet
  for(i=0;i<nb;i++){

    j=t-tabl[i].bouge;

    if( j>0 ){

      if(tabl[i].start==0.0){
        tabl[i].start=SDL_GetTicks();
      }

      tabl[i].t =  (double)((SDL_GetTicks()) - tabl[i].start);

      tabl[i].dt= (tabl[i].t0 - tabl[i].t) / 1000.0;

      tabl[i].t0=tabl[i].t;

    }
    
  }


  // BOUCLE DEPLACEMENT (non use)
 /* for(i = 0; i < nb; i++) {
    
    tabl[i].vy+= tabl[i].poid*(-(9.81) * tabl[i].dt);    

    tabl[i].x  += ((tabl[i].vx) * tabl[i].dt);
    tabl[i].y  += ((tabl[i].vy) * tabl[i].dt);
    tabl[i].z  += ((tabl[i].vz) * tabl[i].dt);

  }*/

}




