#pragma once

#include <SceneObjects/Enemies/SceneObject_Enemy.h>

#include <unordered_map>

#define ENEMY_SELECTOR_NAME "enemSel"

typedef SceneObject_Enemy* (*EnemyFactoryFunc)(const Vec3f &position, float yRot);

struct EnemyDesc
{
	std::string name;

	// Difficulty
	int m_tier;

	// Spawn limits
	int m_count;
	int m_maxCount;

	// Factory function
	EnemyFactoryFunc m_pFactory;

	// Automatically deleted pointer to user data
	void* m_pUserData;

	EnemyDesc();
	EnemyDesc(const std::string &name, int tier, int maxCount, EnemyFactoryFunc pFactory);
	EnemyDesc(const std::string &name, int tier, int maxCount, EnemyFactoryFunc pFactory, void* pUserData);
	~EnemyDesc();
};

class SceneObject_EnemySelector :
	public SceneObject
{
private:
	// Link tiers to a list of enemies
	std::vector<std::vector<std::unique_ptr<EnemyDesc>>> m_tiers;

	// Link names to tiers and enemies
	std::unordered_map<std::string, EnemyDesc*> m_nameToDesc;

	void ExpandTiers(int tier);

public:
	void RegisterEnemy(const std::string &name, int tier, int maxSpawn, EnemyFactoryFunc pFactory);

	// pUserData must be heap allocated, it will automatically be deleted when this SceneObject_EnemySelector is destroyed
	void RegisterEnemy(const std::string &name, int tier, int maxSpawn, EnemyFactoryFunc pFactory, void* pUserData);

	// Returns true if spawn was successful
	bool SpawnByName(const std::string &name, const Vec3f &position, float yRot);
	bool SpawnByTier(int tier, const Vec3f &position, float yRot);

	// Changes pDesc to desc of spawned enemy
	bool SpawnByName(const std::string &name, const Vec3f &position, float yRot, EnemyDesc* &pDesc);
	bool SpawnByTier(int tier, const Vec3f &position, float yRot, EnemyDesc* &pDesc);

	EnemyDesc* GetDesc(const std::string &name);
	EnemyDesc* GetRandomDesc(int tier);
};

