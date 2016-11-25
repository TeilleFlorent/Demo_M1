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


static GLint m_boneLocation[MAX_BONES];
static GLint m_boneLocation2[MAX_BONES];
static float Start_zombie_anim = 10000000000;
static bool zombie_run_state = false;


// HEIGHT MAP    
CMultiLayeredHeightmap MyMap;


/*!\brief identifiant du (futur) vertex array object */
static GLuint skyboxVAO = 0;
static GLuint lampVAO = 0;
static GLuint groundVAO = 0;
static GLuint screenVAO = 0;

/*!\brief identifiant du (futur) buffer de data */
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
static GLuint tex_depth_house = 0;


float depth_map_res_seed = /*2048.0*/ 1024;
float depth_map_res_x, depth_map_res_y, depth_map_res_x_house, depth_map_res_y_house;
static GLuint reflection_cubeMap = 0;
float reflection_cubeMap_res = /*2048.0*/ 1024;

//dimension fenetre SDL
static int w = 800 * 1.5;
static int h = 600 * 1.5;


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


/////////////////////////////////////////////////////////


// FONCTION MAIN
int main() {

  srand(time(NULL));

 
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    fprintf(stderr, "Erreur lors de l'initialisation de SDL :  %s", SDL_GetError());
    return -1;
  }
  atexit(SDL_Quit);


/*  initAudio();

  load_audio();*/


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
    glUseProgram(0);

    skinning_shader.Use();
    glUniform1i(glGetUniformLocation(skinning_shader.Program, "texture_diffuse1"), 0);
    glUniform1i(glGetUniformLocation(skinning_shader.Program, "texture_specular1"), 1);
    glUniform1i(glGetUniformLocation(skinning_shader.Program, "texture_normal1"), 2);
    glUniform1i(glGetUniformLocation(skinning_shader.Program, "reflection_cubeMap"), 5);
    glUniform1i(glGetUniformLocation(skinning_shader.Program, "shadow_map1"), 3);
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
        SDL_WINDOW_SHOWN /*| SDL_WINDOW_FULLSCREEN*/)) == NULL )
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

  // fonction qui paramettre de l'openGL
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

  -1.0,1.0,0.0,
  -0.4,1.0,0.0,
  -1.0,0.2,0.0,  
  -0.4,0.2,0.0,
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
  depth_map_res_x_house = depth_map_res_seed * 2;
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
lights[1].lightPos = glm::vec3(particules_pos.x,particules_pos.y + 0.08f,particules_pos.z);
lights[1].save_lightPos = lights[1].lightPos;

