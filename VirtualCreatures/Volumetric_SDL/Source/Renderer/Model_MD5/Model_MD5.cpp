#include <Renderer/Model_MD5/Model_MD5.h>

#include <Utilities/UtilFuncs.h>

#include <fstream>
#include <sstream>

#include <assert.h>

Model_MD5::Model_MD5()
	: m_loaded(false), m_pAnimation(NULL), m_textures(Asset_Texture::Asset_Factory), m_vertexSkinningMode(e_GPU),
	m_pScene(NULL), m_pBatchRenderer(NULL)
{
}

bool Model_MD5::LoadAsset(const std::string &name)
{
	assert(!m_loaded);
	
	std::ifstream fromFile(name);

	if(!fromFile.is_open())
	{
		std::cerr << "Could not find file " << name << std::endl;
		return false;
	}

	std::string paramName;

	// Skip comments until header
	do
		fromFile >> paramName;
	while(paramName != "MD5Version" && !fromFile.eof());

	// Load header
	fromFile >> m_version;
	assert(m_version == 10);
	fromFile >> paramName;
	assert(paramName == "commandline");
	getline(fromFile, paramName); // Skip
	fromFile >> paramName;
	assert(paramName == "numJoints");
	fromFile >> m_numJoints;
	fromFile >> paramName;
	assert(paramName == "numMeshes");
	fromFile >> m_numMeshes;

	bool usingMaterialLib = false;

	std::unordered_map<std::string, unsigned int> materialNamesToIndices;

	while(!fromFile.eof())
	{
		fromFile >> paramName;
	
		if(paramName == "joints")
		{
			// Skip to next line
			fromFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

			// Load joints
			for(int i = 0; i < m_numJoints; i++)
			{
				std::string line;
				getline(fromFile, line);
				std::stringstream ss(line);

				Joint j;
				ss >> j.m_name;

				RemoveOuterCharacters(j.m_name);

				ss >> j.m_parentID;
				ss >> paramName; // Skip (
				ss >> j.m_pos.x >> j.m_pos.y >> j.m_pos.z;
				ss >> paramName >> paramName; // Skip ) (
				ss >> j.m_orient.x >> j.m_orient.y >> j.m_orient.z;

				j.m_orient.CalculateWFromXYZ();

				m_joints.push_back(j);

				// Add ID to map
				m_jointMap[j.m_name] = i;
			}

			// Skip }
			fromFile >> paramName;
			assert(paramName == "}");

			// Build bind pose now that have skeleton
			BuildBindPose(m_joints);
		}
		else if(paramName == "mesh")
		{
			Mesh m;

			// Skip to next line
			fromFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

			// 4 sections, 4 headers
			for(int h = 0; h < 4; h++)
			{
				fromFile >> paramName;

				if(paramName == "shader")
				{
					if(usingMaterialLib)
					{
						// Load shader name
						std::string materialName;
						fromFile >> materialName;

						RemoveOuterCharacters(materialName);

						m.m_materialIndex = materialNamesToIndices[materialName];
					}
					else
					{
						// Load shader name
						std::string textureName;
						fromFile >> textureName;

						RemoveOuterCharacters(textureName);

						// Get root name
						unsigned int i;

						for(i = name.size() - 1; i > 0; i--)
						{
							if(name[i] == '/' || name[i] == '\\')
								break;
						}

						// Add tex name on to normal name (relative to model file name)
						std::stringstream fullTexName;
						fullTexName << name.substr(0, i + 1) << textureName;

						Asset* pAsset;

						if(!m_textures.GetAsset(fullTexName.str(), pAsset))
						{
							fromFile.close();

							return false;
						}

						Material defaultMaterial;

						defaultMaterial.m_pDiffuseMap = static_cast<Asset_Texture*>(pAsset);

						defaultMaterial.m_pDiffuseMap->GenMipMaps();

						m_materials.push_back(defaultMaterial);

						m.m_materialIndex = m_materials.size() - 1;
					}
				}
				else if(paramName == "numverts")
				{
					// Load vertices
					int numVertices;

					fromFile >> numVertices;

					// Skip to next line
					fromFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

					for(int i = 0; i < numVertices; i++)
					{
						std::string line;
						getline(fromFile, line);
						std::stringstream ss(line);

						ss >> paramName;
						assert(paramName == "vert");
						ss >> paramName; // Skip vertex index (already know what it is - i)

						Vertex v;

						ss >> paramName; // Skip (
						ss >> v.m_texCoord.x >> v.m_texCoord.y;
						ss >> paramName; // Skip )
						ss >> v.m_startWeight >> v.m_numWeights;

						m.m_vertices.push_back(v);

						m.m_texCoords.push_back(v.m_texCoord);
					}
				}
				else if(paramName == "numtris")
				{
					// Load triangles
					int numTriangles;

					fromFile >> numTriangles;

					// Skip to next line
					fromFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

					for(int i = 0; i < numTriangles; i++)
					{
						std::string line;
						getline(fromFile, line);
						std::stringstream ss(line);

						ss >> paramName;
						assert(paramName == "tri");
						ss >> paramName; // Skip triangle index (already know what it is - i)

						Triangle t;

						// 3 indices for a triangle
						t.resize(3);

						// Reverse order since culling is backwards
						ss >> t[2] >> t[1] >> t[0];

						m.m_triangles.push_back(t);

						m.m_indices.push_back(t[0]);
						m.m_indices.push_back(t[1]);
						m.m_indices.push_back(t[2]);
					}
				}
				else if(paramName == "numweights")
				{
					// Load weights
					int numWeights;

					fromFile >> numWeights;

					// Skip to next line
					fromFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

					for(int i = 0; i < numWeights; i++)
					{
						std::string line;
						getline(fromFile, line);
						std::stringstream ss(line);

						ss >> paramName;
						assert(paramName == "weight");
						ss >> paramName; // Skip weight index (already know what it is - i)

						Weight w;

						ss >> w.m_jointID >> w.m_bias;
						ss >> paramName; // Skip (
						ss >> w.m_pos.x >> w.m_pos.y >> w.m_pos.z;

						m.m_weights.push_back(w);
					}
				}
				else
				{
					std::cerr << "Could not identify header " << paramName << " in " << name << std::endl;

					fromFile.close();

					return false;
				}
			}

			PrepareMeshVertices_FirstTime(m);
			PrepareMeshNormals(m);

			CreateMeshBuffers(m);

			m_meshes.push_back(m);

			// Skip }
			fromFile >> paramName;
			assert(paramName == "}");
		}
		else if(paramName == "mtllib")
		{
			usingMaterialLib = true;

			// File name
			std::string materialName;
			fromFile >> materialName;

			// Remove quotes
			RemoveOuterCharacters(materialName);

			if(!Material::LoadFromMTL(materialName, &m_textures, materialNamesToIndices, m_materials))
				return false;
		}
		else
			break;
	}

	fromFile.close();

	assert(m_joints.size() == m_numJoints);
	assert(m_meshes.size() == m_numMeshes);

	FindShaderCombination();

	m_animatedBones.assign(m_numJoints, Matrix4x4f::IdentityMatrix()); // For GPU skinning, set to right size

	m_loaded = true;

	return true;
}

