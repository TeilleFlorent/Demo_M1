
#version 330 core
layout (location = 0) in vec3 position;

out vec3 TexCoords;
out vec3 FragPos;

uniform mat4 projection;
uniform mat4 view;


void main()
{
    gl_Position =   projection * view * vec4(position, 1.0);  
    
    TexCoords = position ;

    FragPos = vec3(mat4(1.0) * vec4(position,1.0f));

}  

