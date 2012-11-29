#include <SceneObjects/Enemies/SceneObject_EnemySelector.h>

#include <Scene/Scene.h>

#include <assert.h>

EnemyDesc::EnemyDesc()
	: m_pUserData(NULL), m_count(0)
{
}

EnemyDesc::EnemyDesc(const std::string &name, int tier, int maxCount, EnemyFactoryFunc pFactory)
	: m_pUserData(NULL), m_count(0),
	m_tier(tier), m_maxCount(maxCount), m_pFactory(pFactory)
{
}

EnemyDesc::EnemyDesc(const std::string &name, int tier, int maxCount, EnemyFactoryFunc pFactory, void* pUserData)
	: m_count(0),
	m_tier(tier), m_maxCount(maxCount), m_pFactory(pFactory), m_pUserData(pUserData)
{
}

EnemyDesc::~EnemyDesc()
{
	if(m_pUserData != NULL)
		delete m_pUserData;
}

void SceneObject_EnemySelector::ExpandTiers(int tier)
{
	for(int i = m_tiers.size(); i <= tier; i++)
		m_tiers.push_back(std::vector<std::unique_ptr<EnemyDesc>>());
}

void SceneObject_EnemySelector::RegisterEnemy(const std::string &name, int tier, int maxSpawn, EnemyFactoryFunc pFactory)
{
	ExpandTiers(tier);

	EnemyDesc* pDesc = new EnemyDesc(name, tier, maxSpawn, pFactory);

	m_tiers[tier].push_back(std::unique_ptr<EnemyDesc>(pDesc));

	m_nameToDesc[name] = pDesc;
}

void SceneObject_EnemySelector::RegisterEnemy(const std::string &name, int tier, int maxSpawn, EnemyFactoryFunc pFactory, void* pUserData)
{
	ExpandTiers(tier);

	EnemyDesc* pDesc = new EnemyDesc(name, tier, maxSpawn, pFactory, pUserData);

	m_tiers[tier].push_back(std::unique_ptr<EnemyDesc>(pDesc));

	m_nameToDesc[name] = pDesc;
}

bool SceneObject_EnemySelector::SpawnByName(const std::string &name, const Vec3f &position, float yRot)
{
	EnemyDesc* pDesc = GetDesc(name);

	if(pDesc == NULL)
	{
		std::cerr << "Could not find enemy \"" << name << "\"!" << std::endl;

		return false;
	}

	assert(pDesc->m_pFactory != NULL);

	if(pDesc->m_count < pDesc->m_maxCount)
	{
		SceneObject_Enemy* pEnemy = (*pDesc->m_pFactory)(position, yRot);

		if(pDesc->m_pUserData != NULL)
			pEnemy->SetUserData(pDesc->m_pUserData);

		GetScene()->Add(pEnemy, true);
	}

	return true;
}

bool SceneObject_EnemySelector::SpawnByTier(int tier, const Vec3f &position, float yRot)
{
	// Random
	assert(tier >= 0 && tier < static_cast<signed>(m_tiers.size()));

	EnemyDesc* pDesc = m_tiers[tier][rand() % m_tiers[tier].size()].get();

	assert(pDesc->m_pFactory != NULL);

	if(pDesc->m_count < pDesc->m_maxCount)
	{
		SceneObject_Enemy* pEnemy = (*pDesc->m_pFactory)(position, yRot);

		if(pDesc->m_pUserData != NULL)
			pEnemy->SetUserData(pDesc->m_pUserData);

		GetScene()->Add(pEnemy, true);
	}

	return true;
}

bool SceneObject_EnemySelector::SpawnByName(const std::string &name, const Vec3f &position, float yRot, EnemyDesc* &pDesc)
{
	pDesc = GetDesc(name);

	if(pDesc == NULL)
	{
		std::cerr << "Could not find enemy \"" << name << "\"!" << std::endl;

		return false;
	}

	assert(pDesc->m_pFactory != NULL);

	if(pDesc->m_count < pDesc->m_maxCount)
	{
		SceneObject_Enemy* pEnemy = (*pDesc->m_pFactory)(position, yRot);

		if(pDesc->m_pUserData != NULL)
			pEnemy->SetUserData(pDesc->m_pUserData);

		GetScene()->Add(pEnemy, true);
	}
	
	return true;
}

bool SceneObject_EnemySelector::SpawnByTier(int tier, const Vec3f &position, float yRot, EnemyDesc* &pDesc)
{
	// Random
	assert(tier >= 0 && tier < static_cast<signed>(m_tiers.size()));

	pDesc = m_tiers[tier][rand() % m_tiers[tier].size()].get();

	assert(pDesc->m_pFactory != NULL);

	if(pDesc->m_count < pDesc->m_maxCount)
	{
		SceneObject_Enemy* pEnemy = (*pDesc->m_pFactory)(position, yRot);

		if(pDesc->m_pUserData != NULL)
			pEnemy->SetUserData(pDesc->m_pUserData);

		GetScene()->Add(pEnemy, true);
	}
	
	return true;
}

EnemyDesc* SceneObject_EnemySelector::GetDesc(const std::string &name)
{
	std::unordered_map<std::string, EnemyDesc*>::iterator it = m_nameToDesc.find(name);

	if(it == m_nameToDesc.end())
		return NULL;

	return it->second; 
}

EnemyDesc* SceneObject_EnemySelector::GetRandomDesc(int tier)
{
	return NULL;
}