// fake light pos pour shadow house;
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
 paladin.SpecularStr = 0.3;
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
  int w2, h2;

  SDL_GetWindowSize(win, &w, &h);
  glViewport(0, 0, /*800*1.5*/w, /*600*1.5*/h);

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

    SDL_GL_SwapWindow(win);


    //printf("walk_speed = %f\n", walk_speed);

    //printf("1cameraX = %f, cameraY = %f, cameraZ = %f\n",cameraPos.x,cameraPos.y, cameraPos.z); 
    //printf("2cameraX = %f, cameraY = %f, cameraZ = %f\n",cameraFront.x,cameraFront.y, cameraFront.z);
    //printf("yaw = %f, pitch = %f\n", yaw, pitch); 

    //printf("lightX = %f, Y = %f, Z = %f\n",lightPos.x, lightPos.y, lightPos.z);
   
    lights[1].lightPos = glm::vec3(particules_pos.x,particules_pos.y + 0.08f,particules_pos.z);
    Feu.x=particules_pos.x;
    Feu.y= Feu.y= particules_pos.y - 0.11;
    Feu.z=particules_pos.z;

    }

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

  if(RUN_MODE == 0 || RUN_MODE == 2){


  if(fly_state == true)
    camera_speed = walk_speed*6;

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

///////////////////////////////////

 float temp1,temp2,temp3, temp;
 glm::vec3 p1,p2,p3,p_temp,p_calc;

 p1 = glm::vec3((MyMap.VertexData[0][0].x*ground->scale), (MyMap.VertexData[0][0].y*ground->scale), (MyMap.VertexData[0][0].z*ground->scale));
 p2 = glm::vec3((MyMap.VertexData[0][1].x*ground->scale), (MyMap.VertexData[0][1].y*ground->scale), (MyMap.VertexData[0][1].z*ground->scale));
 p3 = glm::vec3((MyMap.VertexData[0][2].x*ground->scale), (MyMap.VertexData[0][2].y*ground->scale), (MyMap.VertexData[0][2].z*ground->scale));


 temp1 = glm::distance(glm::vec3(p1.x, 0.0, p1.z), glm::vec3(cameraPos.x, 0.0, cameraPos.z));
 temp2 = glm::distance(glm::vec3(p2.x, 0.0, p2.z), glm::vec3(cameraPos.x, 0.0, cameraPos.z));

 if(temp2 < temp1){
  p_temp = p1;
  p1 = p2;
  p2 = p_temp;
 }

 temp1 = glm::distance(glm::vec3(p1.x, 0.0, p1.z), glm::vec3(cameraPos.x, 0.0, cameraPos.z));
 temp2 = glm::distance(glm::vec3(p2.x, 0.0, p2.z), glm::vec3(cameraPos.x, 0.0, cameraPos.z));
 temp3 = glm::distance(glm::vec3(p3.x, 0.0, p3.z), glm::vec3(cameraPos.x, 0.0, cameraPos.z));

 if(temp3 < temp1){

  p_temp = p3;
  p3 = p2;
  p2 = p1;
  p1 = p_temp;

 }else{ 

  if(temp3 < temp2 && temp3 > temp1){

    p_temp = p3;
    p3 = p2;
    p2 = p_temp;

  }

 }


  for(int i = 0; i < MyMap.iRows; i++){
    for(int j = 0; j < MyMap.iCols; j++){
      
      //if((cameraPos.x < (MyMap.VertexData[i][j].x*ground->scale)+0.99 && cameraPos.x > (MyMap.VertexData[i][j].x*ground->scale)-0.99) 
      //&& (cameraPos.z < (MyMap.VertexData[i][j].z*ground->scale)+0.99 && cameraPos.z > (MyMap.VertexData[i][j].z*ground->scale)-0.99)){
      //  cameraPos.y = (MyMap.VertexData[i][j].y*ground->scale)+(ground->y)+0.4;
      //}

     temp1 = glm::distance(glm::vec3(p1.x, 0.0, p1.z), glm::vec3(cameraPos.x, 0.0, cameraPos.z));
     temp2 = glm::distance(glm::vec3(p2.x, 0.0, p2.z), glm::vec3(cameraPos.x, 0.0, cameraPos.z));
     temp3 = glm::distance(glm::vec3(p3.x, 0.0, p3.z), glm::vec3(cameraPos.x, 0.0, cameraPos.z));


        p_calc = glm::vec3((MyMap.VertexData[i][j].x*ground->scale), (MyMap.VertexData[i][j].y*ground->scale), (MyMap.VertexData[i][j].z*ground->scale)); 
        temp = glm::distance(glm::vec3(p_calc.x, 0.0, p_calc.z), glm::vec3(cameraPos.x, 0.0, cameraPos.z));


        if(temp < temp1){

          p3 = p2;
          p2 = p1;
          p1 = p_calc;

        }else{ 

          if(temp < temp2 && temp > temp1){

            p3 = p2;
            p2 = p_calc;

          }else{
            if(temp < temp3 && temp > temp2){

              p3 = p_calc;

            }
          }

        }


    }
  }

  if(fly_state == false){
      cameraPos.y = MyBarryCentric(p1,p2,p3, glm::vec2(cameraPos.x,cameraPos.z)) + ground->y + taille;
  }

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
     trees[0].x += 0.1;
     break;

     case 'e' :
     trees[0].x -= 0.1;
     break;

     case 't' :
     trees[0].z += 0.2;
     break;

     case 'y' :
     trees[0].z -= 0.2;
     break;

     case 'w' :
     trees[0].y +=0.01;
     break;

     case 'x' :
     trees[0].y -=0.01;
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
      initData();
      resizeGL(win);    

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
 screen_shader.Use();
 glActiveTexture(GL_TEXTURE0);
 glBindTexture(GL_TEXTURE_2D, tex_depth_house);

 glBindVertexArray(screenVAO);

 glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

 glBindVertexArray(0);
 glUseProgram(0);
 



 // DRAW SKYBOX 
 glDepthMask(GL_FALSE); // desactivé juste pour draw la skybox
 skybox_shader.Use();   
 glm::mat4 SkyboxViewMatrix = glm::mat4(glm::mat3(viewMatrix));  // Remove any translation component of the view matrix
 glUniformMatrix4fv(glGetUniformLocation(skybox_shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(SkyboxViewMatrix));
 glUniformMatrix4fv(glGetUniformLocation(skybox_shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projectionM));
 glUniform1f(glGetUniformLocation(skybox_shader.Program, "alpha"), skybox_alpha);
 glUniform1f(glGetUniformLocation(skybox_shader.Program, "is_foggy"), 1.0);
 
 glBindVertexArray(skyboxVAO);
 glActiveTexture(GL_TEXTURE0);
 glUniform1i(glGetUniformLocation(skybox_shader.Program, "skybox"), 0); // envoi du sampler cube 
 glBindTexture(GL_TEXTURE_CUBE_MAP, tex_cube_map /*reflection_cubeMap*/); // bind les 6 textures du cube map
 
 glEnable(GL_BLEND);
 glDrawArrays(GL_TRIANGLES, 0, 36);
 glDisable(GL_BLEND);

 glBindVertexArray(0);
 glDepthMask(GL_TRUE);  // réactivé pour draw le reste
 glUseProgram(0);


 // pre rendu feu
 glViewport(0, 0, depth_map_res_x, depth_map_res_y);
 Pre_rendu_feu(projectionM2, viewMatrix, -1.0f);
 
 //pre rendu reflection cube map
 glViewport(0, 0, reflection_cubeMap_res, reflection_cubeMap_res);
 Pre_rendu_cubeMap();

 //pre rendu shadow
 glViewport(0, 0, depth_map_res_x_house, depth_map_res_y_house);
 Pre_rendu_shadow_house(projectionM3, viewMatrix);


  //rendu normal        
 glViewport(0, 0, w, h);
 RenderShadowedObjects();

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

  
  int face = 0;
  glm::mat4 projectionM,projectionM2,Msend;

  std::vector<glm::mat4> cubeMap_viewMatrices;
  projectionM = glm::perspective(89.5373f, (float)reflection_cubeMap_res/(float)reflection_cubeMap_res, 0.1f, 1000.0f);
  projectionM2 = glm::perspective(45.0f, (float)reflection_cubeMap_res/(float)reflection_cubeMap_res, 0.1f, 1000.0f);
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


  //glViewport(0, 0, reflection_cubeMap_res, reflection_cubeMap_res);

  glBindFramebuffer(GL_FRAMEBUFFER, reflection_cubeMap_FBO);
  

  for (int i = 0; i < 6; ++i){
   
    glBindFramebuffer(GL_FRAMEBUFFER, 0);      
    //glViewport(0, 0, depth_map_res_x, depth_map_res_y);
    if(i == 5){
      Pre_rendu_feu(projectionM, cubeMap_viewMatrices[i], i);
    }
    //glViewport(0, 0, reflection_cubeMap_res, reflection_cubeMap_res);
    glBindFramebuffer(GL_FRAMEBUFFER, reflection_cubeMap_FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i , reflection_cubeMap, 0);


    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i , reflection_cubeMap, 0);
   

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


      feu_model.Draw(basic_shader, glm::mat4(1.0), false);

      glBindVertexArray(0);
      glUseProgram(0);
    }

    // DRAW House
    /*basic_shader.Use();

    Msend = glm::mat4();

    Msend = glm::translate(Msend, glm::vec3(house.x,house.y,house.z));
    Msend = glm::rotate(Msend, house.angle, glm::vec3(0.0, 1.0 , 0.0));
    Msend = glm::scale(Msend, glm::vec3(house.scale)); 


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
    glUniform1f(glGetUniformLocation(basic_shader.Program, "linear[1]"),  0.5);
    glUniform1f(glGetUniformLocation(basic_shader.Program, "quadratic[1]"), 0.9);
    glUniform1f(glGetUniformLocation(basic_shader.Program, "fire_intensity"), fire_intensity);


    glUniform1f(glGetUniformLocation(basic_shader.Program, "ambientSTR"), house.AmbientStr);
    glUniform1f(glGetUniformLocation(basic_shader.Program, "diffuseSTR"), house.DiffuseStr);
    glUniform1f(glGetUniformLocation(basic_shader.Program, "specularSTR"), house.SpecularStr);
    glUniform1f(glGetUniformLocation(basic_shader.Program, "ShiniSTR"), house.ShiniStr);
   
    glUniform3fv(glGetUniformLocation(basic_shader.Program, "viewPos"), 1, &cameraPos[0]);

    glUniform1f(glGetUniformLocation(basic_shader.Program, "alpha"), house.alpha);
    glUniform1f(glGetUniformLocation(basic_shader.Program, "var"), house.var);    
    glUniform1f(glGetUniformLocation(basic_shader.Program, "depth_test"), 0.0);    
    glUniform1f(glGetUniformLocation(basic_shader.Program, "face_cube"), i);     
  

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    house_model.Draw(basic_shader, glm::mat4(1.0), false);
    glDisable(GL_CULL_FACE);
    

    glBindVertexArray(0);
    glUseProgram(0);
*/


    
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

    glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(/*viewMatrix*/ lightView));
    glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(Msend));
    glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(/*projectionMatrix*/ lightProjection));

 /* glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightPos[0]"),1, &lights[0].lightPos[0]);
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


  glUniform1f(glGetUniformLocation(basic_shader.Program, "ambientSTR"), house.AmbientStr);
  glUniform1f(glGetUniformLocation(basic_shader.Program, "diffuseSTR"), house.DiffuseStr);
  glUniform1f(glGetUniformLocation(basic_shader.Program, "specularSTR"), house.SpecularStr);
  glUniform1f(glGetUniformLocation(basic_shader.Program, "ShiniSTR"), house.ShiniStr);*/
 
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
 //glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(light_space_matrix));
 //glUniform1f(glGetUniformLocation(basic_shader.Program, "send_bias"), send_bias);