void Model_MD5::SetRenderer(Scene* pScene)
{
	m_pScene = pScene;

	m_pBatchRenderer = static_cast<Model_MD5_BatchRenderer*>(pScene->GetBatchRenderer(MODEL_MD5_BATCHRENDERER_NAME, Model_MD5_BatchRenderer::Model_MD5_BatchRendererFactory));
}

void Model_MD5::PrepareMeshVertices_FirstTime(Mesh &m)
{
	m.m_positions.clear();
	m.m_texCoords.clear();
	m.m_boneIndices.clear();
    m.m_boneWeights.clear();

	// Negative extremes for AABB expansion
	m_noAnimationAABB.m_lowerBound = Vec3f(9999.99f, 9999.99f, 9999.99f);
	m_noAnimationAABB.m_upperBound = Vec3f(-9999.99f, -9999.99f, -9999.99f);

	bool tooManyWeightsForGPUSkinning = false;

	for(unsigned int i = 0, size = m.m_vertices.size(); i < size; i++)
	{
		// Init to 0
		Vertex &vertex = m.m_vertices[i];

		vertex.m_pos.x = vertex.m_pos.y = vertex.m_pos.z = 0.0f;
		
		// Init normal to 0 as well for later
		vertex.m_normal.x = vertex.m_normal.y = vertex.m_normal.z = 0.0f;

		BoneWeightSet weightSet;
		BoneIndexSet indexSet;

		if(vertex.m_numWeights > 4)
			tooManyWeightsForGPUSkinning = true;
		
		// Sum the weights
		for(int j = 0; j < vertex.m_numWeights; j++)
		{
			Weight &weight = m.m_weights[vertex.m_startWeight + j];
			Joint &joint = m_joints[weight.m_jointID];

			// Joint local to object (transform weight position by joint transform)
			vertex.m_pos += (joint.m_pos + joint.m_orient * weight.m_pos) * weight.m_bias;

			weightSet.m_weights[j] = weight.m_bias;
			indexSet.m_indices[j] = static_cast<float>(weight.m_jointID);
		}

		// Expand AABB
		if(vertex.m_pos.x < m_noAnimationAABB.m_lowerBound.x)
			m_noAnimationAABB.m_lowerBound.x = vertex.m_pos.x;
		if(vertex.m_pos.y < m_noAnimationAABB.m_lowerBound.y)
			m_noAnimationAABB.m_lowerBound.y = vertex.m_pos.y;
		if(vertex.m_pos.z < m_noAnimationAABB.m_lowerBound.z)
			m_noAnimationAABB.m_lowerBound.z = vertex.m_pos.z;

		if(vertex.m_pos.x > m_noAnimationAABB.m_upperBound.x)
			m_noAnimationAABB.m_upperBound.x = vertex.m_pos.x;
		if(vertex.m_pos.y > m_noAnimationAABB.m_upperBound.y)
			m_noAnimationAABB.m_upperBound.y = vertex.m_pos.y;
		if(vertex.m_pos.z > m_noAnimationAABB.m_upperBound.z)
			m_noAnimationAABB.m_upperBound.z = vertex.m_pos.z;

		m.m_positions.push_back(vertex.m_pos);
		m.m_texCoords.push_back(vertex.m_texCoord); // Adding texture coordinate now so don't have to loop through later

		m.m_boneWeights.push_back(weightSet);
		m.m_boneIndices.push_back(indexSet);	
	}

	if(tooManyWeightsForGPUSkinning)
		std::cout << "Warning: Mesh contains more than 4 weights for one vertex! Cannot use GPU skinning!" << std::endl;
}

void Model_MD5::PrepareMeshVertices(Mesh &m)
{
	m.m_positions.clear();

	// Negative extremes for AABB expansion
	m_noAnimationAABB.m_lowerBound = Vec3f(9999.99f, 9999.99f, 9999.99f);
	m_noAnimationAABB.m_upperBound = Vec3f(-9999.99f, -9999.99f, -9999.99f);

	for(unsigned int i = 0, size = m.m_vertices.size(); i < size; i++)
	{
		// Init to 0
		Vertex &vertex = m.m_vertices[i];

		vertex.m_pos.x = vertex.m_pos.y = vertex.m_pos.z = 0.0f;
		
		// Init normal to 0 as well for later
		vertex.m_normal.x = vertex.m_normal.y = vertex.m_normal.z = 0.0f;

		// Sum the weights
		for(int j = 0; j < vertex.m_numWeights; j++)
		{
			Weight &weight = m.m_weights[vertex.m_startWeight + j];
			Joint &joint = m_joints[weight.m_jointID];

			// Joint local to object (transform weight position by joint transform)
			vertex.m_pos += (joint.m_pos + joint.m_orient * weight.m_pos) * weight.m_bias;
		}

		// Expand AABB
		if(vertex.m_pos.x < m_noAnimationAABB.m_lowerBound.x)
			m_noAnimationAABB.m_lowerBound.x = vertex.m_pos.x;
		if(vertex.m_pos.y < m_noAnimationAABB.m_lowerBound.y)
			m_noAnimationAABB.m_lowerBound.y = vertex.m_pos.y;
		if(vertex.m_pos.z < m_noAnimationAABB.m_lowerBound.z)
			m_noAnimationAABB.m_lowerBound.z = vertex.m_pos.z;

		if(vertex.m_pos.x > m_noAnimationAABB.m_upperBound.x)
			m_noAnimationAABB.m_upperBound.x = vertex.m_pos.x;
		if(vertex.m_pos.y > m_noAnimationAABB.m_upperBound.y)
			m_noAnimationAABB.m_upperBound.y = vertex.m_pos.y;
		if(vertex.m_pos.z > m_noAnimationAABB.m_upperBound.z)
			m_noAnimationAABB.m_upperBound.z = vertex.m_pos.z;

		m.m_positions.push_back(vertex.m_pos);
	}
}

