#include <Renderer/Model_OBJ.h>

#include <Constructs/Vec3f.h>
#include <Constructs/Vec2f.h>

#include <Utilities/UtilFuncs.h>

#include <Scene/Scene.h>

#include <fstream>
#include <sstream>

#include <vector>

#include <unordered_map>

#include <iostream>

#include <assert.h>

// Custom hash for custom class
size_t IndexSet::operator()(const IndexSet &set) const
{
	return static_cast<size_t>(set.vi ^ set.ti ^ set.ni);
}

bool IndexSet::operator==(const IndexSet &other) const
{
	return vi == other.vi && ti == other.ti && ni == other.ni;
}

Model_OBJ::Model_OBJ()
	: m_pScene(NULL), m_loaded(false), m_usingMTL(false), m_textureManager(Asset_Texture::Asset_Factory)
{
}

void Model_OBJ::SetRenderer(Scene* pScene)
{
	m_pScene = pScene;

	m_pBatchRenderer = static_cast<Model_OBJ_BatchRenderer*>(pScene->GetBatchRenderer("obj", Model_OBJ_BatchRenderer::Model_OBJ_BatchRendererFactory));
}

bool Model_OBJ::LoadAsset(const std::string &name)
{
	assert(!m_loaded);

	std::ifstream fromFile(name);
	
	if(!fromFile.is_open())
	{
		std::cerr << "Could not load model file " << name << std::endl;
		return false;
	}

	std::string rootName(GetRootName(name));

	std::vector<Vec3f> filePositions;
	std::vector<Vec2f> fileTexCoords;
	std::vector<Vec3f> fileNormals;

	std::vector<Vec3f> positions;
	std::vector<Vec2f> texCoords;
	std::vector<Vec3f> normals;

	// Indices for each OBJ object
	std::vector<std::vector<unsigned short>> indices;

	// Hash map for linking indices to vertex array index for attributes
	std::unordered_map<IndexSet, unsigned int, IndexSet> indexToVertex;

	// Initial extremes
	m_aabb.m_lowerBound = Vec3f(9999.0f, 9999.0f, 9999.0f);
	m_aabb.m_upperBound = Vec3f(-9999.0f, -9999.0f, -9999.0f);

	int currentObj = -1;

	std::unordered_map<std::string, unsigned int> matReferences;

	while(!fromFile.eof())
	{
		// Read line header
		std::string line;
		getline(fromFile, line);

		std::stringstream ss(line);

		std::string header;
		ss >> header;

		if(header == "v")
		{
			// Add vertex
			float x, y, z;

			ss >> x >> y >> z;

			filePositions.push_back(Vec3f(x, y, z));

			// Expand AABB
			if(x < m_aabb.m_lowerBound.x)
				m_aabb.m_lowerBound.x = x;
			if(y < m_aabb.m_lowerBound.y)
				m_aabb.m_lowerBound.y = y;
			if(z < m_aabb.m_lowerBound.z)
				m_aabb.m_lowerBound.z = z;

			if(x > m_aabb.m_upperBound.x)
				m_aabb.m_upperBound.x = x;
			if(y > m_aabb.m_upperBound.y)
				m_aabb.m_upperBound.y = y;
			if(z > m_aabb.m_upperBound.z)
				m_aabb.m_upperBound.z = z;
		}
		else if(header == "vt")
		{
			// Add texture coordinate
			float s, t;

			ss >> s >> t;

			fileTexCoords.push_back(Vec2f(s, t));
		}
		else if(header == "vn")
		{
			// Add normal
			float nx, ny, nz;

			ss >> nx >> ny >> nz;

			fileNormals.push_back(Vec3f(nx, ny, nz));
		}
		else if(header == "f")
		{
			assert(indices.size() > 0);
			assert(currentObj == indices.size() - 1);

			// Add a face
			IndexSet v[3];

			ss >> v[0].vi;
			ss.ignore(1, '/');
			ss >> v[0].ti;
			ss.ignore(1, '/');
			ss >> v[0].ni;

			ss >> v[1].vi;
			ss.ignore(1, '/');
			ss >> v[1].ti;
			ss.ignore(1, '/');
			ss >> v[1].ni;

			ss >> v[2].vi;
			ss.ignore(1, '/');
			ss >> v[2].ti;
			ss.ignore(1, '/');
			ss >> v[2].ni;

			for(int i = 0; i < 3; i++)
			{
				// Search for index set 1
				std::unordered_map<IndexSet, unsigned int>::iterator it = indexToVertex.find(v[i]);

				if(it == indexToVertex.end())
				{
					// Vertex attributes do not exist, create them

					// File indicies start at 1, so convert
					unsigned int vertIndex = v[i].vi - 1;
					unsigned int texCoordIndex = v[i].ti - 1;
					unsigned int normalIndex = v[i].ni - 1;

					positions.push_back(filePositions[vertIndex]);
					texCoords.push_back(fileTexCoords[texCoordIndex]);
					normals.push_back(fileNormals[normalIndex]);

					// Index of vertex in vertex component array
					unsigned int realIndex = positions.size() - 1;

					// Add attribute set index to the map
					indexToVertex[v[i]] = realIndex;

					indices[currentObj].push_back(static_cast<unsigned short>(realIndex));
				}
				else
				{
					// Index already exists, so add it
					indices[currentObj].push_back(static_cast<unsigned short>(it->second));
				}
			}
		}
		else if(header == "usemtl")
		{
			if(m_usingMTL)
			{
				// Get texture name and load it
				std::string matName;
				ss >> matName;

				// Link obj to material
				std::unordered_map<std::string, unsigned int>::iterator it = matReferences.find(matName);

				if(it == matReferences.end())
				{
					std::cerr << "Could not find material \"" << matName << "\"!" << std::endl;
					return false;
				}

				m_objMaterialReferences.push_back(it->second);
			}
			else
			{
				// Get texture name and load it
				std::string texName;
				ss >> texName;

				// Add tex name on to normal name (relative to model file name)
				std::stringstream fullTexName;
				fullTexName << rootName << texName;

				// New OBJ object texture
				Asset* pAsset;

				if(!m_textureManager.GetAsset(fullTexName.str(), pAsset))
					return false;

				Asset_Texture* pTexture = static_cast<Asset_Texture*>(pAsset);

				Material mat;
				mat.m_pDiffuseMap = pTexture;
				mat.m_shader = Scene::e_plain;

				m_materials.push_back(mat);
				m_objMaterialReferences.push_back(m_materials.size() - 1);

				pTexture->GenMipMaps();
			}

			// Next index in indices array
			indices.push_back(std::vector<unsigned short>());
			currentObj++;
		}
		else if(!m_usingMTL && header == "mtllib")
		{
			// Using a material library. Load the materials
			m_usingMTL = true;

			std::string libName;
			ss >> libName;

			std::ostringstream fullMaterialLibraryName;

			fullMaterialLibraryName << rootName << libName;

			if(!LoadMaterialLibrary(fullMaterialLibraryName.str(), matReferences))
			{
				std::cerr << "- in " << name << std::endl;

				return false;
			}
		}
	}

	fromFile.close();

	m_aabb.CalculateHalfDims();
	m_aabb.CalculateCenter();

	// Create the VBOs
	m_positionBuffer.Create();
	m_positionBuffer.Bind(GL_ARRAY_BUFFER);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * positions.size(), &positions[0], GL_STATIC_DRAW);
	m_positionBuffer.Unbind();

	m_texCoordBuffer.Create();
	m_texCoordBuffer.Bind(GL_ARRAY_BUFFER);
	glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(float) * texCoords.size(), &texCoords[0], GL_STATIC_DRAW);
	m_texCoordBuffer.Unbind();

	m_normalBuffer.Create();
	m_normalBuffer.Bind(GL_ARRAY_BUFFER);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * normals.size(), &normals[0], GL_STATIC_DRAW);
	m_normalBuffer.Unbind();

	const unsigned int numObjects = indices.size();

	// Create index VBOs
	m_numVertices.resize(numObjects);
	m_indexBuffers.resize(numObjects);

	for(unsigned int i = 0; i < numObjects; i++)
	{
		m_numVertices[i] = indices[i].size();

		m_indexBuffers[i].Create();
		m_indexBuffers[i].Bind(GL_ELEMENT_ARRAY_BUFFER);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * m_numVertices[i], &indices[i][0], GL_STATIC_DRAW);
		m_indexBuffers[i].Unbind();
	}

	FindShaderCombination();

	m_loaded = true;

	GL_ERROR_CHECK();

	return true;
}

