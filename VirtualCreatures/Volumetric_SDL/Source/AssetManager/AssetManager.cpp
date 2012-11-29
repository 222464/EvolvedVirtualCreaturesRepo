#include <AssetManager/AssetManager.h>

#include <assert.h>

AssetManager::AssetManager()
	: m_assetFactory(NULL)
{
}

AssetManager::AssetManager(Asset* (*assetFactory)())
	: m_assetFactory(assetFactory)
{
}

AssetManager::~AssetManager()
{
	ClearAssets();
}

void AssetManager::Create(Asset* (*assetFactory)())
{
	m_assetFactory = assetFactory;
}

bool AssetManager::Created()
{
	return m_assetFactory != NULL;
}

bool AssetManager::GetAsset(const std::string &name)
{
	assert(m_assetFactory != NULL);

	std::unordered_map<std::string, Asset*>::iterator it = m_assets.find(name);

	if(it == m_assets.end())
	{
		// Load the m_asset
		Asset* pAsset = m_assetFactory();
		
		if(!pAsset->LoadAsset(name))
			return false;

		m_assets[name] = pAsset;

		return true;
	}

	return true;
}

bool AssetManager::GetAsset(const std::string &name, Asset* &pAsset)
{
	assert(m_assetFactory != NULL);

	std::unordered_map<std::string, Asset*>::iterator it = m_assets.find(name);

	if(it == m_assets.end())
	{
		// Load the m_asset
		pAsset = m_assetFactory();

		if(!pAsset->LoadAsset(name))
			return false;

		m_assets[name] = pAsset;
	}
	else
		pAsset = it->second;

	return true;
}

void AssetManager::DestroyAsset(const std::string &name)
{
	std::unordered_map<std::string, Asset*>::iterator it = m_assets.find(name);

	if(it != m_assets.end())
	{
		delete it->second;
		m_assets.erase(it);
	}
}

void AssetManager::ClearAssets()
{
	for(std::unordered_map<std::string, Asset*>::iterator it = m_assets.begin(); it != m_assets.end(); it++)
		delete it->second;

	m_assets.clear();
}