/*    glUniform3fv(glGetUniformLocation(basic_shader.Program, "LightPos[0]"),1, &lights[0].lightPos[0]);
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
 glUniform1f(glGetUniformLocation(basic_shader.Program, "shadow_darkness"), sword.shadow_darkness);*/
 
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "viewPos"), 1, &cameraPos[0]);

 glUniform1f(glGetUniformLocation(basic_shader.Program, "alpha"), sword.alpha);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "var"), sword.var);    
 glUniform1f(glGetUniformLocation(basic_shader.Program, "depth_test"), 1.0);    
 glUniform1f(glGetUniformLocation(basic_shader.Program, "face_cube"), -1.0);     

 /*glUniform4fv(glGetUniformLocation(basic_shader.Program, "fog_color"),1, &fog_color[0]);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fog_density"), fog_density);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fog_equation"), fog_equation);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "mid_fog_position"), 1, &mid_fog_position[0]);*/

  
  //glEnable(GL_CULL_FACE);
  //glCullFace(GL_BACK);
 sword_model.Draw(basic_shader, glm::mat4(1.0), false);
  //glDisable(GL_CULL_FACE);
    

 glBindVertexArray(0);
 glUseProgram(0);

 glBindFramebuffer(GL_FRAMEBUFFER, 0);   



  }

}


