#version 330

#define NB_LIGHTS 10


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

uniform float alpha;
uniform float var;
uniform float depth_test;
uniform float face_cube;
uniform float send_bias;

uniform float fog_density;
uniform float fog_equation;
uniform vec4 fog_color;
uniform vec3 mid_fog_position;


uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_specular2;
uniform sampler2D texture_normal1;
uniform sampler2D texture_metalness;
uniform sampler2D depth_map_particle;
uniform sampler2D tex_render_particle;
uniform samplerCube reflection_cubeMap;
uniform sampler2D shadow_map1;


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



void main(void) {

  if(depth_test != 1.0){

    vec3 color;
    float final_alpha;

    color = texture(texture_diffuse1, TexCoord).rgb;
    
   /* if(var == 7.0){
      //color = texture(texture_specular1, TexCoord).rgb;
      color = vec3(1.0);
    }*/

    //color = vec3(1.0);

    final_alpha = alpha;


    // LIGHT CALCULATION
    vec3 norm = normalize(vsoNormal);
    // normal mapping
    if(var == 5.0 || var == 3.0 || var == 4.0 || var == 0.0 || (var == 2.0 /*&& ShiniSTR != 1.0*/)){
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


    // FINAL LIGHT
    vec3 result = (LightRes1.ambient + LightRes1.diffuse + LightRes1.specular);
    if(var != 0.0){
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
     
      // get blurred reflection fragment
     vec4 sum = vec4(0.0);
     vec3 tc = R;
     float resolution = 1024 * 2.0;
     float radius = 20.0;
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

     float factor = (color_reflect.r + color_reflect.g + (color_reflect.b)) / 3.0f;
     //factor = 1.0;
     //factor = (color_reflect.r + color_reflect.g) / 2.0;

     vec3 metalness_factor = texture(texture_metalness, TexCoord).rgb;
     float final_factor = (metalness_factor.r + metalness_factor.g + metalness_factor.b) / (3.0f);

     result = mix(result, color_reflect, (final_factor) * factor);

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
    if(var == 4.0 /*&& face_cube == -1.0*/){

      vec2 TexCoords2 = (position_for_tex.xy / position_for_tex.w);
      TexCoords2 = TexCoords2 * 0.5 + 0.5;

      float depth_feu = texture(depth_map_particle, TexCoords2).r;

      if(/*gl_FragCoord.z < depth_feu &&*/ depth_feu != 1.0){


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
          //result = vec3(0.0,1.0,0.0);

        }
      }
    }

    fragColor = vec4(result, final_alpha);
    // ADD FOG
    if(var == 2.0 || var == 0.0){
      //float FogCoord = abs(EyeSpacePos.z/EyeSpacePos.w);
      float temp_dist = distance(FragPos, mid_fog_position);
      fragColor = mix(fragColor, fog_color, FogCalculation(0.0,0.0,fog_density,fog_equation,/*FogCoord*/ temp_dist));  
    }

  }
  
}