bool Model_OBJ::LoadMaterialLibrary(const std::string &name, std::unordered_map<std::string, unsigned int> &matReferences)
{
	std::ifstream fromFile(name);

	if(!fromFile.is_open())
	{
		std::cerr << "Could not load material file " << name << std::endl;
		return false;
	}

	Material* pCurrent = NULL;

	while(!fromFile.eof())
	{
		// Read line header
		std::string line;
		getline(fromFile, line);

		std::stringstream ss(line);

		std::string header;
		ss >> header;

		if(header == "newmtl")
		{
			m_materials.push_back(Material());

			std::string matName;
			ss >> matName;

			matReferences[matName] = m_materials.size() - 1;

			pCurrent = &m_materials.back();
		}
		else if(header == "Ka")
		{
			if(pCurrent == NULL)
			{
				std::cerr << "Attempted to set material parameter before defining material in \"" << name << "\"!" << std::endl;
				return false;
			}
			
			ss >> pCurrent->m_emissiveColor.r >> pCurrent->m_emissiveColor.g >> pCurrent->m_emissiveColor.b;
		}
		else if(header == "Kd")
		{
			if(pCurrent == NULL)
			{
				std::cerr << "Attempted to set material parameter before defining material in \"" << name << "\"!" << std::endl;
				return false;
			}
			
			ss >> pCurrent->m_diffuseColor.r >> pCurrent->m_diffuseColor.g >> pCurrent->m_diffuseColor.b;
		}
		else if(header == "Ks")
		{
			if(pCurrent == NULL)
			{
				std::cerr << "Attempted to set material parameter before defining material in \"" << name << "\"!" << std::endl;
				return false;
			}
			
			ss >> pCurrent->m_specularColor;
		}
		else if(header == "map_Ka")
		{
			if(pCurrent == NULL)
			{
				std::cerr << "Attempted to set material parameter before defining material in \"" << name << "\"!" << std::endl;
				return false;
			}
			
			std::string mapName;
			ss >> mapName;

			Asset* pAsset;

			if(!m_textureManager.GetAsset(mapName, pAsset))
			{
				std::cerr << "- in " << name << std::endl;

				return false;
			}

			pCurrent->m_pEmissiveMap = static_cast<Asset_Texture*>(pAsset);
		}
		else if(header == "map_Kd")
		{
			if(pCurrent == NULL)
			{
				std::cerr << "Attempted to set material parameter before defining material in \"" << name << "\"!" << std::endl;
				return false;
			}
			
			std::string mapName;
			ss >> mapName;

			Asset* pAsset;

			if(!m_textureManager.GetAsset(mapName, pAsset))
			{
				std::cerr << "- in " << name << std::endl;

				return false;
			}

			pCurrent->m_pDiffuseMap = static_cast<Asset_Texture*>(pAsset);
		}
		else if(header == "map_Ks")
		{
			if(pCurrent == NULL)
			{
				std::cerr << "Attempted to set material parameter before defining material in \"" << name << "\"!" << std::endl;
				return false;
			}
			
			std::string mapName;
			ss >> mapName;

			Asset* pAsset;

			if(!m_textureManager.GetAsset(mapName, pAsset))
			{
				std::cerr << "- in " << name << std::endl;

				return false;
			}

			pCurrent->m_pSpecularMap = static_cast<Asset_Texture*>(pAsset);
		}
		else if(header == "bump" || header == "map_Bump")
		{
			if(pCurrent == NULL)
			{
				std::cerr << "Attempted to set material parameter before defining material in \"" << name << "\"!" << std::endl;
				return false;
			}
			
			std::string mapName;
			ss >> mapName;

			Asset* pAsset;

			if(!m_textureManager.GetAsset(mapName, pAsset))
			{
				std::cerr << "- in " << name << std::endl;

				return false;
			}

			pCurrent->m_pNormalMap = static_cast<Asset_Texture*>(pAsset);

			pCurrent->m_shader = Scene::e_bump;
		}
	}

	// Set last shader
	assert(!m_materials.empty());

	return true;
}