void RenderShadowedObjects(){


 glm::mat4 projectionM,Msend,viewMatrix,Msend2;

 projectionM = glm::perspective(45.0f, /* 4.0f/3.0f */(float)w/(float)h, 0.1f, 1000.0f);
 viewMatrix=glm::lookAt(cameraPos, (cameraPos) + cameraFront, cameraUp); 


 glm::mat4 lightProjection, lightView, light_space_matrix;
 GLfloat far =  glm::distance(lights[2].lightPos, glm::vec3(house.x,house.y,house.z)) * 1.2f;
 lightProjection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 1.0f, far);
 lightView = glm::lookAt(lights[2].lightPos, glm::vec3(house.x,house.y,house.z) , glm::vec3(0.0,1.0,0.0));
 light_space_matrix = lightProjection * lightView;
 glm::vec3 mid_fog_position = glm::vec3(house.x,house.y,house.z);



  // DRAW LAMP
 /*lamp_shader.Use();
 
 glBindVertexArray(lampVAO);

 Msend= glm::mat4();
 Msend = glm::translate(Msend, lights[1].lightPos);
 Msend = glm::scale(Msend, glm::vec3(0.01f)); 

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
    /*glUniform1f(glGetUniformLocation(caillou_shader.Program, "fStart"), fog_Start);
    glUniform1f(glGetUniformLocation(caillou_shader.Program, "fEnd"), fog_End);*/
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fog_density"), fog_density);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fog_equation"), fog_equation);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "mid_fog_position"), 1, &mid_fog_position[0]);
 
  
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
 vector<glm::mat4> Transforms;

 float start_anim = 0.0;