void Model_MD5::PrepareMeshNormals(Mesh &m)
{
	m.m_normals.clear();

	// Find normal of each triangle, and add on to vertex normals (averaging them, but will not be normalized)
	// Vertex normals are all 0 before this so can average properly
	for(unsigned i = 0, size = m.m_triangles.size(); i < size; i++)
	{
		// Vertices of the triangle
		Vec3f v0(m.m_vertices[m.m_triangles[i][0]].m_pos);
		Vec3f v1(m.m_vertices[m.m_triangles[i][1]].m_pos);
		Vec3f v2(m.m_vertices[m.m_triangles[i][2]].m_pos);

		// Cross product of 2 vectors tangent to triangle to find normal. Order matters (which way normal is facing)
		Vec3f normal((v1 - v0).Cross(v2 - v0));

		// Add on normal to vertices (for averaging)
		for(int j = 0; j < 3; j++)
			m.m_vertices[m.m_triangles[i][j]].m_normal += normal;
	}

	// Normalize the normals (will be way to long from averaging)
	for(unsigned int i = 0, size = m.m_vertices.size(); i < size; i++)
	{
		Vertex &vertex = m.m_vertices[i];

		Vec3f normal = vertex.m_normal.Normalize();

		// Add to buffer for rendering
		m.m_normals.push_back(normal);

		// Reset vertex normal
		vertex.m_normal.x = vertex.m_normal.y = vertex.m_normal.z = 0.0f;

		// Transform bind position normal for animation later
		for(int j = 0; j < vertex.m_numWeights; j++)
		{
			Weight &weight = m.m_weights[vertex.m_startWeight + j];
			Joint &joint = m_joints[weight.m_jointID];

			vertex.m_normal += (joint.m_orient * normal) * weight.m_bias;
		}
	}
}

void Model_MD5::PrepareMeshVertices(Mesh &m, const FrameSkeleton &skeleton)
{
	for(unsigned int i = 0, size = m.m_vertices.size(); i < size; i++)
	{
		// Init to 0
		const Vertex &vertex = m.m_vertices[i];
		Vec3f &pos = m.m_positions[i];
		Vec3f &normal = m.m_normals[i];

		// Sum weights
		pos.x = pos.y = pos.z = 0.0f;
		normal.x = normal.y = normal.z = 0.0f;

		for(int j = 0; j < vertex.m_numWeights; j++)
		{
			const Weight &weight = m.m_weights[vertex.m_startWeight + j];
			const SkeletalJoint &joint = skeleton[weight.m_jointID];

			pos += (joint.m_pos + joint.m_orient * weight.m_pos) * weight.m_bias;
			normal += (joint.m_orient.Inverse() * vertex.m_normal) * weight.m_bias; // For some reason, need to invert it to get it to look right
		}
	}
}

void Model_MD5::CreateMeshBuffers(Mesh &m)
{
	m.m_positionBuffer.Create();
    m.m_normalBuffer.Create();
    m.m_boneWeightBuffer.Create();
    m.m_boneIndexBuffer.Create();
    m.m_texCoordBuffer.Create();
    m.m_indexBuffer.Create();

	m.m_positionBuffer.Bind(GL_ARRAY_BUFFER);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec3f) * m.m_positions.size(), &m.m_positions[0], GL_STATIC_DRAW);

	m.m_normalBuffer.Bind(GL_ARRAY_BUFFER);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec3f) * m.m_normals.size(), &m.m_normals[0], GL_STATIC_DRAW);

	m.m_texCoordBuffer.Bind(GL_ARRAY_BUFFER);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2f) * m.m_texCoords.size(), &m.m_texCoords[0], GL_STATIC_DRAW);

	m.m_boneWeightBuffer.Bind(GL_ARRAY_BUFFER);
	glBufferData(GL_ARRAY_BUFFER, sizeof(BoneWeightSet) * m.m_boneWeights.size(), &m.m_boneWeights[0], GL_STATIC_DRAW);

	m.m_boneIndexBuffer.Bind(GL_ARRAY_BUFFER);
	glBufferData(GL_ARRAY_BUFFER, sizeof(BoneIndexSet) * m.m_boneIndices.size(), &m.m_boneIndices[0], GL_STATIC_DRAW);

	m.m_indexBuffer.Bind(GL_ELEMENT_ARRAY_BUFFER);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * m.m_indices.size(), &m.m_indices[0], GL_STATIC_DRAW);

	VBO::Unbind(GL_ARRAY_BUFFER);
	VBO::Unbind(GL_ELEMENT_ARRAY_BUFFER);

	GL_ERROR_CHECK();
}

void Model_MD5::SetBindPos()
{
	// Re-prep all meshes
	for(int i = 0; i < m_numMeshes; i++)
	{
		Mesh &m = m_meshes[i];

		PrepareMeshVertices(m);
		PrepareMeshNormals(m);
	}
}

void Model_MD5::RenderMesh_CPU(Mesh &m)
{
	m_materials[m.m_materialIndex].Set(m_pScene);

	glVertexPointer(3, GL_FLOAT, 0, &m.m_positions[0]);
	glNormalPointer(GL_FLOAT, 0, &m.m_normals[0]);
	glTexCoordPointer(2, GL_FLOAT, 0, &m.m_texCoords[0]);

	glDrawElements(GL_TRIANGLES, m.m_indices.size(), GL_UNSIGNED_SHORT, &m.m_indices[0]);

	GL_ERROR_CHECK();
}

void Model_MD5::RenderMesh_GPU(Mesh &m)
{
	m_materials[m.m_materialIndex].Set(m_pScene);

	m.m_positionBuffer.Bind(GL_ARRAY_BUFFER);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	m.m_normalBuffer.Bind(GL_ARRAY_BUFFER);
	glNormalPointer(GL_FLOAT, 0, NULL);

	// TEXCOORD0 (base texture coordinates)
	glActiveTexture(GL_TEXTURE0);
	m.m_texCoordBuffer.Bind(GL_ARRAY_BUFFER);
	glTexCoordPointer(2, GL_FLOAT, 0, NULL);

	// TEXCOORD1 (bone weights)
	glActiveTexture(GL_TEXTURE1);
	m.m_boneWeightBuffer.Bind(GL_ARRAY_BUFFER);
	glTexCoordPointer(4, GL_FLOAT, 0, NULL);

	// TEXCOORD2 (bone indices)
	glActiveTexture(GL_TEXTURE2);
	m.m_boneIndexBuffer.Bind(GL_ARRAY_BUFFER);
	glTexCoordPointer(4, GL_FLOAT, 0, NULL);

	m.m_indexBuffer.Bind(GL_ELEMENT_ARRAY_BUFFER);

	glDrawElements(GL_TRIANGLES, m.m_indices.size(), GL_UNSIGNED_SHORT, NULL);

	GL_ERROR_CHECK();
}