void Model_OBJ::FindShaderCombination()
{
	// Add to lists depending on how many shader switches it would require
	const unsigned int numObjects = m_indexBuffers.size();

	// Check to see if all shaders are the same
	Scene::GBufferRenderShader shader = m_materials[m_objMaterialReferences[0]].m_shader;

	for(unsigned int i = 0; i < numObjects; i++)
	{
		// Shaders not all the same, add to multi shader list
		if(shader != m_materials[m_objMaterialReferences[i]].m_shader)
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

void Model_OBJ::BatchRender_RenderSubObject(unsigned int subObjectIndex)
{
	m_indexBuffers[subObjectIndex].Bind(GL_ELEMENT_ARRAY_BUFFER);
	glDrawElements(GL_TRIANGLES, m_numVertices[subObjectIndex], GL_UNSIGNED_SHORT, NULL);
	m_indexBuffers[subObjectIndex].Unbind();
}

void Model_OBJ::BatchRender_SetSubObjectStates(unsigned int subObjectIndex)
{
	unsigned int matIndex = m_objMaterialReferences[subObjectIndex];

	// Set material maps
	if(m_materials[matIndex].m_pDiffuseMap != NULL)
	{
		glActiveTexture(GL_TEXTURE0);

		m_pScene->UseDiffuseTexture(true);

		m_materials[matIndex].m_pDiffuseMap->Bind();
	}
	else
		m_pScene->UseDiffuseTexture(false);

	if(m_materials[matIndex].m_pSpecularMap != NULL)
	{
		glActiveTexture(GL_TEXTURE1);

		m_pScene->UseSpecularTexture(true);

		m_materials[matIndex].m_pSpecularMap->Bind();
	}
	else
		m_pScene->UseSpecularTexture(false);

	if(m_materials[matIndex].m_pEmissiveMap != NULL)
	{
		glActiveTexture(GL_TEXTURE2);

		m_pScene->UseEmissiveTexture(true);

		m_materials[matIndex].m_pEmissiveMap->Bind();
	}
	else
		m_pScene->UseEmissiveTexture(false);

	if(m_materials[matIndex].m_pNormalMap != NULL)
	{
		glActiveTexture(GL_TEXTURE3);

		m_materials[matIndex].m_pNormalMap->Bind();
	}

	// Set material colors
	m_pScene->SetDiffuseColor(m_materials[matIndex].m_diffuseColor);
	m_pScene->SetSpecularColor(m_materials[matIndex].m_specularColor);
	m_pScene->SetEmissiveColor(m_materials[matIndex].m_emissiveColor);

	m_indexBuffers[subObjectIndex].Bind(GL_ELEMENT_ARRAY_BUFFER);
}

void Model_OBJ::BatchRender_SetSubObjectShader(unsigned int subObjectIndex)
{
	m_pScene->SetCurrentGBufferRenderShader(m_materials[m_objMaterialReferences[subObjectIndex]].m_shader);
}

void Model_OBJ::BatchRender_BindBuffers()
{
	assert(m_loaded);

	m_positionBuffer.Bind(GL_ARRAY_BUFFER);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	m_texCoordBuffer.Bind(GL_ARRAY_BUFFER);
	glTexCoordPointer(2, GL_FLOAT, 0, NULL);

	m_normalBuffer.Bind(GL_ARRAY_BUFFER);
	glNormalPointer(GL_FLOAT, 0, NULL);
}

void Model_OBJ::BatchRender_UnbindBuffers()
{
	m_positionBuffer.Unbind();
	m_texCoordBuffer.Unbind();
	m_normalBuffer.Unbind();
}

void Model_OBJ::BatchRender_RenderAllSubObjects_NoShaderSwitch()
{
	// Draw all OBJ objects
	for(unsigned int i = 0, numObjects = m_indexBuffers.size(); i < numObjects; i++)
	{
		unsigned int matIndex = m_objMaterialReferences[i];

		// MUST SWITCH SHADERS BEFORE SET UNIFORMS, OR ELSE WILL MODIFY OLD SHADER!!!
		m_pScene->SetCurrentGBufferRenderShader(m_materials[matIndex].m_shader);

		// Set material maps
		if(m_materials[matIndex].m_pDiffuseMap != NULL)
		{
			glActiveTexture(GL_TEXTURE0);

			m_pScene->UseDiffuseTexture(true);

			m_materials[matIndex].m_pDiffuseMap->Bind();
		}
		else
			m_pScene->UseDiffuseTexture(false);

		if(m_materials[matIndex].m_pSpecularMap != NULL)
		{
			glActiveTexture(GL_TEXTURE1);

			m_pScene->UseSpecularTexture(true);

			m_materials[matIndex].m_pSpecularMap->Bind();
		}
		else
			m_pScene->UseSpecularTexture(false);

		if(m_materials[matIndex].m_pEmissiveMap != NULL)
		{
			glActiveTexture(GL_TEXTURE2);

			m_pScene->UseEmissiveTexture(true);

			m_materials[matIndex].m_pEmissiveMap->Bind();
		}
		else
			m_pScene->UseEmissiveTexture(false);

		if(m_materials[matIndex].m_pNormalMap != NULL)
		{
			glActiveTexture(GL_TEXTURE3);

			m_materials[matIndex].m_pNormalMap->Bind();
		}

		// Set material colors
		//m_pScene->SetDiffuseColor(m_materials[matIndex].m_diffuseColor);
		m_pScene->SetSpecularColor(m_materials[matIndex].m_specularColor);
		m_pScene->SetEmissiveColor(m_materials[matIndex].m_emissiveColor);

		m_indexBuffers[i].Bind(GL_ELEMENT_ARRAY_BUFFER);

		glDrawElements(GL_TRIANGLES, m_numVertices[i], GL_UNSIGNED_SHORT, NULL);

		m_indexBuffers[i].Unbind();
	}
}

void Model_OBJ::BatchRender_RenderAllSubObjects_ShaderSwitch()
{
	// Draw all OBJ objects
	for(unsigned int i = 0, numObjects = m_indexBuffers.size(); i < numObjects; i++)
	{
		unsigned int matIndex = m_objMaterialReferences[i];

		// MUST SWITCH SHADERS BEFORE SET UNIFORMS, OR ELSE WILL MODIFY OLD SHADER!!!
		m_pScene->SetCurrentGBufferRenderShader(m_materials[matIndex].m_shader);

		// Set material maps
		if(m_materials[matIndex].m_pDiffuseMap != NULL)
		{
			glActiveTexture(GL_TEXTURE0);

			m_pScene->UseDiffuseTexture(true);

			m_materials[matIndex].m_pDiffuseMap->Bind();
		}
		else
			m_pScene->UseDiffuseTexture(false);

		if(m_materials[matIndex].m_pSpecularMap != NULL)
		{
			glActiveTexture(GL_TEXTURE1);

			m_pScene->UseSpecularTexture(true);

			m_materials[matIndex].m_pSpecularMap->Bind();
		}
		else
			m_pScene->UseSpecularTexture(false);

		if(m_materials[matIndex].m_pEmissiveMap != NULL)
		{
			glActiveTexture(GL_TEXTURE2);

			m_pScene->UseEmissiveTexture(true);

			m_materials[matIndex].m_pEmissiveMap->Bind();
		}
		else
			m_pScene->UseEmissiveTexture(false);

		if(m_materials[matIndex].m_pNormalMap != NULL)
		{
			glActiveTexture(GL_TEXTURE3);

			m_materials[matIndex].m_pNormalMap->Bind();
		}

		// Set material colors
		//m_pScene->SetDiffuseColor(m_materials[matIndex].m_diffuseColor);
		m_pScene->SetSpecularColor(m_materials[matIndex].m_specularColor);
		m_pScene->SetEmissiveColor(m_materials[matIndex].m_emissiveColor);

		m_indexBuffers[i].Bind(GL_ELEMENT_ARRAY_BUFFER);

		glDrawElements(GL_TRIANGLES, m_numVertices[i], GL_UNSIGNED_SHORT, NULL);

		m_indexBuffers[i].Unbind();
	}
}

void Model_OBJ::Render(const Matrix4x4f &transform)
{
	assert(m_loaded);
	assert(m_pScene != NULL); // Will break here is you did not call SetRenderer(...)

	m_pBatchRenderer->Add(this, transform);
}

void Model_OBJ::Render(const Matrix4x4f &transform, Scene* pScene)
{
	assert(m_loaded);
	
	if(m_pScene == NULL)
		SetRenderer(pScene);

	m_pBatchRenderer->Add(this, transform);
}

bool Model_OBJ::Loaded() const
{
	return m_loaded;
}

unsigned int Model_OBJ::GetNumObjects() const
{
	return m_indexBuffers.size();
}

unsigned int Model_OBJ::GetNumMaterials() const
{
	return m_materials.size();
}

Model_OBJ::Material* Model_OBJ::GetMaterial(unsigned int index)
{
	assert(index >= 0 && index < m_materials.size());

	return &m_materials[index];
}

AABB Model_OBJ::GetAABB() const
{
	return m_aabb;
}

const Vec3f &Model_OBJ::GetAABBOffsetFromModel() const
{
	return m_aabb.GetCenter();
}

Model_OBJ::ShaderCombination Model_OBJ::GetShaderCombination() const
{
	return m_shaderCombination;
}

Asset* Model_OBJ::Asset_Factory()
{
	return new Model_OBJ();
}

void Model_OBJ_BatchRenderer::Add(Model_OBJ* pModel, const Matrix4x4f &transform)
{
	switch(pModel->GetShaderCombination())
	{
	case Model_OBJ::e_plainOnly:
		m_subObjects_plainShaderOnly[pModel].push_back(transform);
		break;

	case Model_OBJ::e_bumpOnly:
		m_subObjects_bumpMappingShaderOnly[pModel].push_back(transform);
		break;

	case Model_OBJ::e_both:
		m_subObjects_mutlipleShaders[pModel].push_back(transform);
		break;
	}
}

void Model_OBJ_BatchRenderer::Execute()
{
	Scene* pScene = GetScene();

	glEnable(GL_VERTEX_ARRAY);
	glEnable(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_NORMAL_ARRAY);

	// Go through lists
	if(!m_subObjects_plainShaderOnly.empty())
	{
		pScene->SetCurrentGBufferRenderShader(Scene::e_plain);

		for(std::unordered_map<Model_OBJ*, std::vector<Matrix4x4f>>::iterator it = m_subObjects_plainShaderOnly.begin();
			it != m_subObjects_plainShaderOnly.end();
			it++)
		{
			Model_OBJ* pModel = it->first;
			std::vector<Matrix4x4f> &matrices = it->second;

			pModel->BatchRender_BindBuffers();

			// Render 1 subobject at a time
			for(unsigned int oi = 0, numObjects = pModel->m_indexBuffers.size(); oi < numObjects; oi++)
			{
				pModel->BatchRender_SetSubObjectStates(oi);

				// Render all instances of this subobject
				for(unsigned int i = 0, size = matrices.size(); i < size; i++)
				{
					pScene->SetWorldMatrix(matrices[i]);

					pModel->BatchRender_RenderSubObject(oi);
				}
			}

			pModel->BatchRender_UnbindBuffers();
		}
	}

	if(!m_subObjects_bumpMappingShaderOnly.empty())
	{
		pScene->SetCurrentGBufferRenderShader(Scene::e_bump);

		for(std::unordered_map<Model_OBJ*, std::vector<Matrix4x4f>>::iterator it = m_subObjects_bumpMappingShaderOnly.begin();
			it != m_subObjects_bumpMappingShaderOnly.end();
			it++)
		{
			Model_OBJ* pModel = it->first;
			std::vector<Matrix4x4f> &matrices = it->second;

			pModel->BatchRender_BindBuffers();

			// Render 1 subobject at a time
			for(unsigned int oi = 0, numObjects = pModel->m_indexBuffers.size(); oi < numObjects; oi++)
			{
				pModel->BatchRender_SetSubObjectStates(oi);

				// Render all instances of this subobject
				for(unsigned int i = 0, size = matrices.size(); i < size; i++)
				{
					pScene->SetWorldMatrix(matrices[i]);

					pModel->BatchRender_RenderSubObject(oi);
				}
			}

			pModel->BatchRender_UnbindBuffers();
		}
	}

	if(!m_subObjects_mutlipleShaders.empty())
	{
		for(std::unordered_map<Model_OBJ*, std::vector<Matrix4x4f>>::iterator it = m_subObjects_mutlipleShaders.begin();
			it != m_subObjects_mutlipleShaders.end();
			it++)
		{
			Model_OBJ* pModel = it->first;
			std::vector<Matrix4x4f> &matrices = it->second;

			pModel->BatchRender_BindBuffers();

			// Render 1 subobject at a time
			for(unsigned int oi = 0, numObjects = pModel->m_indexBuffers.size(); oi < numObjects; oi++)
			{
				pModel->BatchRender_SetSubObjectShader(oi);
				pModel->BatchRender_SetSubObjectStates(oi);

				// Render all instances of this subobject
				for(unsigned int i = 0, size = matrices.size(); i < size; i++)
				{
					pScene->SetWorldMatrix(matrices[i]);

					pModel->BatchRender_RenderSubObject(oi);
				}
			}

			pModel->BatchRender_UnbindBuffers();
		}
	}

	// Reset
	glActiveTexture(GL_TEXTURE0);

	VBO::Unbind(GL_ARRAY_BUFFER);
	VBO::Unbind(GL_ELEMENT_ARRAY_BUFFER);

	pScene->UseDiffuseTexture(true);
	pScene->UseSpecularTexture(false);
	pScene->UseEmissiveTexture(false);

	pScene->SetCurrentGBufferRenderShader(Scene::e_plain);

	pScene->SetDiffuseColor(Color3f(1.0f, 1.0f, 1.0f));
	pScene->SetSpecularColor(0.0f);
	pScene->SetEmissiveColor(Color3f(0.0f, 0.0f, 0.0f));

	pScene->SetWorldMatrix(Matrix4x4f::IdentityMatrix());
	
	glDisable(GL_VERTEX_ARRAY);
	glDisable(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_NORMAL_ARRAY);
}

void Model_OBJ_BatchRenderer::Clear()
{
	m_subObjects_bumpMappingShaderOnly.clear();
	m_subObjects_plainShaderOnly.clear();
	m_subObjects_mutlipleShaders.clear();
}

BatchRenderer* Model_OBJ_BatchRenderer::Model_OBJ_BatchRendererFactory()
{
	return new Model_OBJ_BatchRenderer();
}