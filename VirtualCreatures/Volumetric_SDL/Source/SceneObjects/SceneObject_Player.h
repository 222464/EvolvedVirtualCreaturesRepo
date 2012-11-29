#pragma once

#include <Scene/SceneObject.h>
#include <Renderer/Model_OBJ.h>
#include <World/Physics/MapCollider.h>

#include <SceneObjects/Weapons/Weapon.h>

#include <SceneEffects/SceneEffect_EmissiveRender.h>

#include <Utilities/UtilFuncs.h>

#include <SceneEffects/SceneEffect_Lighting.h>

#include <SceneEffects/Light_Spot_Shadowed.h>

#include <SceneObjects/Physics/DynamicCharacterController.h>

#include <PathFinding/AStar.h>

struct SpriteEmissiveRenderable :
	public SceneEffect_EmissiveRender::EmissiveRenderable
{
	Sprite m_sprite;

	Scene* m_pScene;

	Matrix4x4f* m_pBarrelTipTransform;

	// Inherited from EmissiveRenderable
	void Render()
	{
		m_pScene->SetWorldMatrix(*m_pBarrelTipTransform);
		m_sprite.Render();
		m_pScene->SetWorldMatrix(Matrix4x4f::IdentityMatrix());
	}
};

class SceneObject_Player :
	public SceneObject
{
private:
	MapCollider m_collider;

	float m_xRot, m_yRot;

	Vec3f m_facingDirection;

	Model_OBJ m_armModel;

	bool m_onGround;

	bool m_mouseLocked;

	enum ShootingAnimationState
	{
		e_notShooting, e_recoiling, e_returning
	} m_shootingState;

	Vec3f m_weaponOffset;

	float m_shootingAnimationTimer;

	Matrix4x4f m_playerTransform_noRecoil; // Transform without recoil 
	Matrix4x4f m_playerTransform; // With recoil
	Matrix4x4f m_baseWeaponTransform_noRecoil; // Transform without recoil
	Matrix4x4f m_baseWeaponTransform; // With recoil
	Matrix4x4f m_barrelTipTransform;

	Vec3f m_barrelTipPos;
	Vec3f m_handDelayOffset;

	Vec3f m_shellSpawnPointWorldVelocity;

	Weapon m_weapon;

	SoundSource m_gunSoundSource;

	SpriteEmissiveRenderable m_muzzleFlash;

	SceneEffect_EmissiveRender* m_pEmissiveRender;

	SceneEffect_Lighting* m_pLighting;
	Light_Point* m_pMuzzleFlashLight;

	float m_handDelayTime;

	std::list<Vec3f> m_pastFrameBarrelPositions;

	// Physics
	SceneObject_PhysicsWorld* m_pPhysicsWorld;

	std::unique_ptr<DynamicCharacterController> m_pCharacterController;

	void CreatePhysicsShape();
	void DestroyPhysicsShape();

	// Different physics functions for switching between normal physics and no clip
	void (SceneObject_Player::*m_pPhysicsFunc)();

	void RunPhysics();
	void RunNoClip();

	bool m_physicsMode;

	Light_Spot_Shadowed* m_pFlashLight;

	Point3i end;

	PathFindingTask m_task;

public:
	float m_acceleration_walk;
	float m_acceleration_run;
	float m_deceleration_walk;
	float m_deceleration_run;
	float m_maxSpeed_walk;
	float m_maxSpeed_run;
	float m_mouseSensitivity;

	float m_jumpVel;

	SceneObject_Player();
	~SceneObject_Player();

	// Inherited from SceneObject
	void OnAdd();
	void Logic();
	void Render();

	Vec3f GetViewVec();
	Vec3f GetPosition();

	void UsePhysics(bool use);

	bool UsingPhysics() const;
};