void Model_MD5::Render(float time, const Matrix4x4f &transform)
{
	assert(m_pBatchRenderer != NULL);

	m_pBatchRenderer->Add(this, time, transform);
}

void Model_MD5::Update(float time)
{
	switch(m_vertexSkinningMode)
	{
	case e_CPU:
		{
			if(m_pAnimation != NULL)
			{
				m_pAnimation->Update(time);

				const FrameSkeleton &skeleton = m_pAnimation->GetInterpolatedSkeleton();

				for(unsigned int i = 0, size = m_meshes.size(); i < size; i++)
					PrepareMeshVertices(m_meshes[i], skeleton);
			}
		}

		break;
	case e_GPU:
		{
			if(m_pAnimation == NULL) // No animation, assign identity matrix
				m_animatedBones.assign(m_numJoints, Matrix4x4f::IdentityMatrix());
			else
			{
				m_pAnimation->Update(time);

				const std::vector<Matrix4x4f> &animatedSkeleton = m_pAnimation->GetSkeletonMatrices();

				// Un-transformed versions of animated bones
				for(int i = 0; i < m_numJoints; i++)
					m_animatedBones[i] = animatedSkeleton[i] * m_inverseBindPose[i];
			}
		}

		break;
	}
}

bool Model_MD5::CheckAnimation()
{
	if(m_pAnimation == NULL)
	{
		std::cerr << "Animation check failed: No animation bound!" << std::endl;

		return false;
	}

	if(m_numJoints != m_pAnimation->GetNumJoints())
	{
		std::cerr << "Animation check failed: Joint count mismatch!" << std::endl;

		return false;
	}

	// Check that joints match up
	for(unsigned int i = 0, size = m_joints.size(); i < size; i++)
	{
		const Joint &meshJoint = m_joints[i];
		const JointDesc &animationJoint = m_pAnimation->GetJointDesc(i);

		if(meshJoint.m_name != animationJoint.m_name || meshJoint.m_parentID != animationJoint.m_parentID)
		{
			std::cerr << "Animation check failed: Joint mismatch!" << std::endl;

			return false;
		}
	}

	return true;
}

void Model_MD5::Render_Immediate()
{
	switch(m_vertexSkinningMode)
	{
	case e_CPU:
		Render_Immediate_CPU();

		break;

	case e_GPU:
		Render_Immediate_GPU();

		break;
	}
}

void Model_MD5::RenderAllMeshes_Immediate_CPU()
{
	assert(!m_meshes.empty());

	// Render additional meshes
	for(unsigned int i = 0, size = m_meshes.size(); i < size; i++)
	{
		Scene::GBufferRenderShader shader = m_materials[m_meshes[i].m_materialIndex].m_shader;

		switch(shader)
		{
		case Scene::e_plain:
			m_pScene->SetCurrentGBufferRenderShader(Scene::e_plain);
			break;
		case Scene::e_bump:
			m_pScene->SetCurrentGBufferRenderShader(Scene::e_bump);
			break;
		}

		RenderMesh_CPU(m_meshes[i]);
	}

	// Revert to normal shaders
	m_pScene->SetCurrentGBufferRenderShader(Scene::e_plain);
}

void Model_MD5::RenderAllMeshes_Immediate_GPU()
{
	assert(!m_meshes.empty());

	// Render first mesh separately for shader tracking
	Scene::GBufferRenderShader currentShader = m_materials[m_meshes[0].m_materialIndex].m_shader;

	switch(currentShader)
	{
	case Scene::e_plain:
		m_pBatchRenderer->m_skinning_gBufferRender.Bind();
		m_pBatchRenderer->SetBoneMatrices_Plain(m_animatedBones);
		break;
	case Scene::e_bump:
		m_pBatchRenderer->m_skinning_gBufferRender_bump.Bind();
		m_pBatchRenderer->SetBoneMatrices_Bump(m_animatedBones);
		break;
	}

	// Render additional meshes
	for(unsigned int i = 1, size = m_meshes.size(); i < size; i++)
	{
		Scene::GBufferRenderShader newShader = m_materials[m_meshes[i].m_materialIndex].m_shader;

		if(currentShader != newShader)
		{
			switch(currentShader)
			{
			case Scene::e_plain:
				m_pBatchRenderer->m_skinning_gBufferRender.Bind();
				m_pBatchRenderer->SetBoneMatrices_Plain(m_animatedBones);
				break;
			case Scene::e_bump:
				m_pBatchRenderer->m_skinning_gBufferRender_bump.Bind();
				m_pBatchRenderer->SetBoneMatrices_Bump(m_animatedBones);
				break;
			}

			currentShader = newShader;
		}

		RenderMesh_GPU(m_meshes[i]);
	}

	// Revert to normal shaders
	m_pScene->RebindGBufferRenderShader();
}

void Model_MD5::RenderAllMeshes_CPU()
{
	for(unsigned int i = 0, size = m_meshes.size(); i < size; i++)
		RenderMesh_CPU(m_meshes[i]);
}

void Model_MD5::RenderAllMeshes_GPU()
{
	for(unsigned int i = 0, size = m_meshes.size(); i < size; i++)
		RenderMesh_GPU(m_meshes[i]);
}

void Model_MD5::Render_Immediate_CPU()
{
	glEnable(GL_VERTEX_ARRAY);
	glEnable(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_NORMAL_ARRAY);

	RenderAllMeshes_Immediate_CPU();

	glDisable(GL_VERTEX_ARRAY);
	glDisable(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_NORMAL_ARRAY);

	VBO::Unbind(GL_ARRAY_BUFFER);
	VBO::Unbind(GL_ELEMENT_ARRAY_BUFFER);
}

void Model_MD5::Render_Immediate_GPU()
{
	assert(m_pBatchRenderer != NULL && m_pBatchRenderer->Created());
	glEnable(GL_VERTEX_ARRAY);
	glEnable(GL_NORMAL_ARRAY);

	glActiveTexture(GL_TEXTURE2);
	glEnable(GL_TEXTURE_COORD_ARRAY);
	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_COORD_ARRAY);
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_COORD_ARRAY);

	RenderAllMeshes_Immediate_GPU();

	glActiveTexture(GL_TEXTURE2);
	glDisable(GL_TEXTURE_COORD_ARRAY);
	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_COORD_ARRAY);
	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_COORD_ARRAY);

	glDisable(GL_VERTEX_ARRAY);
	glDisable(GL_NORMAL_ARRAY);

	VBO::Unbind(GL_ARRAY_BUFFER);
	VBO::Unbind(GL_ELEMENT_ARRAY_BUFFER);

	m_pScene->RebindGBufferRenderShader();
}

