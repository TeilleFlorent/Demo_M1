#include "skinning.hpp"

    

    Texture_to_skinning::Texture_to_skinning(GLenum TextureTarget, const std::string& FileName)
    {
        m_textureTarget = TextureTarget;
        m_fileName      = FileName;
    }


    bool Texture_to_skinning::Load()
    {

        t = NULL;
        t = IMG_Load(m_fileName.c_str());


/*            printf("try image load = %s\n",m_fileName.c_str());

            if(!t)
                printf("image null\n");*/


            glGenTextures(1, &m_textureObj);
            glBindTexture(m_textureTarget, m_textureObj);


            if(t->format->format == SDL_PIXELFORMAT_RGB332
                || t->format->format == SDL_PIXELFORMAT_RGB444
                || t->format->format == SDL_PIXELFORMAT_RGB555
                || t->format->format == SDL_PIXELFORMAT_RGB565
                || t->format->format == SDL_PIXELFORMAT_RGB24
                || t->format->format == SDL_PIXELFORMAT_RGB888
              //|| t->format->format == SDL_PIXELFORMAT_RGBX8888
                || t->format->format == SDL_PIXELFORMAT_RGB565
                || t->format->format == SDL_PIXELFORMAT_BGR555
                || t->format->format == SDL_PIXELFORMAT_BGR565
                || t->format->format == SDL_PIXELFORMAT_BGR24
                || t->format->format == SDL_PIXELFORMAT_BGR888){
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels);
           }else{

            if(t->format->format == SDL_PIXELFORMAT_RGBA4444
                || t->format->format == SDL_PIXELFORMAT_RGBA5551
                || t->format->format == SDL_PIXELFORMAT_ARGB4444
                || t->format->format == SDL_PIXELFORMAT_ABGR4444
                || t->format->format == SDL_PIXELFORMAT_BGRA4444
                || t->format->format == SDL_PIXELFORMAT_ABGR1555
                || t->format->format == SDL_PIXELFORMAT_BGRA5551
                || t->format->format == SDL_PIXELFORMAT_ARGB8888
                || t->format->format == SDL_PIXELFORMAT_ABGR8888
                || t->format->format == SDL_PIXELFORMAT_BGRA8888
              //|| t->format->format == SDL_PIXELFORMAT_RGBX8888
                || t->format->format == SDL_PIXELFORMAT_RGBA8888){

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->pixels);
               }else{ glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels); }
           }


        //glTexImage2D(m_textureTarget, 0, GL_RGBA, m_image.columns(), m_image.rows(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_blob.data());  
        //glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        //glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    

           glGenerateMipmap(m_textureTarget);    
           glTexParameteri( m_textureTarget, GL_TEXTURE_WRAP_S, GL_REPEAT );
           glTexParameteri( m_textureTarget, GL_TEXTURE_WRAP_T, GL_REPEAT );
           glTexParameteri( m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
           glTexParameteri( m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
           glBindTexture(m_textureTarget, 0);

           return true;
   }
    
    
   void Texture_to_skinning::Bind(GLenum TextureUnit)
   {
      glActiveTexture(TextureUnit);
      glBindTexture(GL_TEXTURE_2D, m_textureObj);
   }


/////////////////////////////////////////////////////////////////////
   

	// conversion matrice assimp -> glm
    glm::mat4 SkinnedMesh_animation::aiMatrix4x4ToGlm(const aiMatrix4x4* from)
    {
        glm::mat4 to;

        to[0][0] = (GLfloat)from->a1; to[0][1] = (GLfloat)from->b1;  to[0][2] = (GLfloat)from->c1; to[0][3] = (GLfloat)from->d1;
        to[1][0] = (GLfloat)from->a2; to[1][1] = (GLfloat)from->b2;  to[1][2] = (GLfloat)from->c2; to[1][3] = (GLfloat)from->d2;
        to[2][0] = (GLfloat)from->a3; to[2][1] = (GLfloat)from->b3;  to[2][2] = (GLfloat)from->c3; to[2][3] = (GLfloat)from->d3;
        to[3][0] = (GLfloat)from->a4; to[3][1] = (GLfloat)from->b4;  to[3][2] = (GLfloat)from->c4; to[3][3] = (GLfloat)from->d4;

        return to;
    }


	// fonction qui renvoi le nombre de bones max assigné par vertice d'un mesh animé
    int SkinnedMesh_animation::max_bones(const aiMesh *mesh)
    {
        int *counts = (int*)calloc(mesh->mNumVertices, sizeof(int));
        for (uint i = 0; i < mesh->mNumBones; i++) {
            const aiBone *bone = mesh->mBones[i];
            for (uint j = 0; j < bone->mNumWeights; j++) {
                const aiVertexWeight *weight = &bone->mWeights[j];
                counts[weight->mVertexId]++;
            }
        }
        int max = 0;
        for (uint i = 0; i < mesh->mNumVertices; i++) {
            if (max < counts[i])
                max = counts[i];
        }
        return max;
    }


    bool SkinnedMesh_animation::LoadAnimation(const string& Filename)
    {
    
        bool Ret = false;    

        m_pScene = m_Importer.ReadFile(Filename.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);


     /*   aiString bla;   
        m_Importer.GetExtensionList (bla);
        cout << bla.C_Str() << endl;
*/

        return Ret;
    }
    
    
    /////////////////////////////////////////////////////////////////////////////////////
    



    SkinnedMesh::SkinnedMesh()
    {
        m_VAO = 0;

        ZERO_MEM(m_Buffers);
     
        m_NumBones = 0;
       
        m_pScene = NULL;
       
    }
    


    bool SkinnedMesh::LoadMesh(const string& Filename)
    {
        Clear();

        glGenVertexArrays(1, &m_VAO);   
        glBindVertexArray(m_VAO);

        glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);

        bool Ret = false;    

        //std::cout << "TEST" << std::endl;
        m_pScene = m_Importer.ReadFile(Filename.c_str(), aiProcess_Triangulate | /*aiProcess_GenSmoothNormals |*/ aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

        //std::cout << "TEST2" << std::endl;

     /*   aiString bla;   
        m_Importer.GetExtensionList (bla);
        cout << bla.C_Str() << endl;*/


        if (m_pScene){  


            if(m_pScene->mAnimations[0])
                //printf("scene chargée\n");



            //m_GlobalInverseTransform = m_pScene->mRootNode->mTransformation;
                m_GlobalInverseTransform = aiMatrix4x4ToGlm(&m_pScene->mRootNode->mTransformation);

            //std::cout<<glm::to_string(m_GlobalInverseTransform)<<std::endl;
                m_GlobalInverseTransform = glm::inverse(m_GlobalInverseTransform);

            //std::cout<<glm::to_string(m_GlobalInverseTransform)<<std::endl;
                Ret = InitFromScene(m_pScene, Filename);

        }
        else {
            printf("Error parsing '%s': '%s'\n", Filename.c_str(), m_Importer.GetErrorString());
        }

    // Make sure the VAO is not changed from the outside
        glBindVertexArray(0);   

        return Ret;
    }



    void SkinnedMesh::Render()
    {
        glBindVertexArray(m_VAO);

        for (uint i = 0 ; i < m_Entries.size() ; i++) {
        //    const uint MaterialIndex = m_Entries[i].MaterialIndex;

          //  assert(MaterialIndex < m_Textures.size());

            //if (m_Textures[MaterialIndex]) {
                m_Textures[0]->Bind(GL_TEXTURE0);
                m_Textures[1]->Bind(GL_TEXTURE1);
                m_Textures[2]->Bind(GL_TEXTURE2);
            
            //}

                //glActiveTexture(GL_TEXTURE0);
                //glBindTexture(GL_TEXTURE_2D, m_Textures[0]);
                //glActiveTexture(GL_TEXTURE1);
                //glBindTexture(GL_TEXTURE_2D, tex_depthMap);

                if(i == 1 || i == 0)
                    glDrawElementsBaseVertex(GL_TRIANGLES, m_Entries[i].NumIndices, GL_UNSIGNED_INT, (void*)(sizeof(uint) * m_Entries[i].BaseIndex), m_Entries[i].BaseVertex);
        }

    // Make sure the VAO is not changed from the outside    
        glBindVertexArray(0);
    }


    

    void SkinnedMesh::Print_info_model(){


        std::cout << "\nSKINNED MODEL:\nnb mesh(s) = " << m_Entries.size() << std::endl;
        
        for(uint i = 0; i < m_Entries.size(); i++){
            std::cout << "mesh " << i << " = " << m_Entries[i].NumIndices << " vertice(s)" << std::endl;
        }

        std::cout << "nb bone(s) = " << m_NumBones << std::endl;
        
        std::cout << "nb tex(s) = " << m_Textures.size() << std::endl;
        
        std::cout << "num max bones = " << num_bones_per_vertex << std::endl;        

        printf("\n");

    }


    glm::mat4 SkinnedMesh::aiMatrix4x4ToGlm(const aiMatrix4x4* from)
    {
        glm::mat4 to;

        to[0][0] = (GLfloat)from->a1; to[0][1] = (GLfloat)from->b1;  to[0][2] = (GLfloat)from->c1; to[0][3] = (GLfloat)from->d1;
        to[1][0] = (GLfloat)from->a2; to[1][1] = (GLfloat)from->b2;  to[1][2] = (GLfloat)from->c2; to[1][3] = (GLfloat)from->d2;
        to[2][0] = (GLfloat)from->a3; to[2][1] = (GLfloat)from->b3;  to[2][2] = (GLfloat)from->c3; to[2][3] = (GLfloat)from->d3;
        to[3][0] = (GLfloat)from->a4; to[3][1] = (GLfloat)from->b4;  to[3][2] = (GLfloat)from->c4; to[3][3] = (GLfloat)from->d4;

        return to;
    }

    // fonction qui renvoi le nombre de bones max assigné par vertice d'un mesh animé
    int SkinnedMesh::max_bones(const aiMesh *mesh)
    {
        int *counts = (int*)calloc(mesh->mNumVertices, sizeof(int));
        for (uint i = 0; i < mesh->mNumBones; i++) {
            const aiBone *bone = mesh->mBones[i];
            for (uint j = 0; j < bone->mNumWeights; j++) {
                const aiVertexWeight *weight = &bone->mWeights[j];
                counts[weight->mVertexId]++;
            }
        }
        int max = 0;
        for (uint i = 0; i < mesh->mNumVertices; i++) {
            if (max < counts[i])
                max = counts[i];
        }
        return max;
    }

    

    void SkinnedMesh::BoneTransform(float TimeInSeconds, vector<glm::mat4>& Transforms, SkinnedMesh_animation Model)
    {
        glm::mat4 Identity;
        Identity = glm::mat4(1.0);
        //Identity.InitIdentity();

        float TicksPerSecond = (float)(/*m_pScene*/Model.m_pScene->mAnimations[0]->mTicksPerSecond != 0 ? /*m_pScene*/Model.m_pScene->mAnimations[0]->mTicksPerSecond : 25.0f);
        float TimeInTicks = TimeInSeconds * TicksPerSecond;
        float AnimationTime = fmod(TimeInTicks, (float)/*m_pScene*/Model.m_pScene->mAnimations[0]->mDuration);

        ReadNodeHeirarchy(AnimationTime, Model.m_pScene->mRootNode, Identity, Model);
        

        Transforms.resize(m_NumBones /*Model_num_bones*/);


        for (uint i = 0 ; i < m_NumBones/*Model_num_bones*/ ; i++) {
            Transforms[i] = m_BoneInfo[i].FinalTransformation /*glm::mat4(1.0)*/;
        //print_mat4(Model.m_BoneInfo[i].FinalTransformation);

        }
    }



    void SkinnedMesh::CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
    {
        if (pNodeAnim->mNumScalingKeys == 1) {
            Out = pNodeAnim->mScalingKeys[0].mValue;
            return;
        }

        uint ScalingIndex = FindScaling(AnimationTime, pNodeAnim);
        uint NextScalingIndex = (ScalingIndex + 1);
        assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
        float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
        float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
        assert(Factor >= 0.0f && Factor <= 1.0f);
        const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
        const aiVector3D& End   = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
        aiVector3D Delta = End - Start;
        Out = Start + Factor * Delta;
    }

    

    void SkinnedMesh::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
    {

        if (pNodeAnim->mNumRotationKeys == 1) {
            Out = pNodeAnim->mRotationKeys[0].mValue;
            return;
        }
    
        uint RotationIndex = FindRotation(AnimationTime, pNodeAnim);
        uint NextRotationIndex = (RotationIndex + 1);
        assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
        float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
        float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
        assert(Factor >= 0.0f && Factor <= 1.0f);
        const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
        const aiQuaternion& EndRotationQ   = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;    
        aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
        Out = Out.Normalize();
    }
    

    
    void SkinnedMesh::CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
    {
        if (pNodeAnim->mNumPositionKeys == 1) {
            Out = pNodeAnim->mPositionKeys[0].mValue;
            return;
        }

        uint PositionIndex = FindPosition(AnimationTime, pNodeAnim);
        uint NextPositionIndex = (PositionIndex + 1);
        assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
        float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
        float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
        assert(Factor >= 0.0f && Factor <= 1.0f);
        const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
        const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
        aiVector3D Delta = End - Start;
        Out = Start + Factor * Delta;
    }


    uint SkinnedMesh::FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
    {
        assert(pNodeAnim->mNumScalingKeys > 0);

        for (uint i = 0 ; i < pNodeAnim->mNumScalingKeys - 1 ; i++) {
            if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
                return i;
            }
        }

        assert(0);

        return 0;
    }


    uint SkinnedMesh::FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
    {
        assert(pNodeAnim->mNumRotationKeys > 0);

        for (uint i = 0 ; i < pNodeAnim->mNumRotationKeys - 1 ; i++) {
            if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
                return i;
            }
        }

        assert(0);

        return 0;
    }

    uint SkinnedMesh::FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
    {    
        for (uint i = 0 ; i < pNodeAnim->mNumPositionKeys - 1 ; i++) {
            if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
                return i;
            }
        }

        assert(0);

        return 0;
    }

    

    const aiNodeAnim* SkinnedMesh::FindNodeAnim(const aiAnimation* pAnimation, const string NodeName)
    {
        for (uint i = 0 ; i < pAnimation->mNumChannels ; i++) {
            const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

            if (string(pNodeAnim->mNodeName.data) == NodeName) {
            return pNodeAnim;
        }
    }
    
    return NULL;
    }


    
    void SkinnedMesh::ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform, SkinnedMesh_animation Model)
    {    
        string NodeName(pNode->mName.data);

        const aiAnimation* pAnimation = /*m_pScene*/Model.m_pScene->mAnimations[0];
        
        glm::mat4 NodeTransformation = aiMatrix4x4ToGlm(&pNode->mTransformation);


        const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnimation, NodeName);

        if (pNodeAnim) {
        // Interpolate scaling and generate scaling transformation matrix
            aiVector3D Scaling;
            CalcInterpolatedScaling(Scaling, AnimationTime, pNodeAnim);
            glm::mat4 ScalingM = glm::mat4(1.0);
            ScalingM = glm::scale(ScalingM, glm::vec3(Scaling.x, Scaling.y, Scaling.z));
            //ScalingM.InitScaleTransform(Scaling.x, Scaling.y, Scaling.z);

        // Interpolate rotation and generate rotation transformation matrix
            aiQuaternion RotationQ;
            CalcInterpolatedRotation(RotationQ, AnimationTime, pNodeAnim);        
            aiMatrix4x4 temp = aiMatrix4x4(RotationQ.GetMatrix());
            glm::mat4 RotationM = aiMatrix4x4ToGlm(&temp);

        // Interpolate translation and generate translation transformation matrix
            aiVector3D Translation;
            CalcInterpolatedPosition(Translation, AnimationTime, pNodeAnim);
            glm::mat4 TranslationM = glm::mat4(1.0);
            TranslationM = glm::translate(TranslationM, glm::vec3(Translation.x, Translation.y, Translation.z));
            //TranslationM.InitTranslationTransform(Translation.x, Translation.y, Translation.z);

        // Combine the above transformations
            NodeTransformation = TranslationM * RotationM * ScalingM;
        }

        glm::mat4 GlobalTransformation = ParentTransform * NodeTransformation;

        if (m_BoneMapping.find(NodeName) != m_BoneMapping.end()) {
            uint BoneIndex = m_BoneMapping[NodeName];
            m_BoneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation * m_BoneInfo[BoneIndex].BoneOffset;
        }

        for (uint i = 0 ; i < pNode->mNumChildren ; i++) {
            ReadNodeHeirarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation, Model);
        }
    }

    
    
