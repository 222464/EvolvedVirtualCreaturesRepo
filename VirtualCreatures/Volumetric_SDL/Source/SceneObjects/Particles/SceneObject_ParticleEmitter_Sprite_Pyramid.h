#pragma once

#include <SceneObjects/Particles/SceneObject_ParticleEmitter_Sprite.h>

class SceneObject_ParticleEmitter_Sprite_Pyramid :
	public SceneObject_ParticleEmitter_Sprite
{
private:
	// Inherited from SceneObject_ParticleEmitter_Sprite
	void SetParticle(Particle_Sprite* pParticle);

public:
	float m_size; // Width/Height of base of pyramid (depth is 1)

	Vec3f m_direction;

	float m_minSpeed;
	float m_maxSpeed;

	SceneObject_ParticleEmitter_Sprite_Pyramid();
};

