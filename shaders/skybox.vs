
#version 330 core
layout (location = 0) in vec3 position;

out vec3 TexCoords;
out vec3 FragPos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform float is_volum_light;

void main()
{

	vec4 res;

	res = projection * view * vec4(position, 1.0);

	if(is_volum_light == 1.0)
		res = projection * view * model * vec4(position, 1.0);  

	gl_Position = res;  

    TexCoords = vec3(position.x/*/position.x*/,position.y/*/position.y*/, position.z);

    FragPos = vec3(mat4(1.0) * vec4(position,1.0f));
    
    if(is_volum_light == 1.0)
    	FragPos = vec3(model * vec4(position,1.0f));
    	

}  