bool SkinnedMesh::InitFromScene(const aiScene* pScene, const string& Filename)
{  
    m_Entries.resize(pScene->mNumMeshes);
    m_Textures.resize((pScene->mNumMaterials)+1);

    num_bones_per_vertex = 0;

    //std::cout << "\nNumMaterials = " << pScene->mNumMaterials << std::endl;        


    vector<glm::vec3> Positions;
    vector<glm::vec3> Normals;
    vector<glm::vec2> TexCoords;
    vector<glm::vec3> Tangents;
    vector<VertexBoneData> Bones;
    vector<uint> Indices;
       
    uint NumVertices = 0;
    uint NumIndices = 0;
    
    // Count the number of vertices and indices
    for (uint i = 0 ; i < m_Entries.size() ; i++) {
        m_Entries[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;        
        m_Entries[i].NumIndices    = pScene->mMeshes[i]->mNumFaces * 3;
        m_Entries[i].BaseVertex    = NumVertices;
        m_Entries[i].BaseIndex     = NumIndices;
        
        NumVertices += pScene->mMeshes[i]->mNumVertices;
        NumIndices  += m_Entries[i].NumIndices;
    }
    
    // Reserve space in the vectors for the vertex attributes and indices
    Positions.reserve(NumVertices);
    Normals.reserve(NumVertices);
    TexCoords.reserve(NumVertices);
    Tangents.reserve(NumVertices);
    Bones.resize(NumVertices);
    Indices.reserve(NumIndices);
        
    // Initialize the meshes in the scene one by one
    for (uint i = 0 ; i < m_Entries.size() ; i++) {

        //std::cout << "\nmax bones = " << max_bones(pScene->mMeshes[i]) << std::endl;        

        if(max_bones(pScene->mMeshes[i]) > num_bones_per_vertex){
            num_bones_per_vertex = max_bones(pScene->mMeshes[i]);
        }
        
        const aiMesh* paiMesh = pScene->mMeshes[i];
        InitMesh(i, paiMesh, Positions, Normals, TexCoords, Tangents, Bones, Indices);

    }


    if (!InitMaterials(pScene, Filename)) {
        return false;
    }
 


    // Generate and populate the buffers with vertex attributes and the indices
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[POS_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Positions[0]) * Positions.size(), &Positions[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(POSITION_LOCATION);
    glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);    

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[TEXCOORD_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoords[0]) * TexCoords.size(), &TexCoords[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(TEX_COORD_LOCATION);
    glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[NORMAL_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Normals[0]) * Normals.size(), &Normals[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(NORMAL_LOCATION);
    glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[TANGENT_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Tangents[0]) * Tangents.size(), &Tangents[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(TANGENT_LOCATION);
    glVertexAttribPointer(TANGENT_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);


    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[BONE_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Bones[0]) * Bones.size(), &Bones[0], GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(BONE_ID_LOCATION);
    glVertexAttribIPointer(BONE_ID_LOCATION, 4, GL_INT, sizeof(VertexBoneData), (const GLvoid*)0);
    glEnableVertexAttribArray(BONE_ID_LOCATION+1);
    //glVertexAttribIPointer(BONE_ID_LOCATION+1, 2, GL_INT, sizeof(VertexBoneData), (const void *)(4*(sizeof(int))));
    glVertexAttribIPointer(BONE_ID_LOCATION+1, 1, GL_INT, sizeof(VertexBoneData), (const void *)(4*(sizeof(int))));
   

    glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);    
    //glVertexAttribPointer(BONE_WEIGHT_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const void *)(6*(sizeof(int))));
    glVertexAttribPointer(BONE_WEIGHT_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const void *)(5*(sizeof(int))));
    
    glEnableVertexAttribArray(BONE_WEIGHT_LOCATION+1);    
    //glVertexAttribPointer(BONE_WEIGHT_LOCATION+1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const void *)((6*(sizeof(int)))+(4*(sizeof(float)))));
    glVertexAttribPointer(BONE_WEIGHT_LOCATION+1, 1, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const void *)((5*(sizeof(int)))+(4*(sizeof(float)))));
    


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices[0]) * Indices.size(), &Indices[0], GL_STATIC_DRAW);

    return GLCheckError();
}


    
void SkinnedMesh::InitMesh(uint MeshIndex, const aiMesh* paiMesh,
                    vector<glm::vec3>& Positions,
                    vector<glm::vec3>& Normals,
                    vector<glm::vec2>& TexCoords,
                    vector<glm::vec3>& Tangents,
                    vector<VertexBoneData>& Bones,
                    vector<uint>& Indices) {
    
    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
    
    // Populate the vertex attribute vectors
    for (uint i = 0 ; i < paiMesh->mNumVertices ; i++) {
        const aiVector3D* pPos      = &(paiMesh->mVertices[i]);
        const aiVector3D* pNormal   = &(paiMesh->mNormals[i]);
        const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;
        const aiVector3D* pTangent = &(paiMesh->mTangents[i]);

        Positions.push_back(glm::vec3(pPos->x, pPos->y, pPos->z));
        Normals.push_back(glm::vec3(pNormal->x, pNormal->y, pNormal->z));
        TexCoords.push_back(glm::vec2(pTexCoord->x, pTexCoord->y));
        Tangents.push_back(glm::vec3(pTangent->x, pTangent->y, pTangent->z));        
    }
    
    LoadBones(MeshIndex, paiMesh, Bones);
    
    // Populate the index buffer
    for (uint i = 0 ; i < paiMesh->mNumFaces ; i++) {
        const aiFace& Face = paiMesh->mFaces[i];
        assert(Face.mNumIndices == 3);
        Indices.push_back(Face.mIndices[0]);
        Indices.push_back(Face.mIndices[1]);
        Indices.push_back(Face.mIndices[2]);
    }
}


    
void SkinnedMesh::LoadBones(uint MeshIndex, const aiMesh* pMesh, vector<VertexBoneData>& Bones)
{
    for (uint i = 0 ; i < pMesh->mNumBones ; i++) {                
        uint BoneIndex = 0;        
        string BoneName(pMesh->mBones[i]->mName.data);
        
        if (m_BoneMapping.find(BoneName) == m_BoneMapping.end()) {
            // Allocate an index for a new bone
            BoneIndex = m_NumBones;
            m_NumBones++;            
            BoneInfo bi;            
            m_BoneInfo.push_back(bi);
            m_BoneInfo[BoneIndex].BoneOffset = aiMatrix4x4ToGlm(&pMesh->mBones[i]->mOffsetMatrix);            
            m_BoneMapping[BoneName] = BoneIndex;
        }
        else {
            BoneIndex = m_BoneMapping[BoneName];
        }                      
        
        for (uint j = 0 ; j < pMesh->mBones[i]->mNumWeights ; j++) {
            uint VertexID = m_Entries[MeshIndex].BaseVertex + pMesh->mBones[i]->mWeights[j].mVertexId;
            float Weight  = pMesh->mBones[i]->mWeights[j].mWeight;                   
            Bones[VertexID].AddBoneData(BoneIndex, Weight);
        }
    }    
}