void Model_MD5::DebugRender_Skeleton()
{
	// Bind pos skeleton
	if(m_pAnimation == NULL)
	{
		glBegin(GL_LINES);

		for(int i = 0; i < m_numJoints; i++)
		{
			const Joint &joint = m_joints[i];

			if(joint.m_parentID != -1)
			{
				const Joint &parent = m_joints[joint.m_parentID];

				glVertex3f(parent.m_pos.x, parent.m_pos.y, parent.m_pos.z);
				glVertex3f(joint.m_pos.x, joint.m_pos.y, joint.m_pos.z);
			}
		}

		glEnd();
	}
	else // Animated skeleton
	{
		const FrameSkeleton &skeleton = m_pAnimation->GetInterpolatedSkeleton();

		glBegin(GL_LINES);

		for(int i = 0; i < m_numJoints; i++)
		{
			const SkeletalJoint &joint = skeleton[i];

			if(joint.m_parentID != -1)
			{
				const SkeletalJoint &parent = skeleton[joint.m_parentID];

				glVertex3f(parent.m_pos.x, parent.m_pos.y, parent.m_pos.z);
				glVertex3f(joint.m_pos.x, joint.m_pos.y, joint.m_pos.z);
			}
		}

		glEnd();
	}
}

void Model_MD5::BuildBindPose(const std::vector<Joint> &joints)
{
	m_bindPose.clear();
	m_inverseBindPose.clear();

	for(unsigned int i = 0, size = joints.size(); i < size; i++)
	{
		const Joint &j = joints[i];

		Matrix4x4f transform(Matrix4x4f::TranslateMatrix(j.m_pos) * j.m_orient.GetMatrix());
		Matrix4x4f inverseTransform;

		if(!transform.Inverse(inverseTransform))
			std::cerr << "Could not calculate bind position inverse matrix!" << std::endl;

		m_bindPose.push_back(transform);
		m_inverseBindPose.push_back(inverseTransform);
	}
}

void Model_MD5::FindShaderCombination()
{
	// Add to lists depending on how many shader switches it would require
	const unsigned int numObjects = m_meshes.size();

	// Check to see if all shaders are the same
	Scene::GBufferRenderShader shader = m_materials[m_meshes[0].m_materialIndex].m_shader;

	for(unsigned int i = 0; i < numObjects; i++)
	{
		// Shaders not all the same, add to multi shader list
		if(shader != m_materials[m_meshes[i].m_materialIndex].m_shader)
		{
			m_shaderCombination = e_both;

			return;
		}
	}

	// If got here, all shaders are the same
	if(shader == Scene::e_bump)
		m_shaderCombination = e_bumpOnly;
	else
		m_shaderCombination = e_plainOnly;
}

Model_MD5::ShaderCombination Model_MD5::GetShaderCombination() const
{
	return m_shaderCombination;
}

const AABB &Model_MD5::GetAABB()
{
	if(m_pAnimation != NULL)
		return m_pAnimation->GetAABB();

	return m_noAnimationAABB;
}

int Model_MD5::GetNumMeshes() const
{
	return m_numMeshes;
}

const Mesh &Model_MD5::GetMesh(int index) const
{
	assert(index >= 0 && index < m_numMeshes);

	return m_meshes[index];
}

Asset* Model_MD5::Asset_Factory()
{
	return new Model_MD5();
}

Model_MD5_BatchRenderer::Model_MD5_BatchRenderer()
	: m_created(false),
	m_usingDiffuseTexture(true), m_usingSpecularTexture(false), m_usingEmissiveTexture(false),
	m_diffuseColor(1.0f, 1.0f, 1.0f), m_specularColor(0.0f), m_emissiveColor(0.0f, 0.0f, 0.0f),
	m_pSetDiffuseColorFunc(&Model_MD5_BatchRenderer::SetDiffuseColor_DeferredRender),
	m_pSetSpecularColorFunc(&Model_MD5_BatchRenderer::SetSpecularColor_DeferredRender),
	m_pSetEmissiveColorFunc(&Model_MD5_BatchRenderer::SetEmissiveColor_DeferredRender),
	m_currentGBufferRenderShader(e_plain)
{
	// Default shader pointer
	m_pCurrentGBufferRenderShader = &m_skinning_gBufferRender;

	// Set up shader references
	m_pGBufferRenderShaders[e_plain] = &m_skinning_gBufferRender;
	m_pGBufferRenderShaders[e_bump] = &m_skinning_gBufferRender_bump;
}

bool Model_MD5_BatchRenderer::Create(const std::string &gBufferRenderShaderName, const std::string &gBufferRenderBumpShaderName)
{		
	assert(GetScene() != NULL);

	Scene* pScene = GetScene();

	std::stringstream os;

	if(!m_skinning_gBufferRender.LoadAsset(gBufferRenderShaderName))
		return false;

	if(!m_skinning_gBufferRender_bump.LoadAsset(gBufferRenderBumpShaderName))
		return false;

	// Defaults
	m_skinning_gBufferRender.Bind();

	m_skinning_gBufferRender.SetUniformv3f("gDiffuseColor", m_diffuseColor);
	m_skinning_gBufferRender.SetUniformf("gSpecularColor", m_specularColor);
	m_skinning_gBufferRender.SetUniformv3f("gEmissiveColor", m_emissiveColor);

	m_skinning_gBufferRender.SetShaderTexture("gDiffuseMap", pScene->GetWhiteTexture().GetTextureID(), GL_TEXTURE_2D); // GL_TEXTURE0
	m_skinning_gBufferRender.SetShaderTexture("gSpecularMap", pScene->GetWhiteTexture().GetTextureID(), GL_TEXTURE_2D); // GL_TEXTURE1
	m_skinning_gBufferRender.SetShaderTexture("gEmissiveMap", pScene->GetWhiteTexture().GetTextureID(), GL_TEXTURE_2D); // GL_TEXTURE2

	// Get attribute locations - plain
	for(int i = 0; i < s_numBoneMatrices; i++)
	{
		os.str("");
		os << "boneMatrices[" << i << "]";
		m_boneMatrixUniformLocations_plain[i] = m_skinning_gBufferRender.GetAttributeLocation(os.str());

		assert(m_boneMatrixUniformLocations_plain[i] != -1);
	}

	// Defaults - Bump version
	m_skinning_gBufferRender_bump.Bind();

	m_skinning_gBufferRender_bump.SetUniformv3f("gDiffuseColor", m_diffuseColor);
	m_skinning_gBufferRender_bump.SetUniformf("gSpecularColor", m_specularColor);
	m_skinning_gBufferRender_bump.SetUniformv3f("gEmissiveColor", m_emissiveColor);

	m_skinning_gBufferRender_bump.SetShaderTexture("gDiffuseMap", pScene->GetWhiteTexture().GetTextureID(), GL_TEXTURE_2D); // GL_TEXTURE0
	m_skinning_gBufferRender_bump.SetShaderTexture("gSpecularMap", pScene->GetWhiteTexture().GetTextureID(), GL_TEXTURE_2D); // GL_TEXTURE1
	m_skinning_gBufferRender_bump.SetShaderTexture("gEmissiveMap", pScene->GetWhiteTexture().GetTextureID(), GL_TEXTURE_2D); // GL_TEXTURE2
	m_skinning_gBufferRender_bump.SetShaderTexture("gNormalMap", pScene->GetWhiteTexture().GetTextureID(), GL_TEXTURE_2D); // GL_TEXTURE3

	// Get attribute locations - bump
	for(int i = 0; i < s_numBoneMatrices; i++)
	{
		os.str("");
		os << "boneMatrices[" << i << "]";
		m_boneMatrixUniformLocations_bump[i] = m_skinning_gBufferRender_bump.GetAttributeLocation(os.str());

		assert(m_boneMatrixUniformLocations_bump[i] != -1);
	}

	Shader::Unbind();

	m_created = true;

	return true;
}

