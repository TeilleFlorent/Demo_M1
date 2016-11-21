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




class CMultiLayeredHeightmap
{

	public:


		float rand_FloatRange(float a, float b)
		{
			return ((b-a)*((float)rand()/RAND_MAX))+a;
		}	


		float MyBarryCentric(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec2 pos) {

			float det = (p2.z - p3.z) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.z - p3.z);
            float l1 = ((p2.z - p3.z) * (pos.x - p3.x) + (p3.x - p2.x) * (pos.y - p3.z)) / det;
            float l2 = ((p3.z - p1.z) * (pos.x - p3.x) + (p1.x - p3.x) * (pos.y - p3.z)) / det;
            float l3 = 1.0f - l1 - l2;

            return l1 * p1.y + l2 * p2.y + l3 * p3.y;
        }


        void RenderHeightmap(){
	
	/*spTerrain.SetUniform("fRenderHeight", vRenderScale.y);
	spTerrain.SetUniform("fMaxTextureU", float(iCols)*0.1f);
	spTerrain.SetUniform("fMaxTextureV", float(iRows)*0.1f);*/
	
	//spTerrain.SetUniform("HeightmapScaleMatrix", glm::scale(glm::mat4(1.0), glm::vec3(vRenderScale)));
	
	// Now we're ready to render - we are drawing set of triangle strips using one call, but we g otta enable primitive restart
	       glBindVertexArray(uiVAO);
	       glEnable(GL_PRIMITIVE_RESTART);
	       glPrimitiveRestartIndex(iRows*iCols);

	       int iNumIndices = (iRows-1)*iCols*2 + iRows-1;
	       glDrawElements(GL_TRIANGLE_STRIP, iNumIndices, GL_UNSIGNED_INT, 0);
	       glDisable(GL_PRIMITIVE_RESTART);

	   }

	

	   Uint32 Getpixel(SDL_Surface *surface, int x, int y){
	   	int bpp = surface->format->BytesPerPixel;

    /* Here p is the address to the pixel we want to retrieve */
	   	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	   	switch(bpp) {
	   		case 1:
	   		return *p;
	   		break;

	   		case 2:
	   		return *(Uint16 *)p;
	   		break;

	   		case 3:
	   		if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
	   			return p[0] << 16 | p[1] << 8 | p[2];
	   		else
	   			return p[0] | p[1] << 8 | p[2] << 16;
	   		break;

	   		case 4:
	   		return *(Uint32 *)p;
	   		break;

	   		default:
            return 0;       /* shouldn't happen, but avoids warnings */
	   	}
	   }



	bool LoadHeightMapFromImage(string sImagePath){
		
		if(bLoaded)
		{
			bLoaded = false;
			//ReleaseHeightmap();
		}

		
		SDL_Surface * t = NULL;
        t = IMG_Load(sImagePath.c_str());

        /*printf("try image load = %s\n",m_fileName.c_str());
        if(!t)
            printf("image null\n");*/

    	
	///*BYTE*/ void * bDataPointer = t->pixels/*FreeImage_GetBits(dib)*/; 
	    iRows = /*FreeImage_GetHeight(dib)*/t->h;
	    iCols = /*FreeImage_GetWidth(dib)*/t->w;


	// How much to increase data pointer to get to next pixel data
	//unsigned int ptr_inc = /*FreeImage_GetBPP(dib)*/t->format->BitsPerPixel == 24 ? 3 : 1;
	// Length of one row in data
	//unsigned int row_step = ptr_inc*iCols;

	//vboHeightmapData.CreateVBO();
    	glGenBuffers(1, &Data_vbo.uiBuffer);
        Data_vbo.data.reserve(0);
        Data_vbo.iSize = 0;
        Data_vbo.iCurrentSize = 0;
	

	// All vertex data are here (there are iRows*iCols vertices in this heightmap), we will get to normals later
        vector< vector< glm::vec3> > vVertexData(iRows, vector<glm::vec3>(iCols));
        vector< vector< glm::vec2> > vCoordsData(iRows, vector<glm::vec2>(iCols));


        float fTextureU = float(iCols)*0.1f;
        float fTextureV = float(iRows)*0.1f;

	//int acc = 0;

	   /*FOR(i, iRows)*/for(int i = 0; i < iRows; i++)
        {
		   /*FOR(j, iCols)*/for(int j = 0; j < iCols; j++)
        	{
			//std::cout << "test = " << i*j << "   " << bDataPointer << std::endl;
			//bDataPointer+= ptr_inc /* 1 */;

        		float fScaleC = float(j)/float(iCols-1);
        		float fScaleR = float(i)/float(iRows-1);

        		float temp = Getpixel(t,i,j);
			//printf("%d temp = %f\n", acc++, temp);
			    float fVertexHeight = /*float(*((bDataPointer)+row_step*i+j*ptr_inc))*/temp/ /*90000000.0f*0.5f;*/ 300000000.0f;
        		vVertexData[i][j] = glm::vec3(-0.5f+fScaleC, fVertexHeight, -0.5f+fScaleR);
        		vCoordsData[i][j] = glm::vec2(fTextureU*fScaleC, fTextureV*fScaleR);
        	}
        }

	

	// Normals are here - the heightmap contains ( (iRows-1)*(iCols-1) quads, each one containing 2 triangles, therefore array of we have 3D array)
        vector< vector<glm::vec3> > vNormals[2];

	    /*FOR(i, 2)*/for(int i = 0; i < 2; i++)
        vNormals[i] = vector< vector<glm::vec3> >(iRows-1, vector<glm::vec3>(iCols-1));

	    /*FOR(i, iRows-1)*/for(int i = 0; i < iRows-1 ; i++)
        {
	 	    /*FOR(j, iCols-1)*/for(int j = 0; j < iCols-1; j++)
        	{
        		glm::vec3 vTriangle0[] = 
        		{
        			vVertexData[i][j],
        			vVertexData[i+1][j],
        			vVertexData[i+1][j+1]
        		};
        		glm::vec3 vTriangle1[] = 
        		{
        			vVertexData[i+1][j+1],
        			vVertexData[i][j+1],
        			vVertexData[i][j]
        		};

        		glm::vec3 vTriangleNorm0 = glm::cross(vTriangle0[0]-vTriangle0[1], vTriangle0[1]-vTriangle0[2]);
        		glm::vec3 vTriangleNorm1 = glm::cross(vTriangle1[0]-vTriangle1[1], vTriangle1[1]-vTriangle1[2]);

        		vNormals[0][i][j] = glm::normalize(vTriangleNorm0);
        		vNormals[1][i][j] = glm::normalize(vTriangleNorm1);
        	}
        }


        vector< vector<glm::vec3> > vFinalNormals = vector< vector<glm::vec3> >(iRows, vector<glm::vec3>(iCols));


	    for(int i = 0; i < iRows; i++) /*FOR(i, iRows)*/
	    for(int j = 0; j < iCols; j++) /*FOR(j, iCols)*/
        {
		// Now we wanna calculate final normal for [i][j] vertex. We will have a look at all triangles this vertex is part of, and then we will make average vector
		// of all adjacent triangles' normals

        	glm::vec3 vFinalNormal = glm::vec3(0.0f, 0.0f, 0.0f);

		// Look for upper-left triangles
        	if(j != 0 && i != 0){
			/*FOR(k, 2)*/
        		for(int k = 0; k < 2; k++)
        			vFinalNormal += vNormals[k][i-1][j-1];
        	}
		
		// Look for upper-right triangles
        	if(i != 0 && j != iCols-1)
        		vFinalNormal += vNormals[0][i-1][j];
		
		// Look for bottom-right triangles
        	if(i != iRows-1 && j != iCols-1){
			/*FOR(k, 2)*/
        		for(int k = 0; k < 2; k++)
        			vFinalNormal += vNormals[k][i][j];
        	}
		
		// Look for bottom-left triangles
        	if(i != iRows-1 && j != 0)
        		vFinalNormal += vNormals[1][i][j-1];

        	vFinalNormal = glm::normalize(vFinalNormal);

		    vFinalNormals[i][j] = vFinalNormal; // Store final normal of j-th vertex in i-th row
		}


	// First, create a VBO with only vertex data
	// vboHeightmapData.CreateVBO(iRows*iCols*(2*sizeof(glm::vec3)+sizeof(glm::vec2))); // Preallocate memory
	
		int temp_size = iRows*iCols*(2*sizeof(glm::vec3)+sizeof(glm::vec2));

		glGenBuffers(1, &Data_vbo.uiBuffer);
		Data_vbo.data.reserve(temp_size);
		Data_vbo.iSize = temp_size;
		Data_vbo.iCurrentSize = 0;


	    // SAVE
		VertexData = vVertexData;

	    for(int i = 0; i < iRows; i++) /*FOR(i, iRows)*/
		{
		    for(int j = 0; j < iCols ; j++) /*FOR(j, iCols)*/
			{
				Data_vbo.data.insert(Data_vbo.data.end(), (unsigned char*)&vVertexData[i][j], (unsigned char*)&vVertexData[i][j]+sizeof(glm::vec3));
				Data_vbo.iCurrentSize += sizeof(glm::vec3);

				Data_vbo.data.insert(Data_vbo.data.end(), (unsigned char*)&vCoordsData[i][j], (unsigned char*)&vCoordsData[i][j]+sizeof(glm::vec2));
				Data_vbo.iCurrentSize += sizeof(glm::vec2);

				Data_vbo.data.insert(Data_vbo.data.end(), (unsigned char*)&vFinalNormals[i][j], (unsigned char*)&vFinalNormals[i][j]+sizeof(glm::vec3));
				Data_vbo.iCurrentSize += sizeof(glm::vec3);

	        /*
			vboHeightmapData.AddData(&vVertexData[i][j], sizeof(glm::vec3)); // Add vertex
			vboHeightmapData.AddData(&vCoordsData[i][j], sizeof(glm::vec2)); // Add tex. coord
			vboHeightmapData.AddData(&vFinalNormals[i][j], sizeof(glm::vec3)); // Add normal*/
		    }
		}
	
	// Now create a VBO with heightmap indices
	//vboHeightmapIndices.CreateVBO();
	
		glGenBuffers(1, &Indices_vbo.uiBuffer);
		Indices_vbo.data.reserve(0);
		Indices_vbo.iSize = 0;
		Indices_vbo.iCurrentSize = 0;

		int iPrimitiveRestartIndex = iRows*iCols;
	    for(int i = 0; i < iRows-1; i ++) /*FOR(i, iRows-1)*/
		{
		    for(int j = 0; j < iCols; j++) /*FOR(j, iCols)*/
		    for(int k = 0; k < 2; k++) /*FOR(k, 2)*/
			{
				int iRow = i+(1-k);
				int iIndex = iRow*iCols+j;
			//vboHeightmapIndices.AddData(&iIndex, sizeof(int));
		
				Indices_vbo.data.insert(Indices_vbo.data.end(), (unsigned char*)&iIndex, (unsigned char*)&iIndex+sizeof(int));
				Indices_vbo.iCurrentSize += sizeof(int);

			}
		// Restart triangle strips
		//vboHeightmapIndices.AddData(&iPrimitiveRestartIndex, sizeof(int));

			Indices_vbo.data.insert(Indices_vbo.data.end(), (unsigned char*)&iPrimitiveRestartIndex, (unsigned char*)&iPrimitiveRestartIndex+sizeof(int));
			Indices_vbo.iCurrentSize += sizeof(int);

		}

	
		glGenVertexArrays(1, &uiVAO);
		glBindVertexArray(uiVAO);
	// Attach vertex data to this VAO
	//vboHeightmapData.BindVBO();
		Data_vbo.iBufferType =  GL_ARRAY_BUFFER;
		glBindBuffer(Data_vbo.iBufferType, Data_vbo.uiBuffer);

	//vboHeightmapData.UploadDataToGPU(GL_STATIC_DRAW);
		glBufferData(Data_vbo.iBufferType, Data_vbo.data.size(), &Data_vbo.data[0], GL_STATIC_DRAW);
		Data_vbo.bDataUploaded = true;
	//Data_vbo.data.clear();

	// Vertex positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec3)+sizeof(glm::vec2), 0);
	// Texture coordinates
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec3)+sizeof(glm::vec2), (void*)sizeof(glm::vec3));
	// Normal vectors
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec3)+sizeof(glm::vec2), (void*)(sizeof(glm::vec3)+sizeof(glm::vec2)));

	// And now attach index data to this VAO
	// Here don't forget to bind another type of VBO - the element array buffer, or simplier indices to vertices

	//vboHeightmapIndices.BindVBO(GL_ELEMENT_ARRAY_BUFFER);
		Indices_vbo.iBufferType =  GL_ELEMENT_ARRAY_BUFFER;
		glBindBuffer(Indices_vbo.iBufferType, Indices_vbo.uiBuffer);

	//vboHeightmapIndices.UploadDataToGPU(GL_STATIC_DRAW);
		glBufferData(Indices_vbo.iBufferType, Indices_vbo.data.size(), &Indices_vbo.data[0], GL_STATIC_DRAW);
		Indices_vbo.bDataUploaded = true;
	//Indices_vbo.data.clear();


		bLoaded = true; 
		return true;
	}






	float GetHeightFromRealVector(glm::vec3 vRealPosition)
	{
		int iColumn = int((vRealPosition.x + vRenderScale.x*0.5f)*float(iCols) / (vRenderScale.x));
		int iRow = int((vRealPosition.z + vRenderScale.z*0.5f)*float(iRows) / (vRenderScale.z));

		iColumn = min(iColumn, iCols-1);
		iRow = min(iRow, iRows-1);

		iColumn = max(iColumn, 0);
		iRow = max(iRow, 0);

		return VertexData[iRow][iColumn].y*vRenderScale.y;
	}



	//static CShaderProgram* GetShaderProgram();

	CMultiLayeredHeightmap()
	{
		vRenderScale = glm::vec3(100.0f, 100.0f, 100.0f);
		fTimePassed = 0.0f;
	}

//private:

	GLuint uiVAO;

	bool bLoaded;
	//bool bShaderProgramLoaded;
	int iRows;
	int iCols;
	float fTimePassed;

	glm::vec3 vRenderScale;


	MyVbo Data_vbo;
	MyVbo Indices_vbo;

	vector< vector< glm::vec3> > VertexData;
};