bool SkinnedMesh::InitMaterials(const aiScene* pScene, const string& Filename)
{
    // Extract the directory part from the file name
    string::size_type SlashIndex = Filename.find_last_of("/");
    string Dir;

    if (SlashIndex == string::npos) {
        Dir = ".";
    }
    else if (SlashIndex == 0) {
        Dir = "/";
    }
    else {
        Dir = Filename.substr(0, SlashIndex);
    }

    bool Ret = true;


    // init des tex
    for (uint i = 0 ; i < pScene->mNumMaterials; i++) {
        const aiMaterial* pMaterial = pScene->mMaterials[i];

        //std::cout << "TEST = " << pScene->mMaterials[0]. << std::endl;



        m_Textures[i] = NULL;

        if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            aiString Path;


            if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                string p(Path.data);

                
                if (p.substr(0, 2) == ".\\") {                    
                    p = p.substr(2, p.size() - 2);
                }

                string FullPath = Dir + "/" + p;


                m_Textures[i] = new Texture_to_skinning(GL_TEXTURE_2D, FullPath.c_str());


                if (!m_Textures[i]->Load()) {

                    printf("Error loading texture '%s'\n", FullPath.c_str());
                    delete m_Textures[i];
                    m_Textures[i] = NULL;
                    Ret = false;
                }
                else {
                    //printf("texture %d load : '%s'\n", i, FullPath.c_str());
                }
            }
        }
    }



    string FullPath = Dir + "/" + "../../../../../mixamo/data/skins/paladin_prop_j_nordstrom/paladin_prop_j_nordstrom.fbm/Paladin_specular.png";
    m_Textures[1] = new Texture_to_skinning(GL_TEXTURE_2D, FullPath.c_str());
    if (!m_Textures[1]->Load()) {
        printf("Error loading texture '%s'\n", FullPath.c_str());
        delete m_Textures[1];
        m_Textures[1] = NULL;
        Ret = false;
    }
    else {
        //printf("texture %d load : '%s'\n", 1, FullPath.c_str());
    }

    FullPath = Dir + "/" + "../../../../../mixamo/data/skins/paladin_prop_j_nordstrom/paladin_prop_j_nordstrom.fbm/Paladin_normal.png";
    m_Textures[2] = new Texture_to_skinning(GL_TEXTURE_2D, FullPath.c_str());
    if (!m_Textures[2]->Load()) {
        printf("Error loading texture '%s'\n", FullPath.c_str());
        delete m_Textures[2];
        m_Textures[2] = NULL;
        Ret = false;
    }
    else {
        //printf("texture %d load : '%s'\n", 1, FullPath.c_str());
    }


    return Ret;
}



void SkinnedMesh::Clear()
{
    for (uint i = 0 ; i < m_Textures.size() ; i++) {
        SAFE_DELETE(m_Textures[i]);
    }

    if (m_Buffers[0] != 0) {
        glDeleteBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);
    }
       
    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
}



