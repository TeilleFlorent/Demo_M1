#version 330 core
in vec3 TexCoords;
in vec3 FragPos;

out vec4 color;

uniform samplerCube skybox;
uniform float alpha;
uniform float is_foggy;

void main()
{    
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


    // FOG
    float res;
    vec3 factor;
    if(is_foggy == 1.0){
        float min = -0.001;
        float max = 0.7;

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
    

    color = vec4(texture(skybox, TexCoords).rgb * factor , res);

}
  