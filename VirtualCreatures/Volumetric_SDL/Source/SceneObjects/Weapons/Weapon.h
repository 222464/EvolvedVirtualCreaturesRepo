#pragma once

#include <Scene/Scene.h>

#include <SceneObjects/Weapons/WeaponAssetPack.h>

#include <SceneObjects/Physics/SceneObject_PhysicsWorld.h>

#include <Renderer/BufferObjects/VBO.h>

#include <Renderer/Octree/StaticOctree.h>

#include <SceneObjects/Weapons/Wire.h>

#include <Sound/SoundSystem.h>

#include <Renderer/Sprite.h>

#include <string>

class Weapon
{
public:
	enum WeaponType
	{
		e_ballistic, e_directedEnergy, e_biological, e_worldAffector
	};

private:
	WeaponType m_type;

	WeaponAssetPack* m_pAssetPack;

	struct ModelAndTransform
	{
		Matrix4x4f m_transform;
		Model_OBJ* pModel;
	};

	// Models part of weapon
	std::vector<ModelAndTransform> m_models;

	// VBOs for generated geometry - not using indices (too annoying to generate)
	VBO m_positions;
	VBO m_texCoords;
	VBO m_normals;

	unsigned int m_numVertices;

	Model_OBJ* m_pShellModel;
	Sound_Effect* m_pShootSound;

	Asset_Texture* m_pMuzzleFlashSpriteSheet;

	Scene* m_pScene;
	Vec3f m_handPos;

	Vec3f m_barrelTipPos;

	AABB m_genGeomBounds;
	AABB m_entireWeaponBounds;

	SceneObject_PhysicsWorld* m_pPhysicsWorld;

	//Vec3f m_aimEyePosOffsetFromHand;
	//bool m_hasScope;

	Wire m_testWire;

	bool m_created;

public:
	float m_damage;

	Weapon();

	bool Created();

	const Vec3f &GetHandPos() const;

	void Render(const Matrix4x4f &baseTransform);

	const AABB &GetAABB();

	float GetRecoilingTime();
	float GetReturningTime();

	void Shoot(const Vec3f m_barrelTipPos, const Quaternion &baseAngle, const Vec3f &baseVel);
	void EjectShell(const Matrix4x4f &baseTransform, const Quaternion &baseAngle, const Vec3f &baseVel);

	Vec3f GetEjectPos() const;
	const Vec3f &GetBarrelTipPos() const;

	Sound_Effect* GetShootSound() const;

	Asset_Texture* GetMuzzleFlashSpriteSheet() const;

	friend class WeaponFactory;
};

