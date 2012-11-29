#pragma once

#include <AssetManager/Asset.h>

#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>

#include <vector>

class DynamicConcaveShape :
	public Asset
{
private:
	btCompoundShape m_compoundShape;

	std::vector<btConvexHullShape*> m_pConvexMeshes;

	bool m_loaded;

public:
	DynamicConcaveShape();
	~DynamicConcaveShape();

	// Inherited from the asset manager
	bool LoadAsset(const std::string &name);

	btCompoundShape* GetShape();

	unsigned int GetNumConvexMeshes() const;
	btConvexHullShape* GetConvexMesh(unsigned int index);

	// Asset factory
	static Asset* Asset_Factory();
};

