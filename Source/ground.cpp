#include "ground.hpp"
			


	float GroundFromHeightMap::MyBarryCentric(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec2 pos) {

		float det = (p2.z - p3.z) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.z - p3.z);
		float l1 = ((p2.z - p3.z) * (pos.x - p3.x) + (p3.x - p2.x) * (pos.y - p3.z)) / det;
		float l2 = ((p3.z - p1.z) * (pos.x - p3.x) + (p1.x - p3.x) * (pos.y - p3.z)) / det;
		float l3 = 1.0f - l1 - l2;

		return l1 * p1.y + l2 * p2.y + l3 * p3.y;
	}


	void GroundFromHeightMap::RenderHeightmap(){
	
		glBindVertexArray(uiVAO);
		glEnable(GL_PRIMITIVE_RESTART);
		glPrimitiveRestartIndex(iRows*iCols);

		int iNumIndices = (iRows-1)*iCols*2 + iRows-1;
		glDrawElements(GL_TRIANGLE_STRIP, iNumIndices, GL_UNSIGNED_INT, 0);
		glDisable(GL_PRIMITIVE_RESTART);

	}

	

	Uint32 GroundFromHeightMap::Getpixel(SDL_Surface *surface, int x, int y){
		int bpp = surface->format->BytesPerPixel;

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
	   		return 0;      
        }
    }



    bool GroundFromHeightMap::LoadHeightMapFromImage(string sImagePath){

		if(bLoaded)
		{
			bLoaded = false;
			//ReleaseHeightmap();
		}

		
		SDL_Surface * t = NULL;
        t = IMG_Load(sImagePath.c_str());

	    iRows = t->h;
	    iCols = t->w;
	    glGenBuffers(1, &Data_vbo.uiBuffer);
        Data_vbo.data.reserve(0);
        Data_vbo.iSize = 0;
        Data_vbo.iCurrentSize = 0;
	

        vector< vector< glm::vec3> > vVertexData(iRows, vector<glm::vec3>(iCols));
        vector< vector< glm::vec2> > vCoordsData(iRows, vector<glm::vec2>(iCols));

        std::cout << "\nGround vertices = " << iRows * iCols << std::endl << std::endl;

        float fTextureU = float(iCols)*0.1f;
        float fTextureV = float(iRows)*0.1f;

        int acc = 0;
        for(int i = 0; i < iRows; i++)
        {
        	for(int j = 0; j < iCols; j++)
        	{
			
        		float fScaleC = float(j)/float(iCols-1);
        		float fScaleR = float(i)/float(iRows-1);

        		float temp = Getpixel(t,i,j);
        	    float fVertexHeight = temp / 300000000.0f;
        		vVertexData[i][j] = glm::vec3(-0.5f+fScaleC, fVertexHeight, -0.5f+fScaleR);
        		vCoordsData[i][j] = glm::vec2(fTextureU*fScaleC, fTextureV*fScaleR);
        	}
        }

	
        vector< vector<glm::vec3> > vNormals[2];

	   for(int i = 0; i < 2; i++)
        vNormals[i] = vector< vector<glm::vec3> >(iRows-1, vector<glm::vec3>(iCols-1));

	   for(int i = 0; i < iRows-1 ; i++)
        {
	 	 for(int j = 0; j < iCols-1; j++)
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


	    for(int i = 0; i < iRows; i++) 
	    	for(int j = 0; j < iCols; j++) 
	    		{    
    			glm::vec3 vFinalNormal = glm::vec3(0.0f, 0.0f, 0.0f);

        	if(j != 0 && i != 0){
        		for(int k = 0; k < 2; k++)
        			vFinalNormal += vNormals[k][i-1][j-1];
        	}
		
        	if(i != 0 && j != iCols-1)
        		vFinalNormal += vNormals[0][i-1][j];
		
        	if(i != iRows-1 && j != iCols-1){
        		for(int k = 0; k < 2; k++)
        			vFinalNormal += vNormals[k][i][j];
        	}
		
        	if(i != iRows-1 && j != 0)
        		vFinalNormal += vNormals[1][i][j-1];

        	vFinalNormal = glm::normalize(vFinalNormal);

		    vFinalNormals[i][j] = vFinalNormal; 
		}


	
		int temp_size = iRows*iCols*(2*sizeof(glm::vec3)+sizeof(glm::vec2));

		glGenBuffers(1, &Data_vbo.uiBuffer);
		Data_vbo.data.reserve(temp_size);
		Data_vbo.iSize = temp_size;
		Data_vbo.iCurrentSize = 0;


	    // SAVE
		VertexData = vVertexData;

	    for(int i = 0; i < iRows; i++) 
		{
		    for(int j = 0; j < iCols ; j++) 
			{
				Data_vbo.data.insert(Data_vbo.data.end(), (unsigned char*)&vVertexData[i][j], (unsigned char*)&vVertexData[i][j]+sizeof(glm::vec3));
				Data_vbo.iCurrentSize += sizeof(glm::vec3);

				Data_vbo.data.insert(Data_vbo.data.end(), (unsigned char*)&vCoordsData[i][j], (unsigned char*)&vCoordsData[i][j]+sizeof(glm::vec2));
				Data_vbo.iCurrentSize += sizeof(glm::vec2);

				Data_vbo.data.insert(Data_vbo.data.end(), (unsigned char*)&vFinalNormals[i][j], (unsigned char*)&vFinalNormals[i][j]+sizeof(glm::vec3));
				Data_vbo.iCurrentSize += sizeof(glm::vec3);

	        }
		}
	
		glGenBuffers(1, &Indices_vbo.uiBuffer);
		Indices_vbo.data.reserve(0);
		Indices_vbo.iSize = 0;
		Indices_vbo.iCurrentSize = 0;

		int iPrimitiveRestartIndex = iRows*iCols;
	    for(int i = 0; i < iRows-1; i ++) 
		{
		    for(int j = 0; j < iCols; j++) 
		    for(int k = 0; k < 2; k++) 
			{
				int iRow = i+(1-k);
				int iIndex = iRow*iCols+j;
			
				Indices_vbo.data.insert(Indices_vbo.data.end(), (unsigned char*)&iIndex, (unsigned char*)&iIndex+sizeof(int));
				Indices_vbo.iCurrentSize += sizeof(int);

			}
		
			Indices_vbo.data.insert(Indices_vbo.data.end(), (unsigned char*)&iPrimitiveRestartIndex, (unsigned char*)&iPrimitiveRestartIndex+sizeof(int));
			Indices_vbo.iCurrentSize += sizeof(int);

		}

	
		glGenVertexArrays(1, &uiVAO);
		glBindVertexArray(uiVAO);
		Data_vbo.iBufferType =  GL_ARRAY_BUFFER;
		glBindBuffer(Data_vbo.iBufferType, Data_vbo.uiBuffer);

		glBufferData(Data_vbo.iBufferType, Data_vbo.data.size(), &Data_vbo.data[0], GL_STATIC_DRAW);
		Data_vbo.bDataUploaded = true;

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec3)+sizeof(glm::vec2), 0);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec3)+sizeof(glm::vec2), (void*)sizeof(glm::vec3));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec3)+sizeof(glm::vec2), (void*)(sizeof(glm::vec3)+sizeof(glm::vec2)));

		Indices_vbo.iBufferType =  GL_ELEMENT_ARRAY_BUFFER;
		glBindBuffer(Indices_vbo.iBufferType, Indices_vbo.uiBuffer);

		glBufferData(Indices_vbo.iBufferType, Indices_vbo.data.size(), &Indices_vbo.data[0], GL_STATIC_DRAW);
		Indices_vbo.bDataUploaded = true;


		bLoaded = true; 
		return true;
	}






	float GroundFromHeightMap::GetHeightFromRealVector(glm::vec3 vRealPosition)
	{
		int iColumn = int((vRealPosition.x + vRenderScale.x*0.5f)*float(iCols) / (vRenderScale.x));
		int iRow = int((vRealPosition.z + vRenderScale.z*0.5f)*float(iRows) / (vRenderScale.z));

		iColumn = min(iColumn, iCols-1);
		iRow = min(iRow, iRows-1);

		iColumn = max(iColumn, 0);
		iRow = max(iRow, 0);

		return VertexData[iRow][iColumn].y*vRenderScale.y;
	}


	GroundFromHeightMap::GroundFromHeightMap()
	{
		vRenderScale = glm::vec3(100.0f, 100.0f, 100.0f);
		fTimePassed = 0.0f;
	}
