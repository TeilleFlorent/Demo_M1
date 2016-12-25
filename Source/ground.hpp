#include <string>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/gtx/string_cast.hpp"
#include "glm/ext.hpp"

#include <algorithm>
using namespace std;

#define GLEW_STATIC
#include <GL/glew.h>

#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>


class MyVbo {
	public:
		uint uiBuffer;
		int iSize;
		int iCurrentSize;
		int iBufferType;
		vector<unsigned char> data;
		bool bDataUploaded;
};
	



class GroundFromHeightMap
{

	public:

		GroundFromHeightMap();

		float rand_FloatRange(float a, float b)
		{
			return ((b-a)*((float)rand()/RAND_MAX))+a;
		}	

		float MyBarryCentric(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec2 pos);

		void RenderHeightmap();

		Uint32 Getpixel(SDL_Surface *surface, int x, int y);

		bool LoadHeightMapFromImage(string sImagePath);

		float GetHeightFromRealVector(glm::vec3 vRealPosition);



		GLuint uiVAO;

		bool bLoaded;
		int iRows;
		int iCols;
		float fTimePassed;

		glm::vec3 vRenderScale;

		MyVbo Data_vbo;
		MyVbo Indices_vbo;

		vector< vector< glm::vec3> > VertexData;
};