void Model_MD5_BatchRenderer::Add(Model_MD5* pModel, float time, const Matrix4x4f &transform)
{
	if(pModel->m_vertexSkinningMode == Model_MD5::e_GPU)
	{
		switch(pModel->GetShaderCombination())
		{
		case Model_MD5::e_plainOnly:
			m_GPU_group.m_subObjects_plainShaderOnly[pModel].push_back(TimeAndTransform(time, transform));
			break;

		case Model_MD5::e_bumpOnly:
			m_GPU_group.m_subObjects_bumpMappingShaderOnly[pModel].push_back(TimeAndTransform(time, transform));
			break;

		case Model_MD5::e_both:
			m_subObjects_mutlipleShaders[pModel].push_back(TimeAndTransform(time, transform));
			break;
		}
	}
	else
	{
		switch(pModel->GetShaderCombination())
		{
		case Model_MD5::e_plainOnly:
			m_CPU_group.m_subObjects_plainShaderOnly[pModel].push_back(TimeAndTransform(time, transform));
			break;

		case Model_MD5::e_bumpOnly:
			m_CPU_group.m_subObjects_bumpMappingShaderOnly[pModel].push_back(TimeAndTransform(time, transform));
			break;

		case Model_MD5::e_both:
			m_subObjects_mutlipleShaders[pModel].push_back(TimeAndTransform(time, transform));
			break;
		}
	}
}

void Model_MD5_BatchRenderer::SetCurrentGBufferRenderShader(Model_MD5_BatchRenderer::GBufferRenderShader gBufferRenderShader)
{
	assert(gBufferRenderShader >= 0 && gBufferRenderShader < 2);

	if(!GetScene()->m_enableShaderSwitches)
		return;

	if(m_currentGBufferRenderShader != gBufferRenderShader)
	{
		m_currentGBufferRenderShader = gBufferRenderShader;

		m_pCurrentGBufferRenderShader = m_pGBufferRenderShaders[gBufferRenderShader];

		m_pCurrentGBufferRenderShader->Bind();
		m_pCurrentGBufferRenderShader->BindShaderTextures();

		// Set color uniforms, since they easily carry if not carefully reset after every shader change
		m_pCurrentGBufferRenderShader->SetUniformv3f("gDiffuseColor", m_diffuseColor);
		m_pCurrentGBufferRenderShader->SetUniformf("gSpecularColor", m_specularColor);
		m_pCurrentGBufferRenderShader->SetUniformv3f("gEmissiveColor", m_emissiveColor);
	}
}

void Model_MD5_BatchRenderer::Execute()
{
	assert(m_created);

	Scene* pScene = GetScene();

	// Set up shader modifier functions
	if(pScene->m_renderingDeferred)
		SetDeferredRenderFuncs();
	else
		SetNotUsingShaderFuncs();

	glEnable(GL_VERTEX_ARRAY);
	glEnable(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_NORMAL_ARRAY);

	DrawGroups();

	// Reset
	glActiveTexture(GL_TEXTURE0);

	VBO::Unbind(GL_ARRAY_BUFFER);
	VBO::Unbind(GL_ELEMENT_ARRAY_BUFFER);

	pScene->SetCurrentGBufferRenderShader_ForceBind(Scene::e_plain);

	pScene->UseDiffuseTexture(true);
	pScene->UseSpecularTexture(false);
	pScene->UseEmissiveTexture(false);

	pScene->SetDiffuseColor(Color3f(1.0f, 1.0f, 1.0f));
	pScene->SetSpecularColor(0.0f);
	pScene->SetEmissiveColor(Color3f(0.0f, 0.0f, 0.0f));

	pScene->SetWorldMatrix(Matrix4x4f::IdentityMatrix());
	
	glDisable(GL_VERTEX_ARRAY);
	glDisable(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_NORMAL_ARRAY);
}

