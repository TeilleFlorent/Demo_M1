#version 330

#define NB_LIGHTS 10
#define G_SCATTERING 0.2
#define PI 3.14159265358979323846264338
#define NB_STEPS 10

struct LightRes {    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular; 
};

//in vec3 vsoColor;
in vec2 TexCoord;
in vec3 vsoNormal;
in vec3 FragPos;
in vec4 position_for_tex;
in vec3 Tangent;
in vec4 FragPosLightSpace;
smooth in vec4 EyeSpacePos;


out vec4 fragColor;


uniform vec3 LightPos[NB_LIGHTS];
uniform vec3 LightColor[NB_LIGHTS];
uniform vec3 LightSpecularColor[NB_LIGHTS];
uniform float constant[NB_LIGHTS];
uniform float linear[NB_LIGHTS];
uniform float quadratic[NB_LIGHTS];

uniform vec3 viewPos;

uniform float ShiniSTR;
uniform float ambientSTR;
uniform float diffuseSTR;
uniform float specularSTR;
uniform float shadow_darkness;

uniform float fire_intensity;
uniform float VL_intensity;

uniform float alpha;
uniform float var;
uniform float depth_test;
uniform float face_cube;
uniform float send_bias;
uniform int shadow_point_light;

uniform float fog_density;
uniform float fog_equation;
uniform vec4 fog_color;
uniform vec3 mid_fog_position;

uniform mat4 lightSpaceMatrix;


uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_specular2;
uniform sampler2D texture_normal1;
uniform sampler2D texture_metalness;
uniform sampler2D depth_map_particle;
uniform sampler2D tex_render_particle;
uniform samplerCube reflection_cubeMap;
uniform sampler2D shadow_map1;
uniform samplerCube shadow_cube_map;



LightRes LightCalculation(int num_light, vec3 norm, vec3 color, vec3 light_color, vec3 light_specular_color){

  LightRes res;

  // ambient
  vec3 ambient = ambientSTR * color * light_color; 

  //diffuse
  vec3 diffuse = vec3(0.0,0.0,0.0); 
  vec3 lightDir = normalize(LightPos[num_light] - FragPos);          
  if(diffuseSTR > 0.0){
    float diff = max(dot(norm, lightDir),0.0);
    diffuse = diff * (diffuseSTR) * color * light_color;
  }
  //specular
  vec3 specular = vec3(0.0,0.0,0.0);
  if(specularSTR > 0.0){
    vec3 viewDir = normalize(viewPos - FragPos);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(norm, halfwayDir), 0.0), ShiniSTR);     
    
    if(var != 5.0){
      specular = spec * specularSTR * light_specular_color; 
    }else{
      specular = spec * specularSTR * texture(texture_specular2, TexCoord).rgb;
    }

    // add specular mapping
    if(var == 1.0)
      specular *= texture(texture_specular1, TexCoord).r;
    if(var == 5.0)
      specular *= texture(texture_metalness, TexCoord).r;    

    /*vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), ShiniSTR);
    vec3 specular = (spec * specularSTR) * LightSpecularColor[num_light];*/ 

  }

  // Attenuation
  float distance = length(LightPos[num_light] - FragPos);
  float attenuation = 1.0f / (constant[num_light] + linear[num_light] * distance + quadratic[num_light] * (distance * distance)); 


  if(var == 5.0){
    attenuation *= 1.5;
  }

  res.ambient = ambient;
  res.diffuse = diffuse;
  res.specular = specular;

  res.ambient *= attenuation;  
  res.diffuse *= attenuation;
  res.specular *= attenuation;  


  return res;
}





vec3 normal_mapping_calculation()                                                                     
{                                                               
    float fact = 1.0;
    if(var == 2.0 && ShiniSTR != 8.0){
      fact = 5.0;
    }

    vec3 Normal = normalize(vsoNormal);                                                       
    vec3 temp_tangent;
    if(var == 0.0){
      temp_tangent = normalize(vec3(1.0,0.0,0.0));                                                     
    }else{
      temp_tangent = normalize(Tangent);                                                     
    }
    temp_tangent = normalize(temp_tangent - dot(temp_tangent, Normal) * Normal);                           
    vec3 Bitemp_tangent = cross(temp_tangent, Normal);                                                
    vec3 BumpMapNormal = texture(texture_normal1, TexCoord*fact).xyz;                                
    BumpMapNormal = 2.0 * BumpMapNormal - vec3(1.0, 1.0, 1.0);                              
    vec3 NewNormal;                                                                         
    mat3 TBN = mat3(temp_tangent, Bitemp_tangent, Normal);                                            
    NewNormal = TBN * BumpMapNormal;                                                        
    NewNormal = normalize(NewNormal);                                                       
    return NewNormal;                                                                       
}