//printf("t = %f\n", zombie[0].t);
        
 float RunningTime1 = (float)((double)ground->t - ((double)0.0f+start_anim))/1000.0f; 
 //float RunningTime2 = (float)((double)zombie[1].t - ((double)4600.0f+start_anim))/1000.0f; 
 //float RunningTime3 = ((float)((double)zombie[1].t - ((double)6700.0f+4600.0+start_anim))/1000.0f); 
 //float RunningTime4 = ((float)((double)zombie[1].t - ((double)6700.0f+4600.0+3700.0+start_anim))/1000.0f); 

  //std::cout << "t = " << RunningTime1 << std::endl;    

 /*if(zombie[0].t < start_anim)
  paladin_skinned.BoneTransform(0.0, Transforms, paladin_sitting_idle);*/
  
  
  if(RunningTime1 >= 0.0 /*&& RunningTime1 <= 1000.0*/){
    paladin_skinned.BoneTransform(RunningTime1, Transforms, /*paladin_sitting_idle*/ /*paladin_standing_up*/ /*paladin_breathing_idle*/ paladin_warrior_idle);
  }

  /*if(RunningTime2 >= 0.0 && RunningTime2 < 4.6+6.7+(start_anim/1000.0)){

    zombie_model.BoneTransform(RunningTime2, Transforms, zombie_attack);
  }

  if(RunningTime3 >= 0.0 && RunningTime3 < 4.6+6.7+3.7+(start_anim/1000.0)){

    zombie_model.BoneTransform(RunningTime3+0.28f, Transforms, zombie_scream);
  }

  if(RunningTime4 >= 0.0 ){

    zombie_model.BoneTransform(RunningTime4, Transforms, zombie_run);
  }*/
         
  //std::cout << "size = " << Transforms.size() << std::endl;    



 skinning_shader.Use();

 glActiveTexture(GL_TEXTURE5); 
 glBindTexture(GL_TEXTURE_CUBE_MAP, reflection_cubeMap);
 glActiveTexture(GL_TEXTURE3);
 glBindTexture(GL_TEXTURE_2D, tex_depth_house);
 

 Msend = glm::mat4();

 Msend = glm::translate(Msend, glm::vec3(paladin.x,paladin.y,paladin.z));
 Msend = glm::rotate(Msend, paladin.angle, glm::vec3(0.0 , 1.0 , 0.0));
 Msend = glm::rotate(Msend, paladin.acca, glm::vec3(1.0 , 0.0 , 0.0));
 Msend = glm::scale(Msend, glm::vec3(paladin.scale)); 

 for (uint i = 0 ; i < Transforms.size() ; i++) {
  SetBoneTransform(i, Transforms[i],1);
 }

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
   
 glEnable(GL_CULL_FACE);
 glCullFace(GL_BACK); 
 paladin_skinned.Render(); 
 glDisable(GL_CULL_FACE);

 glBindVertexArray(0);
 glUseProgram(0);