void Model_MD5_BatchRenderer::DrawGroups()
{
	Scene* pScene = GetScene();

	// -------------------------------------- CPU Lists --------------------------------------

	if(!m_CPU_group.m_subObjects_plainShaderOnly.empty())
	{
		pScene->SetCurrentGBufferRenderShader(Scene::e_plain);

		for(std::unordered_map<Model_MD5*, std::vector<TimeAndTransform>>::iterator it = m_CPU_group.m_subObjects_plainShaderOnly.begin();
			it != m_CPU_group.m_subObjects_plainShaderOnly.end();
			it++)
		{
			Model_MD5* pModel = it->first;
			std::vector<TimeAndTransform> &timeAndTransforms = it->second;

			// Render all instances of this subobject
			for(unsigned int i = 0, size = timeAndTransforms.size(); i < size; i++)
			{
				pScene->SetWorldMatrix(timeAndTransforms[i].m_transform);

				pModel->Update(timeAndTransforms[i].m_time);

				pModel->RenderAllMeshes_CPU();
			}
		}
	}

	if(!m_CPU_group.m_subObjects_bumpMappingShaderOnly.empty())
	{
		pScene->SetCurrentGBufferRenderShader(Scene::e_bump);

		for(std::unordered_map<Model_MD5*, std::vector<TimeAndTransform>>::iterator it = m_CPU_group.m_subObjects_bumpMappingShaderOnly.begin();
			it != m_CPU_group.m_subObjects_bumpMappingShaderOnly.end();
			it++)
		{
			Model_MD5* pModel = it->first;
			std::vector<TimeAndTransform> &timeAndTransforms = it->second;

			// Render all instances of this subobject
			for(unsigned int i = 0, size = timeAndTransforms.size(); i < size; i++)
			{
				pScene->SetWorldMatrix(timeAndTransforms[i].m_transform);

				pModel->Update(timeAndTransforms[i].m_time);

				pModel->RenderAllMeshes_CPU();
			}
		}
	}

	// -------------------------------------- GPU Lists --------------------------------------

	if(pScene->m_renderingDeferred) // GPU render allowed
	{
		glActiveTexture(GL_TEXTURE2);
		glEnable(GL_TEXTURE_COORD_ARRAY);
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_COORD_ARRAY);

		if(!m_GPU_group.m_subObjects_plainShaderOnly.empty())
		{
			m_skinning_gBufferRender.Bind();
			m_skinning_gBufferRender.BindShaderTextures();

			for(std::unordered_map<Model_MD5*, std::vector<TimeAndTransform>>::iterator it = m_GPU_group.m_subObjects_plainShaderOnly.begin();
				it != m_GPU_group.m_subObjects_plainShaderOnly.end();
				it++)
			{
				Model_MD5* pModel = it->first;
				std::vector<TimeAndTransform> &timeAndTransforms = it->second;

				// Render all instances of this subobject
				for(unsigned int i = 0, size = timeAndTransforms.size(); i < size; i++)
				{
					pScene->SetWorldMatrix(timeAndTransforms[i].m_transform);

					pModel->Update(timeAndTransforms[i].m_time);

					SetBoneMatrices_Plain(pModel->m_animatedBones);

					pModel->RenderAllMeshes_GPU();
				}
			}
		}

		if(!m_GPU_group.m_subObjects_bumpMappingShaderOnly.empty())
		{
			m_skinning_gBufferRender_bump.Bind();
			m_skinning_gBufferRender_bump.BindShaderTextures();

			for(std::unordered_map<Model_MD5*, std::vector<TimeAndTransform>>::iterator it = m_GPU_group.m_subObjects_bumpMappingShaderOnly.begin();
				it != m_GPU_group.m_subObjects_bumpMappingShaderOnly.end();
				it++)
			{
				Model_MD5* pModel = it->first;
				std::vector<TimeAndTransform> &timeAndTransforms = it->second;

				// Render all instances of this subobject
				for(unsigned int i = 0, size = timeAndTransforms.size(); i < size; i++)
				{
					pScene->SetWorldMatrix(timeAndTransforms[i].m_transform);

					pModel->Update(timeAndTransforms[i].m_time);

					SetBoneMatrices_Bump(pModel->m_animatedBones);

					pModel->RenderAllMeshes_GPU();
				}
			}
		}

		glActiveTexture(GL_TEXTURE2);
		glEnable(GL_TEXTURE_COORD_ARRAY);
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_COORD_ARRAY);

		// -------------------------------------- Mixed Shader List --------------------------------------

		if(!m_subObjects_mutlipleShaders.empty())
		{
			for(std::unordered_map<Model_MD5*, std::vector<TimeAndTransform>>::iterator it = m_subObjects_mutlipleShaders.begin();
				it != m_subObjects_mutlipleShaders.end();
				it++)
			{
				Model_MD5* pModel = it->first;
				std::vector<TimeAndTransform> &timeAndTransforms = it->second;

				// Render all instances of this subobject
				for(unsigned int i = 0, size = timeAndTransforms.size(); i < size; i++)
				{
					pScene->SetWorldMatrix(timeAndTransforms[i].m_transform);

					pModel->Update(timeAndTransforms[i].m_time);

					pModel->Render_Immediate();
				}
			}
		}
	}
	else // ---------------------------------- No deferred shading allowed, all-CPU render ----------------------------------
	{
		if(!m_GPU_group.m_subObjects_plainShaderOnly.empty())
		{
			pScene->SetCurrentGBufferRenderShader(Scene::e_plain);

			for(std::unordered_map<Model_MD5*, std::vector<TimeAndTransform>>::iterator it = m_GPU_group.m_subObjects_plainShaderOnly.begin();
				it != m_GPU_group.m_subObjects_plainShaderOnly.end();
				it++)
			{
				Model_MD5* pModel = it->first;
				std::vector<TimeAndTransform> &timeAndTransforms = it->second;

				// Render all instances of this subobject
				for(unsigned int i = 0, size = timeAndTransforms.size(); i < size; i++)
				{
					pScene->SetWorldMatrix(timeAndTransforms[i].m_transform);

					pModel->Update(timeAndTransforms[i].m_time);

					pModel->RenderAllMeshes_CPU();
				}
			}
		}

		if(!m_GPU_group.m_subObjects_bumpMappingShaderOnly.empty())
		{
			pScene->SetCurrentGBufferRenderShader(Scene::e_bump);

			for(std::unordered_map<Model_MD5*, std::vector<TimeAndTransform>>::iterator it = m_GPU_group.m_subObjects_bumpMappingShaderOnly.begin();
				it != m_GPU_group.m_subObjects_bumpMappingShaderOnly.end();
				it++)
			{
				Model_MD5* pModel = it->first;
				std::vector<TimeAndTransform> &timeAndTransforms = it->second;

				// Render all instances of this subobject
				for(unsigned int i = 0, size = timeAndTransforms.size(); i < size; i++)
				{
					pScene->SetWorldMatrix(timeAndTransforms[i].m_transform);

					pModel->Update(timeAndTransforms[i].m_time);

					pModel->RenderAllMeshes_CPU();
				}
			}
		}

		// -------------------------------------- Mixed Shader List --------------------------------------

		if(!m_subObjects_mutlipleShaders.empty())
		{
			for(std::unordered_map<Model_MD5*, std::vector<TimeAndTransform>>::iterator it = m_subObjects_mutlipleShaders.begin();
				it != m_subObjects_mutlipleShaders.end();
				it++)
			{
				Model_MD5* pModel = it->first;
				std::vector<TimeAndTransform> &timeAndTransforms = it->second;

				// Render all instances of this subobject
				for(unsigned int i = 0, size = timeAndTransforms.size(); i < size; i++)
				{
					pScene->SetWorldMatrix(timeAndTransforms[i].m_transform);

					pModel->Update(timeAndTransforms[i].m_time);

					pModel->RenderAllMeshes_CPU();
				}
			}
		}
	}
}

