#include <SceneObjects/Particles/SceneObject_ParticleEmitter_Sprite_Point.h>

#include <Utilities/UtilFuncs.h>

SceneObject_ParticleEmitter_Sprite_Point::SceneObject_ParticleEmitter_Sprite_Point()
	: m_minSpeed(0.1f), m_maxSpeed(0.2f)
{
}

void SceneObject_ParticleEmitter_Sprite_Point::SetParticle(Particle_Sprite* pParticle)
{
	// Random direction
	pParticle->m_position = m_position;
	pParticle->m_velocity = Vec3f(Randf(-1.0f, 1.0f), Randf(-1.0f, 1.0f), Randf(-1.0f, 1.0f));

	// Zero vector check
	if(pParticle->m_velocity == Vec3f(0.0f, 0.0f, 0.0f))
		pParticle->m_velocity = Vec3f(1.0f, 0.0f, 0.0f);
	else
		pParticle->m_velocity.NormalizeThis();

	// Random speed
	pParticle->m_velocity *= Randf(m_minSpeed, m_maxSpeed);
}