#include <SceneObjects/Weapons/WeaponAssetPack.h>

#include <Renderer/SDL_OpenGL.h>

#include <Utilities/UtilFuncs.h>

#include <sstream>
#include <fstream>

#include <assert.h>

WeaponAssetPack::WeaponAssetPack()
	: m_loaded(false), m_swayMultiplier(0.1f), m_renderersSet(false)
{
}

WeaponAssetPack::~WeaponAssetPack()
{
	// Delete weapon models
	for(std::unordered_map<std::string, std::vector<Model_And_Params*>>::iterator it = m_categorizedModels.begin(); it != m_categorizedModels.end(); it++)
	{
		for(unsigned int i = 0, size = it->second.size(); i < size; i++)
			delete it->second[i];
	}
}

bool WeaponAssetPack::LoadAsset(const std::string &name)
{
	std::ifstream fromFile(name);

	if(!fromFile.is_open())
	{
		std::cerr << "Could not open weapon descriptor file \"" << name << "\"!" << std::endl;

		fromFile.close();

		return false;
	}

	// Get relative texture name
	unsigned int i;

	for(i = name.size() - 1; i > 0; i--)
	{
		if(name[i] == '/' || name[i] == '\\')
			break;
	}

	std::string rootName(name.substr(0, i + 1));

	// Add tex name on to normal name (relative to model file name)
	std::stringstream fullTexName;

	std::string param;

	// ----------------------------- Load random texture -----------------------------

	std::string randomTextureName;

	fromFile >> param;
	assert(param == "RandomTexture:");
	fromFile >> randomTextureName; 

	// Remove quotes
	RemoveOuterCharacters(randomTextureName);

	fullTexName << rootName << randomTextureName;

	if(!m_randomTexture_diffuse.LoadAsset(fullTexName.str()))
	{
		fromFile.close();

		return false;
	}

	m_randomTexture_diffuse.Bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	fromFile >> randomTextureName; 

	// Remove quotes
	RemoveOuterCharacters(randomTextureName);

	fullTexName.str("");
	fullTexName << rootName << randomTextureName;

	if(!m_randomTexture_normal.LoadAsset(fullTexName.str()))
	{
		fromFile.close();

		return false;
	}

	m_randomTexture_normal.Bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	fromFile >> randomTextureName; 

	// Remove quotes
	RemoveOuterCharacters(randomTextureName);

	fullTexName.str("");
	fullTexName << rootName << randomTextureName;

	if(!m_randomTexture_specular.LoadAsset(fullTexName.str()))
	{
		fromFile.close();

		return false;
	}

	m_randomTexture_specular.Bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	m_randomTextureDimsInverse = Vec2f(1.0f / static_cast<float>(m_randomTexture_diffuse.GetWidth()), 1.0f / static_cast<float>(m_randomTexture_diffuse.GetHeight()));

	// Texture filtering mode
	//m_randomTexture_diffuse.Bind();
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Load texture biases
	int numBiases;

	fromFile >> param;
	assert(param == "NumBiases:");
	fromFile >> numBiases;

	for(int i = 0; i < numBiases; i++)
	{
		fromFile >> param;
		assert(param == "Bias");

		// Bias name
		std::string biasName;
		fromFile >> biasName;

		// Remove quotes
		RemoveOuterCharacters(biasName);

		// Bias position
		Vec2f biasPos;
		fromFile >> biasPos.x >> biasPos.y;

		biasPos *= m_randomTextureDimsInverse;

		m_randomTextureRegionBias[biasName] = biasPos;
	}

	// ----------------------------- Load Models in Categories -----------------------------

	int numModels;

	fromFile >> param;
	assert(param == "NumModels:");
	fromFile >> numModels;

	for(int i = 0; i < numModels; i++)
	{
		fromFile >> param;
		assert(param == "Model");

		// Model category
		std::string categoryName;
		fromFile >> categoryName;

		RemoveOuterCharacters(categoryName);

		// Model name
		std::string modelRootName;
		fromFile >> modelRootName;

		RemoveOuterCharacters(modelRootName);

		Model_And_Params* pNewModelAndParams = new Model_And_Params();

		m_categorizedModels[categoryName].push_back(pNewModelAndParams);

		// Load model
		std::stringstream fullModelName;
		fullModelName << rootName << modelRootName;

		if(!pNewModelAndParams->m_model.LoadAsset(fullModelName.str()))
		{
			fromFile.close();

			return false;
		}

		// Default texture setting: nearest filtering
		/*for(unsigned int i = 0, size = pNewModelAndParams->m_model.GetNumMaterials(); i < size; i++)
		{
			Model_OBJ::Material* pMat = pNewModelAndParams->m_model.GetMaterial(i);
		
			pMat->m_pDiffuseMap->Bind();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			if(pMat->m_pSpecularMap != NULL)
			{
				pMat->m_pSpecularMap->Bind();
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}

			if(pMat->m_pNormalMap != NULL)
			{
				pMat->m_pNormalMap->Bind();
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}
		}*/

		// Check for parameters
		std::string restOfLine;

		getline(fromFile, restOfLine);

		if(!restOfLine.empty())
		{
			// Set parameters
			pNewModelAndParams->m_params = restOfLine;
		}
	}

	fromFile.close();

	m_loaded = true;

	return true;
}

void WeaponAssetPack::SetModelRenderers(Scene* pScene)
{
	for(std::unordered_map<std::string, std::vector<Model_And_Params*>>::iterator it = m_categorizedModels.begin();
		it != m_categorizedModels.end();
		it++)
	{
		for(unsigned int i = 0, size = it->second.size(); i < size; i++)
			it->second[i]->m_model.SetRenderer(pScene);
	}

	m_renderersSet = true;
}

void WeaponAssetPack::BindRandomWeaponTextures()
{
	assert(m_loaded);

	m_randomTexture_diffuse.Bind();

	glActiveTexture(GL_TEXTURE1);
	m_randomTexture_specular.Bind();

	glActiveTexture(GL_TEXTURE3);
	m_randomTexture_normal.Bind();

	glActiveTexture(GL_TEXTURE0);
}

Vec2f WeaponAssetPack::GetRandomTextureOffset(const std::string &regionName)
{
	assert(m_loaded);

	// Sway towards region
	std::unordered_map<std::string, Vec2f>::iterator it = m_randomTextureRegionBias.find(regionName);

	if(it == m_randomTextureRegionBias.end())
	{
		std::cerr << "Could not find region \"" << regionName << "\"!" << std::endl;
		abort();
	}

	Vec2f randOffset((rand() % 1000) / 1000.0f, (rand() % 1000) / 1000.0f);

	Vec2f randPos = it->second + randOffset * m_swayMultiplier;

	// Use exponent to sway randomness over to bias region
	return randPos;
}

WeaponAssetPack::Model_And_Params* WeaponAssetPack::GetRandomModel(const std::string &category)
{
	assert(m_loaded);
	assert(m_renderersSet);

	std::unordered_map<std::string, std::vector<Model_And_Params*>>::iterator it = m_categorizedModels.find(category);

	if(it == m_categorizedModels.end())
	{
		std::cerr << "Could not find category \"" << category << "\"!" << std::endl;
		abort();
	}

	std::vector<Model_And_Params*> &categoryModels = it->second;

	return categoryModels[rand() % categoryModels.size()];
}

const Vec2f &WeaponAssetPack::GetRandomTextureDimsInverse()
{
	return m_randomTextureDimsInverse;
}

Asset* WeaponAssetPack::Asset_Factory()
{
	return new WeaponAssetPack();
}