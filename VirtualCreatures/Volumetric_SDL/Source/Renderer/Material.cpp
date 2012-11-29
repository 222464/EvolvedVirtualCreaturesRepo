#include <Renderer/Material.h>

#include <fstream>
#include <sstream>
#include <assert.h>

Material::Material()
	: m_diffuseColor(1.0f, 1.0f, 1.0f), m_specularColor(0.0f), m_emissiveColor(0.0f, 0.0f, 0.0f),
	m_pDiffuseMap(NULL), m_pSpecularMap(NULL), m_pNormalMap(NULL), m_pEmissiveMap(NULL),
	m_shader(Scene::e_plain)
{
}

void Material::Set(Scene* pScene)
{
	// MUST SWITCH SHADERS BEFORE SET UNIFORMS, OR ELSE WILL MODIFY OLD SHADER!!!
	pScene->SetCurrentGBufferRenderShader(m_shader);

	// Set material maps
	if(m_pDiffuseMap != NULL)
	{
		glActiveTexture(GL_TEXTURE0);

		pScene->UseDiffuseTexture(true);

		m_pDiffuseMap->Bind();
	}
	else
		pScene->UseDiffuseTexture(false);

	if(m_pSpecularMap != NULL)
	{
		glActiveTexture(GL_TEXTURE1);

		pScene->UseSpecularTexture(true);

		m_pSpecularMap->Bind();
	}
	else
		pScene->UseSpecularTexture(false);

	if(m_pEmissiveMap != NULL)
	{
		glActiveTexture(GL_TEXTURE2);

		pScene->UseEmissiveTexture(true);

		m_pEmissiveMap->Bind();
	}
	else
		pScene->UseEmissiveTexture(false);

	if(m_pNormalMap != NULL)
	{
		glActiveTexture(GL_TEXTURE3);

		m_pNormalMap->Bind();
	}

	// Set material colors
	pScene->SetDiffuseColor(m_diffuseColor);
	pScene->SetSpecularColor(m_specularColor);
	pScene->SetEmissiveColor(m_emissiveColor);
}

void Material::ResetSceneToDefault(Scene* pScene)
{
	pScene->SetCurrentGBufferRenderShader(Scene::e_plain);

	pScene->SetDiffuseColor(Color3f(1.0f, 1.0f, 1.0f));
	pScene->SetSpecularColor(0.0f);
	pScene->SetEmissiveColor(Color3f(0.0f, 0.0f, 0.0f));
}

