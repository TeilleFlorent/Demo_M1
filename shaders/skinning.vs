#version 330

const int MAX_BONES = 100;
uniform mat4 gBones[MAX_BONES];

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 lightSpaceMatrix;
uniform float var;

layout (location = 0) in vec3 vsiPosition;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in ivec4 BoneIDs[4];
layout (location = 8) in vec4 Weights[4];



 out vec3 vsoNormal;
 out vec2 TexCoord;
 out vec3 FragPos;
 out vec3 Tangent;
 out vec4 FragPosLightSpace;



 void main(void) {

 	mat4 modelViewMatrix2, viewMatrix2;


 	ivec4 BoneIDvec1 = ivec4(BoneIDs[0][0], BoneIDs[0][1], BoneIDs[0][2], BoneIDs[0][3]);
    ivec4 BoneIDvec2 = ivec4(BoneIDs[1][0], BoneIDs[1][1], BoneIDs[1][2], BoneIDs[1][3]);
    ivec4 BoneIDvec3 = ivec4(BoneIDs[2][0], BoneIDs[2][1], BoneIDs[2][2], BoneIDs[2][3]);
  
    mat4 BoneTransform = gBones[BoneIDvec1.x] * Weights[0].x;
    BoneTransform     += gBones[BoneIDvec1.y] * Weights[0].y;
    BoneTransform     += gBones[BoneIDvec1.z] * Weights[0].z;
    BoneTransform     += gBones[BoneIDvec1.w] * Weights[0].w;
    
    BoneTransform     += gBones[BoneIDvec2.x] * Weights[1].x;
    //BoneTransform     += gBones[BoneIDvec2.y] * Weights[1].y;
   /* BoneTransform     += gBones[BoneIDvec2.z] * Weights[1].z;
    BoneTransform     += gBones[BoneIDvec2.w] * Weights[1].w;
    
    BoneTransform     += gBones[BoneIDvec3.x] * Weights[2].x;
    BoneTransform     += gBones[BoneIDvec3.y] * Weights[2].y;
    BoneTransform     += gBones[BoneIDvec3.z] * Weights[2].z;
    BoneTransform     += gBones[BoneIDvec3.w] * Weights[2].w;*/

    vec4 temp_position = BoneTransform * vec4(vsiPosition,1.0f);

 	modelViewMatrix2 = modelViewMatrix;

 	viewMatrix2 = viewMatrix;

 	vec4 out_position = projectionMatrix * viewMatrix2 * modelViewMatrix2 * temp_position /*vec4(vsiPosition, 1.0)*/;
 
 	gl_Position = out_position;

 	FragPos=vec3(modelViewMatrix2 * vec4(vsiPosition,1.0f));

    vec3 temp_normal = vec3(BoneTransform * vec4(normal, 0.0));
 	vsoNormal = mat3(transpose(inverse(modelViewMatrix2))) * /*normal*/ temp_normal;  

	TexCoord = texCoord; //vec2(texCoord.x, 1.0 - texCoord.y);

    vec3 temp_tangent = vec3(BoneTransform * vec4(tangent, 0.0));
	Tangent = vec3(modelViewMatrix2 * vec4(/*tangent*/ temp_tangent, 0.0));

    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
   
}

