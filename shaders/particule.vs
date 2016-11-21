
#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

out vec2 TexCoords;
out vec4 position_for_tex;


uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

//uniform vec2 offset;
uniform mat4 offset;
uniform float x_num;
uniform float y_num;
uniform float atlas_size;
uniform float sens;
uniform float test;
uniform float face_cube;


void main()
{
    float scale = 0.2f;

    // texture atlas 
    if(test != 2){
        TexCoords = vertex.zw;
        float chunk_size = (1.0/atlas_size);
        TexCoords.x = (chunk_size * x_num) + (TexCoords.x * chunk_size);
        TexCoords.y = (chunk_size * y_num) + (TexCoords.y * chunk_size);
    }

    mat4 model_view_billboarded = view * model;
    
    if(face_cube == -1){
        model_view_billboarded[0][0] = 1;
        model_view_billboarded[0][1] = 0;
        model_view_billboarded[0][2] = 0;
        
        model_view_billboarded[1][0] = 0;
        model_view_billboarded[1][1] = 1;
        model_view_billboarded[1][2] = 0;
        
        model_view_billboarded[2][0] = 0;
        model_view_billboarded[2][1] = 0;
        model_view_billboarded[2][2] = 1;
    }
   
  

    model_view_billboarded = model_view_billboarded * offset;


    float temp_vertex_x;
    if(sens == 0.0){
        temp_vertex_x = vertex.x*-1.0;
    }else{
        temp_vertex_x = vertex.x;
    }

    vec4 out_position = projection * model_view_billboarded * vec4(vec2(temp_vertex_x,vertex.y), 0.0 , 1.0); 
    position_for_tex = out_position;
    gl_Position = out_position;
}