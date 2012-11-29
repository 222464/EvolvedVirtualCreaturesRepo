#pragma once

#include <Renderer/Model_MD5/Model_MD5.h>
#include <Renderer/Animation_MD5/Animation_MD5.h>

#include <SceneObjects/Enemies/SceneObject_Enemy.h>
#include <SceneObjects/SceneObject_Player.h>

#include <SceneObjects/Physics/SceneObject_PhysicsWorld.h>

#include <SceneObjects/Particles/SceneObject_ParticleEmitter_Sprite_Point.h>

/*class SwarmerAnimator
{
};*/

class SceneObject_Enemy_Swarmer :
	public SceneObject_Enemy
{
private:
	Model_MD5* m_pModel;
	Animation_MD5* m_pAnimation;

	float m_animationTime;

	Quaternion m_rotation;
	float m_faceAngleXZ;
	Vec3f m_position;

	// Player reference
	SceneObject_Player* m_pPlayer;

	// Physics
	SceneObjectReferenceTracker m_physicsWorldTracker;
	SceneObject_PhysicsWorld* m_pPhysicsWorld;

	btCollisionShape* m_pCollisionShape;
	btDefaultMotionState* m_pMotionState;
	btRigidBody* m_pRigidBody;

	void Die();

	static const int s_maxNumSwarmers = 32;
	static int s_numSwarmers;

	float m_splitTimer;
	float m_splitTime;

	void Split();

	float GetSplitTime()
	{
		return (rand() % 1000) / 5.0f + 300.0f;
	}

	// Particle effects
	SceneObjectReferenceTracker m_particleEffectTracker;
	SceneObject_ParticleEmitter_Sprite_Point* m_pParticleEffect;

public:
	float m_agroRange;

	SceneObject_Enemy_Swarmer(const Vec3f &position);
	~SceneObject_Enemy_Swarmer();

	// Inherited from SceneObject
	void OnAdd();
	void Logic();
	void Render();

	// Inherited from SceneObject_Enemy
	std::string GetTypeName();

	friend SceneObject_Enemy_Swarmer;
};

