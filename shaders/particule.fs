#version 330 core

in vec2 TexCoords;
in vec4 position_for_tex;	

out vec4 color;


uniform sampler2D MyTex;
uniform sampler2D depth_map_feu;
uniform sampler2D particles_pre_render;
uniform float test;
uniform float distance_camera;
uniform vec4 ParticleColor;
uniform float blend_factor;


void main()
{
    if(test != 2.0){

       float final_alpha;
       vec3 result;	

       vec2 TexCoords2 = (position_for_tex.xy / position_for_tex.w);
       TexCoords2 = TexCoords2 * 0.5 + 0.5;

       vec4 temp = (texture(MyTex, TexCoords) * ParticleColor);
       result = temp.rgb;
       final_alpha = temp.a;

       // smoke transform
       if(result.r == 0.0){
         result = vec3(0.4,0.4,0.4);
       }

       //////// COLOR TRAITEMENT
       vec4 blend_color = vec4(0.0,0.0,0.0,0.0);

       if(test == 1){
         blend_color = texture(particles_pre_render, TexCoords2);
       }
       result.r = result.r*0.45*0.8 + blend_color.r*0.45*0.8;
       result.g = result.g*0.5*0.8 + blend_color.g*0.5*0.8;
       result.b = result.b*0.5*0.8 + blend_color.b*0.5*0.8;

       //final_alpha = final_alpha + blend_color.a*0.0;

   
       //// BLEND DEPTH TRAITEMENT
       if(test == 1){
        
          float depth_feu = texture(depth_map_feu, TexCoords2).r;
        
          if(depth_feu != 1.0){

            // re linearisation des profondeur des fragment
            float depth = 0.1 * 1000.0 / ((gl_FragCoord.z * (1000.0 - 0.1)) - 1000.0);
            depth_feu = 0.1 * 1000.0 / ((depth_feu * (1000.0 - 0.1)) - 1000.0);
            float dist = depth_feu - depth;
            dist = abs(dist);


            float blend_factor2 = blend_factor;


            if(dist < blend_factor2){

                dist = dist / blend_factor2;

                final_alpha *= dist;

            }
          }
      }

      color = vec4(result, final_alpha);
   }

 }  