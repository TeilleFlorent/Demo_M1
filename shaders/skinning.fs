#version 330

#define NB_LIGHTS 10


struct LightRes {    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular; 
};

in vec2 TexCoord;
in vec3 vsoNormal;
in vec3 FragPos;
in vec3 Tangent;
in vec4 FragPosLightSpace;


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
uniform float test_bias;
uniform int shadow_point_light;


uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
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
    specular = spec * specularSTR * light_specular_color; 
    // add specular mapping
    if(var == 1.0)
      specular *= texture(texture_specular1, TexCoord).r;

    /*vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), ShiniSTR);
    vec3 specular = (spec * specularSTR) * LightSpecularColor[num_light];*/ 

  }


  // Attenuation
  float distance = length(LightPos[num_light] - FragPos);
  float attenuation = 1.0f / (constant[num_light] + linear[num_light] * distance + quadratic[num_light] * (distance * distance)); 

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
    vec3 Normal = normalize(vsoNormal);                                                       
    vec3 temp_tangent = normalize(Tangent);                                                     
    temp_tangent = normalize(temp_tangent - dot(temp_tangent, Normal) * Normal);                           
    vec3 Bitemp_tangent = cross(temp_tangent, Normal);                                                
    vec3 BumpMapNormal = texture(texture_normal1, TexCoord).xyz;                                
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
    float bias = /*test_bias*/ 0.0413001;
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


  if(depth_test != 1.0){

    vec3 color;
    float final_alpha;

    
    color = texture(texture_diffuse1, TexCoord).rgb;

    //color = vec3(1.0,1.0,1.0);

    final_alpha = alpha;


    // LIGHT CALCULATION
    vec3 old_norm,norm;
    old_norm = normalize(vsoNormal);
    // normal mapping
    if(var == 1.0){
      norm = normal_mapping_calculation();
    }

    LightRes LightRes1 = LightCalculation(0,norm,color,LightColor[0],LightSpecularColor[0] /*vec3(0.0,0.0,1.0)*/);
    LightRes LightRes2 = LightCalculation(1,norm,color,LightColor[1] ,LightSpecularColor[1] /*vec3(1.0,0.0,0.0)*/);

    // add fire variance intensity
    LightRes2.diffuse *= fire_intensity;
    LightRes2.specular *= fire_intensity;
    LightRes2.specular *= fire_intensity;

     // SHADOW CALCULATION
    float shadow_intensity = ShadowCalculation1(FragPosLightSpace, norm, shadow_darkness);

    LightRes1.diffuse *= shadow_intensity;
    LightRes1.specular *= shadow_intensity;
    
    // POINT SHADOW CALCULATION
    if(shadow_point_light == 1){
      shadow_intensity = ShadowCubeMapCalculation(FragPos) * 1.0;
      LightRes2.diffuse *= (1.0 - shadow_intensity);
      LightRes2.specular *= (1.0 - shadow_intensity);
    //LightRes1.diffuse *= (1.0 - shadow_intensity);
    //LightRes1.specular *= (1.0 - shadow_intensity);
    }

    // FINAL LIGHT
    vec3 result = (LightRes1.ambient + LightRes1.diffuse + LightRes1.specular);
    result += (/*LightRes2.ambient*/ + LightRes2.diffuse*2.0 + LightRes2.specular*1.5);


    //ADD REFLECTION CUBEMAP (on Paladin)
    if(var == 1.0){
     vec3 I = normalize(FragPos - viewPos);
     vec3 R = reflect(I, norm);
  
     vec3 base = texture(texture_diffuse1, TexCoord).rgb;;
     float moy = (base.r + base.g + base.b) / 3.0;
     float test1 = abs(base.r - moy); 
     float test2 = abs(base.g - moy);
     float test3 = abs(base.b - moy);
     
     if(test1 < 0.015 && test2 < 0.015 && test3 < 0.015 && moy > 0.05){
   

      vec3 color_reflect = texture(reflection_cubeMap, R).rgb;
      //vec3 color_reflect = sum.rgb;

      float factor = (color_reflect.r + color_reflect.g + color_reflect.b) / 3.0;

      result = mix(result, color_reflect, 0.45 * (factor));
     
     }

    }

    // final out color
    fragColor = vec4(result, final_alpha);
    
  }else{
    if(face_cube != -1.0){
     float far = 1000.0;
     float lightDistance = length(FragPos - viewPos);
     lightDistance /= far;
     gl_FragDepth = lightDistance; //near * far / ((gl_FragCoord.z * (far - near)) - far);          
    }
  }
  
}
