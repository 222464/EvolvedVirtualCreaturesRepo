#pragma once

#include <Scene/Scene.h>

#include <SceneObjects/Weapons/WeaponAssetPack.h>

#include <Renderer/BufferObjects/VBO.h>

#include <Renderer/Octree/StaticOctree.h>

#include <SceneObjects/Weapons/Weapon.h>

#include <SceneObjects/Weapons/Wire.h>

#include <Sound/SoundSystem.h>

#include <Renderer/Sprite.h>

#include <string>

class WeaponFactory
{
private:
	Weapon* m_pWeapon;

	StaticOctree m_geomSPT;

	struct GeomOccupant :
		public OctreeOccupant
	{
		void SetAABB(const AABB &aabb)
		{
			m_aabb = aabb;
		}
	};

	std::vector<std::unique_ptr<GeomOccupant>> m_geom;

	void Create(const std::string &baseCategory,
		bool wiring,
		int initNumBranches, float maxNumBranches, float minNumBranches,
		float preferBackFactor, float doNotPreferSidesFactor,
		const Vec3f &scaling, float branchSizeDecreaseMultiplier, const AABB &maxBranchBounds);

	Vec3f RayCastGeom(const Vec3f &origin, const Vec3f &direction);

	void GenGeom_Box(const Matrix4x4f &transform, const Vec3f &texScalar, const Vec2f &texOffset,
		std::vector<Vec3f> &positions,
		std::vector<Vec2f> &texCoords,
		std::vector<Vec3f> &normals);

	// Recursive branching
	void BranchGeom(const AABB &prevLocalBox,
		float downScalar,
		float maxNumBranches, float minBranches,
		float preferBackFactor, float doNotPreferSidesFactor);

	void SpamAttachments(const std::string &baseCategory);

public:
	float m_genGeomTexScalar;
	Vec3f m_scalar; // Stretch guns length wise
	float m_minBranchScale;
	float m_branchShrinkFactor;
	float m_generatedGeomScalar;
	float m_preferBackIncreaseFactor;
	float m_maxBranchDecreaseFactor;
	float m_minBranchDecreaseFactor;
	float m_doNotPreferSidesDecreaseFactor;
	float m_minTriggerOffsetFromStock;
	float m_maxTriggerOffsetFromStock;
	float m_minHandTriggerOffset;
	float m_maxHandTriggerOffset;
	float m_positionTestIncrement;
	AABB m_maxBranchBounds;
	Vec3f m_backMostPos;
	Vec3f m_scopePos;
	bool m_foundScopePos;
	Model_OBJ* m_selectedScope;
	Vec3f m_baseBoxScale;
	std::string m_currentRandomTextureRegion;

	WeaponFactory();

	bool Create(Weapon* weapon, Weapon::WeaponType type, const std::string &weaponAssetPackName, Scene* pScene);
};