float ShadowCalculation1(vec4 fragPosLightSpace, vec3 norm, float darkness)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    vec2 uv = projCoords.xy;
    if (uv.x < 0. || uv.x > 1.0 || uv.y < 0. || uv.y > 1.0){
      return 1.0;
    }

    float closestDepth = texture(shadow_map1, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    
    float shadow = 0.0;
    if(closestDepth < currentDepth){
      shadow = 1.0;
    }

    // Calculate bias (based on depth map resolution and slope)
    /*vec3 normal = norm;
    vec3 lightDir = normalize(LightPos[2] - FragPos);
    float bias = max((0.05*2) * ((1.0) - dot(normal, lightDir)*2), (0.005)*2);*/
    //float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    
    float bias = send_bias;

    // PCF
    shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadow_map1, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadow_map1, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
  
    shadow /= 9.0;

    // Keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
        

    shadow = min(shadow, darkness);
    shadow = 1.0 - shadow;
    return shadow;
}



float FogCalculation(float fStart, float fEnd, float fDensity, float iEquation, float fFogCoord)
{
  float fResult = 0.0;
  if(iEquation == 0.0)
    fResult = (fEnd-fFogCoord)/(fEnd-fStart);
  else if(iEquation == 1.0)
    fResult = exp(-fDensity*fFogCoord);
  else if(iEquation == 2.0)
    fResult = exp(-pow(fDensity*fFogCoord, 2.0));
    
  fResult = 1.0-clamp(fResult, 0.0, 1.0);
  
  return fResult;
}


// scaterring calculé avec la fonction de Henyey-Greenstein
float ComputeScattering(float lightDotView){

  float result = 1.0f - G_SCATTERING * G_SCATTERING;
  result /= (4.0f * PI * pow(1.0f + G_SCATTERING * G_SCATTERING - (2.0f * G_SCATTERING) *      lightDotView, 1.5f));
  return result;

}


vec3 VolumetricLightCalculation(){

  vec3 acc = vec3(0.0);
  mat4 noise = mat4(vec4(0.0f, 0.5f, 0.125f, 0.625f),
    vec4(0.75f, 0.22f, 0.875f, 0.375f),
    vec4(0.1875f, 0.6875f, 0.0625f, 0.5625),
    vec4(0.9375f, 0.4375f, 0.8125f, 0.3125));


  vec3 frag_pos = FragPos;
  vec3 start_ray_position = viewPos;
  vec3 end_ray_position = frag_pos; // a modif => frag_pos de la premiere intersection 

  vec3 ray_vector = end_ray_position - start_ray_position;
  float ray_length = length(ray_vector);

  vec3 ray_direction = ray_vector / ray_length;
  float step_length = ray_length / NB_STEPS;

  vec3 step = ray_direction * step_length;

  vec3 current_ray_position = start_ray_position;

  for(int i = 0; i < NB_STEPS; i++){


    vec4 frag_pos_light_space = lightSpaceMatrix * vec4(current_ray_position, 1.0);

    vec3 projCoords = frag_pos_light_space.xyz / frag_pos_light_space.w;
    projCoords = projCoords * 0.5 + 0.5;

   
    float shadowMapValue = texture(shadow_map1, projCoords.xy).r;

    vec3 light_direction = normalize(LightPos[2] - /*FragPos*/ current_ray_position);
    
    if(shadowMapValue > projCoords.z)
    {
      acc += vec3(ComputeScattering(dot(ray_direction, light_direction))) * vec3(1.0,1.0,1.0);

    }
 
    // noise correction
    float scale = 1.0;
    float temp_x = gl_FragCoord.x * scale;
    float temp_y = gl_FragCoord.y * scale;
    temp_x = mod(temp_x, 4.0);
    temp_y = mod(temp_y, 4.0);
    float ditherValue = noise[int(temp_x)][int(temp_y)];

    current_ray_position += step * ditherValue;
  
  }

  acc /= NB_STEPS;

  return acc;
}


// tableau d'offset de direction
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1), 
   vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
   vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
   vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
);

float ShadowCubeMapCalculation(vec3 fragPos)
{
    vec3 fragToLight = fragPos - LightPos[1];
    float currentDepth = length(fragToLight);
  
    float shadow = 0.0;
    float bias = 0.001;
    int samples = /*20*/ 1;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / /*far_plane*/ 1000.0)) / /*25.0*/ 450;
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(shadow_cube_map, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= /*far_plane*/ 1000.0;  
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);
        
    return shadow;
}