bool Material::LoadFromMTL(const std::string &fileName, AssetManager* pTextureManager, std::vector<Material> &materials)
{
	std::ifstream fromFile(fileName);

	if(!fromFile.is_open())
	{
		std::cerr << "Could not load material file " << fileName << std::endl;
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
			materials.push_back(Material());

			pCurrent = &materials.back();
		}
		else if(header == "Ka")
		{
			if(pCurrent == NULL)
			{
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
				return false;
			}
			
			ss >> pCurrent->m_emissiveColor.r >> pCurrent->m_emissiveColor.g >> pCurrent->m_emissiveColor.b;
		}
		else if(header == "Kd")
		{
			if(pCurrent == NULL)
			{
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
				return false;
			}
			
			ss >> pCurrent->m_diffuseColor.r >> pCurrent->m_diffuseColor.g >> pCurrent->m_diffuseColor.b;
		}
		else if(header == "Ks")
		{
			if(pCurrent == NULL)
			{
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
				return false;
			}
			
			ss >> pCurrent->m_specularColor;
		}
		else if(header == "map_Ka")
		{
			if(pCurrent == NULL)
			{
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
				return false;
			}
			
			std::string mapName;
			ss >> mapName;

			Asset* pAsset;

			if(!pTextureManager->GetAsset(mapName, pAsset))
			{
				std::cerr << "- in " << fileName << std::endl;

				return false;
			}

			pCurrent->m_pEmissiveMap = static_cast<Asset_Texture*>(pAsset);
		}
		else if(header == "map_Kd")
		{
			if(pCurrent == NULL)
			{
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
				return false;
			}
			
			std::string mapName;
			ss >> mapName;

			Asset* pAsset;

			if(!pTextureManager->GetAsset(mapName, pAsset))
			{
				std::cerr << "- in " << fileName << std::endl;

				return false;
			}

			pCurrent->m_pDiffuseMap = static_cast<Asset_Texture*>(pAsset);
		}
		else if(header == "map_Ks")
		{
			if(pCurrent == NULL)
			{
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
				return false;
			}
			
			std::string mapName;
			ss >> mapName;

			Asset* pAsset;

			if(!pTextureManager->GetAsset(mapName, pAsset))
			{
				std::cerr << "- in " << fileName << std::endl;

				return false;
			}

			pCurrent->m_pSpecularMap = static_cast<Asset_Texture*>(pAsset);
		}
		else if(header == "bump" || header == "map_Bump")
		{
			if(pCurrent == NULL)
			{
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
				return false;
			}
			
			std::string mapName;
			ss >> mapName;

			Asset* pAsset;

			if(!pTextureManager->GetAsset(mapName, pAsset))
			{
				std::cerr << "- in " << fileName << std::endl;

				return false;
			}

			pCurrent->m_pNormalMap = static_cast<Asset_Texture*>(pAsset);

			pCurrent->m_shader = Scene::e_bump;
		}
	}

	// Set last shader
	assert(!materials.empty());

	return true;
}

bool Material::LoadFromMTL(const std::string &fileName, AssetManager* pTextureManager, std::vector<std::string> &materialNames, std::vector<Material> &materials)
{
	std::ifstream fromFile(fileName);

	if(!fromFile.is_open())
	{
		std::cerr << "Could not load material file " << fileName << std::endl;
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
			materials.push_back(Material());

			std::string matName;
			ss >> matName;

			materialNames.push_back(matName);

			pCurrent = &materials.back();
		}
		else if(header == "Ka")
		{
			if(pCurrent == NULL)
			{
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
				return false;
			}
			
			ss >> pCurrent->m_emissiveColor.r >> pCurrent->m_emissiveColor.g >> pCurrent->m_emissiveColor.b;
		}
		else if(header == "Kd")
		{
			if(pCurrent == NULL)
			{
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
				return false;
			}
			
			ss >> pCurrent->m_diffuseColor.r >> pCurrent->m_diffuseColor.g >> pCurrent->m_diffuseColor.b;
		}
		else if(header == "Ks")
		{
			if(pCurrent == NULL)
			{
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
				return false;
			}
			
			ss >> pCurrent->m_specularColor;
		}
		else if(header == "map_Ka")
		{
			if(pCurrent == NULL)
			{
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
				return false;
			}
			
			std::string mapName;
			ss >> mapName;

			Asset* pAsset;

			if(!pTextureManager->GetAsset(mapName, pAsset))
			{
				std::cerr << "- in " << fileName << std::endl;

				return false;
			}

			pCurrent->m_pEmissiveMap = static_cast<Asset_Texture*>(pAsset);
		}
		else if(header == "map_Kd")
		{
			if(pCurrent == NULL)
			{
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
				return false;
			}
			
			std::string mapName;
			ss >> mapName;

			Asset* pAsset;

			if(!pTextureManager->GetAsset(mapName, pAsset))
			{
				std::cerr << "- in " << fileName << std::endl;

				return false;
			}

			pCurrent->m_pDiffuseMap = static_cast<Asset_Texture*>(pAsset);
		}
		else if(header == "map_Ks")
		{
			if(pCurrent == NULL)
			{
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
				return false;
			}
			
			std::string mapName;
			ss >> mapName;

			Asset* pAsset;

			if(!pTextureManager->GetAsset(mapName, pAsset))
			{
				std::cerr << "- in " << fileName << std::endl;

				return false;
			}

			pCurrent->m_pSpecularMap = static_cast<Asset_Texture*>(pAsset);
		}
		else if(header == "bump" || header == "map_Bump")
		{
			if(pCurrent == NULL)
			{
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
				return false;
			}
			
			std::string mapName;
			ss >> mapName;

			Asset* pAsset;

			if(!pTextureManager->GetAsset(mapName, pAsset))
			{
				std::cerr << "- in " << fileName << std::endl;

				return false;
			}

			pCurrent->m_pNormalMap = static_cast<Asset_Texture*>(pAsset);

			pCurrent->m_shader = Scene::e_bump;
		}
	}

	// Set last shader
	assert(!materials.empty());

	return true;
}