void Model_MD5_BatchRenderer::Clear()
{
	m_CPU_group.m_subObjects_bumpMappingShaderOnly.clear();
	m_CPU_group.m_subObjects_plainShaderOnly.clear();
	m_GPU_group.m_subObjects_bumpMappingShaderOnly.clear();
	m_GPU_group.m_subObjects_plainShaderOnly.clear();
	m_subObjects_mutlipleShaders.clear();
}

void Model_MD5_BatchRenderer::SetBoneMatrices_Plain(const std::vector<Matrix4x4f> &matrices)
{
	assert(matrices.size() < s_numBoneMatrices);

	m_skinning_gBufferRender.Bind();

	for(int i = 0, size = matrices.size(); i < size; i++)
		m_skinning_gBufferRender.SetUniformmat4(m_boneMatrixUniformLocations_plain[i], matrices[i]);
}

void Model_MD5_BatchRenderer::SetBoneMatrices_Bump(const std::vector<Matrix4x4f> &matrices)
{
	assert(matrices.size() < s_numBoneMatrices);

	m_skinning_gBufferRender_bump.Bind();

	for(int i = 0, size = matrices.size(); i < size; i++)
		m_skinning_gBufferRender_bump.SetUniformmat4(m_boneMatrixUniformLocations_bump[i], matrices[i]);
}

void Model_MD5_BatchRenderer::SetMaterial_GPU(const Material &mat)
{
	// MUST SWITCH SHADERS BEFORE SET UNIFORMS, OR ELSE WILL MODIFY OLD SHADER!!!
	//m_pScene->SetCurrentGBufferRenderShader(m_shader);

	// Set material maps
	if(mat.m_pDiffuseMap != NULL)
	{
		glActiveTexture(GL_TEXTURE0);

		UseDiffuseTexture(true);

		mat.m_pDiffuseMap->Bind();
	}
	else
		UseDiffuseTexture(false);

	if(mat.m_pSpecularMap != NULL)
	{
		glActiveTexture(GL_TEXTURE1);

		UseSpecularTexture(true);

		mat.m_pSpecularMap->Bind();
	}
	else
		UseSpecularTexture(false);

	if(mat.m_pEmissiveMap != NULL)
	{
		glActiveTexture(GL_TEXTURE2);

		UseEmissiveTexture(true);

		mat.m_pEmissiveMap->Bind();
	}
	else
		UseEmissiveTexture(false);

	if(mat.m_pNormalMap != NULL)
	{
		glActiveTexture(GL_TEXTURE3);

		mat.m_pNormalMap->Bind();
	}

	// Set material colors
	SetDiffuseColor(mat.m_diffuseColor);
	SetSpecularColor(mat.m_specularColor);
	SetEmissiveColor(mat.m_emissiveColor);
}

void Model_MD5_BatchRenderer::SetDeferredRenderFuncs()
{
	m_pSetDiffuseColorFunc = &Model_MD5_BatchRenderer::SetDiffuseColor_DeferredRender;
	m_pSetSpecularColorFunc = &Model_MD5_BatchRenderer::SetSpecularColor_DeferredRender;
	m_pSetEmissiveColorFunc = &Model_MD5_BatchRenderer::SetEmissiveColor_DeferredRender;
}

void Model_MD5_BatchRenderer::SetNotUsingShaderFuncs()
{
	m_pSetDiffuseColorFunc = &Model_MD5_BatchRenderer::SetDiffuseColor_NotUsingShader;
	m_pSetSpecularColorFunc = &Model_MD5_BatchRenderer::SetSpecularColor_NotUsingShader;
	m_pSetEmissiveColorFunc = &Model_MD5_BatchRenderer::SetEmissiveColor_NotUsingShader;
}

void Model_MD5_BatchRenderer::SetDiffuseColor_DeferredRender(const Color3f &color)
{
	m_diffuseColor = color;

	m_pCurrentGBufferRenderShader->SetUniformv3f("gDiffuseColor", m_diffuseColor);
}

void Model_MD5_BatchRenderer::SetDiffuseColor_NotUsingShader(const Color3f &color)
{
	m_diffuseColor = color;
}

void Model_MD5_BatchRenderer::SetSpecularColor_DeferredRender(float color)
{
	m_specularColor = color;

	m_pCurrentGBufferRenderShader->SetUniformf("gSpecularColor", m_specularColor);
}

void Model_MD5_BatchRenderer::SetSpecularColor_NotUsingShader(float color)
{
	m_specularColor = color;
}

void Model_MD5_BatchRenderer::SetEmissiveColor_DeferredRender(const Color3f &color)
{
	m_emissiveColor = color;

	m_pCurrentGBufferRenderShader->SetUniformv3f("gEmissiveColor", m_emissiveColor);
}

void Model_MD5_BatchRenderer::SetEmissiveColor_NotUsingShader(const Color3f &color)
{
	m_emissiveColor = color;
}

void Model_MD5_BatchRenderer::SetDiffuseColor(const Color3f &color)
{
	(this->*m_pSetDiffuseColorFunc)(color);
}

void Model_MD5_BatchRenderer::SetSpecularColor(float color)
{
	(this->*m_pSetSpecularColorFunc)(color);
}

void Model_MD5_BatchRenderer::SetEmissiveColor(const Color3f &color)
{
	(this->*m_pSetEmissiveColorFunc)(color);
}

void Model_MD5_BatchRenderer::UseDiffuseTexture(bool use)
{
	if(use)
		m_usingDiffuseTexture = true;
	else
	{
		if(m_usingDiffuseTexture)
		{
			// White texture
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, GetScene()->GetWhiteTexture().GetTextureID());

			m_usingDiffuseTexture = false;
		}
	}
}

void Model_MD5_BatchRenderer::UseSpecularTexture(bool use)
{
	if(use)
		m_usingSpecularTexture = true;
	else
	{
		if(m_usingSpecularTexture)
		{
			// White texture
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, GetScene()->GetWhiteTexture().GetTextureID());
			glActiveTexture(GL_TEXTURE0);

			m_usingSpecularTexture = false;
		}
	}
}

void Model_MD5_BatchRenderer::UseEmissiveTexture(bool use)
{
	if(use)
		m_usingEmissiveTexture = true;
	else
	{
		if(m_usingEmissiveTexture)
		{
			// White texture
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, GetScene()->GetWhiteTexture().GetTextureID());
			glActiveTexture(GL_TEXTURE0);
	
			m_usingEmissiveTexture = false;
		}
	}
}

BatchRenderer* Model_MD5_BatchRenderer::Model_MD5_BatchRendererFactory()
{
	return new Model_MD5_BatchRenderer();
}