void main(void) {
  // re init
  gl_FragDepth = gl_FragCoord.z;

  if(depth_test == 0.0){

    vec3 color;
    float final_alpha;

    if(var == 7.0){
      color = texture(texture_diffuse1, TexCoord).bgr;
    }else{
      color = texture(texture_diffuse1, TexCoord).rgb;
      if(var == 8.0)
        color *= vec3(1.0,0.85,1.0);
    }

    
    /*if(var == 9.0){
      //color = texture(texture_normal1, TexCoord).rgb;
      color = vec3(1.0);
    }*/

    //color = vec3(1.0);

    final_alpha = alpha;


    // LIGHT CALCULATION
    vec3 norm = normalize(vsoNormal);
    // normal mapping
    if(var == 9.0 || var == 7.0 || var == 5.0 || var == 3.0 || var == 4.0 || var == 0.0 || (var == 2.0 /*&& ShiniSTR != 1.0*/)){
       norm = normal_mapping_calculation();
    }

    LightRes LightRes1 = LightCalculation(0,norm,color,LightColor[0],LightSpecularColor[0] /*vec3(0.0,0.0,1.0)*/);
    LightRes LightRes2 = LightCalculation(1,norm,color,LightColor[1] ,LightSpecularColor[1] /*vec3(1.0,0.0,0.0)*/);


    // add fire variance intensity
    LightRes2.diffuse *= fire_intensity;
    LightRes2.specular *= fire_intensity;
    LightRes2.specular *= fire_intensity;


    // SHADOW CALCULATION
    if((var == 0.0 || var == 2.0 || var == 4.0) && face_cube == -1.0){
      float shadow_intensity = ShadowCalculation1(FragPosLightSpace, norm, shadow_darkness);
      
      LightRes1.diffuse *= shadow_intensity;
      LightRes1.specular *= shadow_intensity;
    }

    // POINT SHADOW CALCULATION
    if(var == 2.0 && shadow_point_light == 1){
      float shadow_intensity = ShadowCubeMapCalculation(FragPos) * 0.65;
      LightRes2.diffuse *= (1.0 - shadow_intensity);
      LightRes2.specular *= (1.0 - shadow_intensity);
      
      LightRes1.diffuse *= (1.0 - shadow_intensity);
      LightRes1.specular *= (1.0 - shadow_intensity);
    
    }


    // FINAL LIGHT
    vec3 result = (LightRes1.ambient + LightRes1.diffuse + LightRes1.specular);
    if(var != 0.0 && var != 6.0 && var != 7.0){
      result += (LightRes2.diffuse + LightRes2.specular);
      if(var == 5.0){
        result = (LightRes1.ambient + LightRes2.diffuse + LightRes2.specular);
      }
    }
  

    // ADD REFLECTION ON SWORD
    if(var == 3.0){

     vec3 I = normalize(FragPos - viewPos);
     vec3 R = reflect(vec3(I.x, I.y * - 1.0f, I.z), norm);
     
      // get blurred reflection fragment
     vec4 sum = vec4(0.0);
     vec3 tc = R;
     float resolution = 1024;
     float radius = 7.0;
     vec3 dir = vec3(1.0,1.0,1.0);

     float blur = radius / resolution;

     float hstep = dir.x;
     float vstep = dir.y;
     float zstep = dir.z;

     sum += texture(reflection_cubeMap, vec3(tc.x - 4.0*blur*hstep, tc.y - 4.0*blur*vstep, tc.z - 4.0*blur*zstep)) * 0.0162162162;
     sum += texture(reflection_cubeMap, vec3(tc.x - 3.0*blur*hstep, tc.y - 3.0*blur*vstep, tc.z - 3.0*blur*zstep)) * 0.0540540541;
     sum += texture(reflection_cubeMap, vec3(tc.x - 2.0*blur*hstep, tc.y - 2.0*blur*vstep, tc.z - 2.0*blur*zstep)) * 0.1216216216;
     sum += texture(reflection_cubeMap, vec3(tc.x - 1.0*blur*hstep, tc.y - 1.0*blur*vstep, tc.z - 1.0*blur*zstep)) * 0.1945945946;

     sum += texture(reflection_cubeMap, vec3(tc.x, tc.y, tc.z)) * 0.2270270270;

     sum += texture(reflection_cubeMap, vec3(tc.x + 1.0*blur*hstep, tc.y + 1.0*blur*vstep, tc.z - 1.0*blur*zstep)) * 0.1945945946;
     sum += texture(reflection_cubeMap, vec3(tc.x + 2.0*blur*hstep, tc.y + 2.0*blur*vstep, tc.z - 2.0*blur*zstep)) * 0.1216216216;
     sum += texture(reflection_cubeMap, vec3(tc.x + 3.0*blur*hstep, tc.y + 3.0*blur*vstep, tc.z - 3.0*blur*zstep)) * 0.0540540541;
     sum += texture(reflection_cubeMap, vec3(tc.x + 4.0*blur*hstep, tc.y + 4.0*blur*vstep, tc.z - 4.0*blur*zstep)) * 0.0162162162;

     //vec3 color_reflect = texture(reflection_cubeMap, R).rgb;
      vec3 color_reflect = sum.rgb;

     float factor = (color_reflect.r + color_reflect.g + color_reflect.b) / 3.0;
     factor = 1.0;

     vec3 metalness_factor = texture(texture_specular1, TexCoord).rgb;

     result = mix(result, color_reflect, metalness_factor.b * (factor));

    }

    // ADD REFLECTION ON SHIELD
    if(var == 5.0){

      vec3 I = normalize(vec3(FragPos.x ,FragPos.y,FragPos.z ) - vec3(viewPos.x, viewPos.y, viewPos.z));
      vec3 R = reflect(vec3(I.x * 1.0, I.y * 1.0, I.z * 1.0), norm * vec3(-0.2f));
     
      vec3 color_reflect = texture(reflection_cubeMap, R).rgb;

      float factor = (color_reflect.r + color_reflect.g + (color_reflect.b)) / 3.0f;
     //factor = 1.0;
     //factor = (color_reflect.r + color_reflect.g) / 2.0;

      vec3 metalness_factor = texture(texture_metalness, TexCoord).rgb;
      float final_factor = (metalness_factor.r + metalness_factor.g + metalness_factor.b) / (3.0f);

      result = mix(result, color_reflect, (final_factor) * factor * 1.4);

    }


    // ADD AO mapping
    if(var == 5.0 || var == 0.0 || (var == 2.0 && ShiniSTR == 8.0)){
      vec3 temp_AO = texture(texture_specular1, TexCoord).rgb;
      float temp = (temp_AO.r + temp_AO.g + temp_AO.b) / 3.0;
      result *= temp;
      if(var == 2.0 || var == 5.0)
        result *= temp;
    }


    // FAKE VOLUMETRIC PARTICLE (on feu mesh)
    if(var == 4.0 ){

      vec2 TexCoords2 = (position_for_tex.xy / position_for_tex.w);
      TexCoords2 = TexCoords2 * 0.5 + 0.5;

      float depth_feu = texture(depth_map_particle, TexCoords2).r;

      if(depth_feu != 1.0){


         // ADD COLOR FRAGMENT PARTICLE
         vec4 temp = texture(tex_render_particle, TexCoords2); 

         temp.r = temp.r*0.45*0.8 + temp.r*0.45*0.8;
         temp.g = temp.g*0.5*0.8 + temp.g*0.5*0.8;
         temp.b = temp.b*0.5*0.8 + temp.b*0.5*0.8;

      
         // remove too much dark fragment
         if(((temp.r + temp.g + temp.b)/3.0) > 0.25){
          temp.r *= 1.5;
          temp.g *= 1.5;
          temp.b *= 1.5;

          result = mix(result, temp.rgb, 0.75 * temp.a); // plus dist petite plus fact petit
         
        }
      }
    }

    // ADD VOLUMETRIC LIGHT
    if(VL_intensity > 0.0){
      float temp_res = VolumetricLightCalculation();
      //if(temp_res > 0.1)
        result += (temp_res * 6.5 * VL_intensity);
    }
   

    fragColor = vec4(result, final_alpha);

    // ADD FOG
    if(var == 0.0 || var == 6.0 || var == 7.0 || var == 8.0 || var == 9.0){
      //float FogCoord = abs(EyeSpacePos.z/EyeSpacePos.w);
      float temp_dist = distance(FragPos, mid_fog_position);
      fragColor = mix(fragColor, fog_color, FogCalculation(0.0,0.0,fog_density,fog_equation,/*FogCoord*/ temp_dist));  
    }

  }else{
    if(face_cube != -1.0){
     float far = 1000.0;
     if(var == 5.0)
      far = 1000.0 * 1.15;

     float lightDistance = length(FragPos - viewPos);
     lightDistance /= far;
     gl_FragDepth = lightDistance; //near * far / ((gl_FragCoord.z * (far - near)) - far);          
    }
  }
  
}
