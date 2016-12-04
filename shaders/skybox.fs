#version 330 core

#define G_SCATTERING 0.3
#define PI 3.14159265358979323846264338
#define NB_STEPS 10

in vec3 TexCoords;
in vec3 FragPos;

out vec4 fragColor;

uniform samplerCube skybox;
uniform float alpha;
uniform float is_foggy;

uniform float is_volum_light;
uniform vec3 viewPos;
uniform mat4 lightSpaceMatrix;
uniform mat4 model;
uniform sampler2D shadow_map1;
uniform vec3 LightPos[3];


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
  ray_vector *= 0.03;
  float ray_length = length(ray_vector);


  vec3 ray_direction = ray_vector / ray_length;
  float step_length = ray_length / NB_STEPS;

  vec3 step = ray_direction * step_length;

  vec3 current_ray_position = start_ray_position;

  float final_nb_steps = NB_STEPS;

  for(int i = 0; i < NB_STEPS; i++){

    vec4 frag_pos_light_space = lightSpaceMatrix * vec4(current_ray_position, 1.0);

    vec3 projCoords = frag_pos_light_space.xyz / frag_pos_light_space.w;
    projCoords = projCoords * 0.5 + 0.5;

    float shadowMapValue = texture(shadow_map1, projCoords.xy).r;
    
    vec3 light_direction = normalize(LightPos[2] - current_ray_position);
    
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

  acc /= final_nb_steps;

  return acc;
}



void main(){    
	
    vec3 result,color;
    float final_alpha;

    color = texture(skybox, TexCoords).rgb;

    //color = vec3(0.0,0.0,0.0);

    /*vec4 sum = vec4(0.0);
	vec3 tc = TexCoords;
	float resolution = 1024;
	float radius = 8.0;
	vec3 dir = vec3(1.0,1.0,1.0);

	float blur = radius / resolution;

	float hstep = dir.x;
	float vstep = dir.y;
	float zstep = dir.z;


    sum += texture(skybox, vec3(tc.x - 4.0*blur*hstep, tc.y - 4.0*blur*vstep, tc.z - 4.0*blur*zstep)) * 0.0162162162;
    sum += texture(skybox, vec3(tc.x - 3.0*blur*hstep, tc.y - 3.0*blur*vstep, tc.z - 3.0*blur*zstep)) * 0.0540540541;
    sum += texture(skybox, vec3(tc.x - 2.0*blur*hstep, tc.y - 2.0*blur*vstep, tc.z - 2.0*blur*zstep)) * 0.1216216216;
    sum += texture(skybox, vec3(tc.x - 1.0*blur*hstep, tc.y - 1.0*blur*vstep, tc.z - 1.0*blur*zstep)) * 0.1945945946;

    sum += texture(skybox, vec3(tc.x, tc.y, tc.z)) * 0.2270270270;

    sum += texture(skybox, vec3(tc.x + 1.0*blur*hstep, tc.y + 1.0*blur*vstep, tc.z - 1.0*blur*zstep)) * 0.1945945946;
    sum += texture(skybox, vec3(tc.x + 2.0*blur*hstep, tc.y + 2.0*blur*vstep, tc.z - 2.0*blur*zstep)) * 0.1216216216;
    sum += texture(skybox, vec3(tc.x + 3.0*blur*hstep, tc.y + 3.0*blur*vstep, tc.z - 3.0*blur*zstep)) * 0.0540540541;
    sum += texture(skybox, vec3(tc.x + 4.0*blur*hstep, tc.y + 4.0*blur*vstep, tc.z - 4.0*blur*zstep)) * 0.0162162162;*/

    //color = vec4(sum.rgb * 1.3f ,alpha);


    // ADD FOG
    float res;
    vec3 factor;
    if(is_foggy == 1.0){
        float min = -0.001 * 50;
        float max = 0.7 * 50;

        float full = max - min;
        float temp = 0.0;
        

        if(FragPos.y > min){

            if(FragPos.y <= 0.0){
                temp += (abs(min) - abs(FragPos.y));

            }else{
                temp += FragPos.y;
                temp += abs(min);
            }

            res = temp / full;
        }

        factor = vec3(1.0);
    }

    if(is_foggy == 0.0){
        res = 1.0;
        factor = vec3(1.3);
    }

    color *= factor;

    result = color;

    // ADD VOLUMETRIC LIGHT
    if(is_volum_light == 1.0){
       result += VolumetricLightCalculation() * 5.0;
    }



    
    final_alpha = res;
    final_alpha = 1.0;

    fragColor = vec4(result , final_alpha);

}
  