/////////////////////////////////// DRAW HOUSE
 basic_shader.Use();

 glActiveTexture(GL_TEXTURE6);
 glBindTexture(GL_TEXTURE_2D, tex_depth_house);

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

 glUniform4fv(glGetUniformLocation(basic_shader.Program, "fog_color"),1, &fog_color[0]);
    /*glUniform1f(glGetUniformLocation(caillou_shader.Program, "fStart"), fog_Start);
    glUniform1f(glGetUniformLocation(caillou_shader.Program, "fEnd"), fog_End);*/
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fog_density"), fog_density);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fog_equation"), fog_equation);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "mid_fog_position"), 1, &mid_fog_position[0]);

  
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
    /*glUniform1f(glGetUniformLocation(caillou_shader.Program, "fStart"), fog_Start);
    glUniform1f(glGetUniformLocation(caillou_shader.Program, "fEnd"), fog_End);*/
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fog_density"), fog_density);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fog_equation"), fog_equation);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "mid_fog_position"), 1, &mid_fog_position[0]);

  
  //glEnable(GL_CULL_FACE);
  //glCullFace(GL_BACK);
 sword_model.Draw(basic_shader, Msend2, false);
  //glDisable(GL_CULL_FACE);
    

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
    /*glUniform1f(glGetUniformLocation(caillou_shader.Program, "fStart"), fog_Start);
    glUniform1f(glGetUniformLocation(caillou_shader.Program, "fEnd"), fog_End);*/
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fog_density"), fog_density);
 glUniform1f(glGetUniformLocation(basic_shader.Program, "fog_equation"), fog_equation);
 glUniform3fv(glGetUniformLocation(basic_shader.Program, "mid_fog_position"), 1, &mid_fog_position[0]);

  
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



float MyBarryCentric(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec2 pos) {
/*
      std::cout << "p1 = " << p1.x << " "  << p1.y << " " << p1.z << std::endl;    
  std::cout << "p2 = " << p2.x << " "  << p2.y << " " << p2.z << std::endl;    
  std::cout << "p3 = " << p3.x << " "  << p3.y << " " << p3.z << std::endl;    
*/

  float det = (p2.z - p3.z) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.z - p3.z);
    //printf("det = %f\n", det);
  float l1 = ((p2.z - p3.z) * (pos.x - p3.x) + (p3.x - p2.x) * (pos.y - p3.z)) / det;
    //printf("l1 = %f\n", l1);
  float l2 = ((p3.z - p1.z) * (pos.x - p3.x) + (p1.x - p3.x) * (pos.y - p3.z)) / det;
    //printf("l2 = %f\n", l2);
  float l3 = 1.0f - l1 - l2;
    //printf("l3 = %f\n", l3);

  return l1 * p1.y + l2 * p2.y + l3 * p3.y;
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

  // dynamic poc fire light source
  if(acc <= 0.0){
    state2 = 1;
  }
  if(acc >= 1.0){
    state2 = 0;
  }

  if(state2 == 0){
    acc -= (ground->dt*-1)*4;
    lights[1].lightPos.z -= ((ground->dt*-1)*0.3);  
    lights[1].lightPos.y -= ((ground->dt*-1)*0.3);  
  }

  if(state2 == 1){
    acc += (ground->dt*-1)*4;
    lights[1].lightPos.z += ((ground->dt*-1)*0.3);
    lights[1].lightPos.y += ((ground->dt*-1)*0.3);
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