bool Material::LoadFromMTL(const std::string &fileName, AssetManager* pTextureManager, std::unordered_map<std::string, unsigned int> &materialNamesToIndicesMap, std::vector<Material> &materials)
{
	std::ifstream fromFile(fileName);

	if(!fromFile.is_open())
	{
		std::cerr << "Could not load material file " << fileName << std::endl;
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
			materials.push_back(Material());

			std::string matName;
			ss >> matName;

			materialNamesToIndicesMap[matName] = materials.size() - 1;

			pCurrent = &materials.back();
		}
		else if(header == "Ka")
		{
			if(pCurrent == NULL)
			{
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
				return false;
			}
			
			ss >> pCurrent->m_emissiveColor.r >> pCurrent->m_emissiveColor.g >> pCurrent->m_emissiveColor.b;
		}
		else if(header == "Kd")
		{
			if(pCurrent == NULL)
			{
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
				return false;
			}
			
			ss >> pCurrent->m_diffuseColor.r >> pCurrent->m_diffuseColor.g >> pCurrent->m_diffuseColor.b;
		}
		else if(header == "Ks")
		{
			if(pCurrent == NULL)
			{
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
				return false;
			}
			
			ss >> pCurrent->m_specularColor;
		}
		else if(header == "map_Ka")
		{
			if(pCurrent == NULL)
			{
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
				return false;
			}
			
			std::string mapName;
			ss >> mapName;

			Asset* pAsset;

			if(!pTextureManager->GetAsset(mapName, pAsset))
			{
				std::cerr << "- in " << fileName << std::endl;

				return false;
			}

			pCurrent->m_pEmissiveMap = static_cast<Asset_Texture*>(pAsset);
		}
		else if(header == "map_Kd")
		{
			if(pCurrent == NULL)
			{
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
				return false;
			}
			
			std::string mapName;
			ss >> mapName;

			Asset* pAsset;

			if(!pTextureManager->GetAsset(mapName, pAsset))
			{
				std::cerr << "- in " << fileName << std::endl;

				return false;
			}

			pCurrent->m_pDiffuseMap = static_cast<Asset_Texture*>(pAsset);
		}
		else if(header == "map_Ks")
		{
			if(pCurrent == NULL)
			{
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
				return false;
			}
			
			std::string mapName;
			ss >> mapName;

			Asset* pAsset;

			if(!pTextureManager->GetAsset(mapName, pAsset))
			{
				std::cerr << "- in " << fileName << std::endl;

				return false;
			}

			pCurrent->m_pSpecularMap = static_cast<Asset_Texture*>(pAsset);
		}
		else if(header == "bump" || header == "map_Bump")
		{
			if(pCurrent == NULL)
			{
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
				return false;
			}
			
			std::string mapName;
			ss >> mapName;

			Asset* pAsset;

			if(!pTextureManager->GetAsset(mapName, pAsset))
			{
				std::cerr << "- in " << fileName << std::endl;

				return false;
			}

			pCurrent->m_pNormalMap = static_cast<Asset_Texture*>(pAsset);

			pCurrent->m_shader = Scene::e_bump;
		}
	}

	// Set last shader
	assert(!materials.empty());

	return true;
}