#pragma once

#include <SceneObjects/Particles/Particle_Sprite.h>

#include <Scene/SceneObject.h>

class SceneObject_ParticleEmitter_Sprite :
	public SceneObject
{
private:
	std::list<Particle_Sprite*> m_pParticles;

	struct SpriteAndRadius
	{
		Sprite m_sprite;
		float m_radius;
	};

	std::vector<SpriteAndRadius> m_sprites;

	typedef void (*ParticleAffectorFunc)(Particle_Sprite* pParticle);

	// Affector list
	std::vector<ParticleAffectorFunc> m_pAffectorFuncs;

	Particle_Sprite* EmitParticle();

	float m_spawnTimer;
	float m_spawnTime;

protected:
	virtual void SetParticle(Particle_Sprite* pParticle) = 0;

public:
	Vec3f m_position;

	float m_minSpawnTime;
	float m_maxSpawnTime;

	float m_minDespawnTime;
	float m_maxDespawnTime;

	bool m_emit;

	bool m_autoDestruct;

	SceneObject_ParticleEmitter_Sprite();
	virtual ~SceneObject_ParticleEmitter_Sprite();

	void AddSprite(const Sprite &sprite);

	void AddAffector(ParticleAffectorFunc pAffectorFunc);

	void SpawnBurst(int numParticles);

	void Clear();

	// Inherited from SceneObject
	void Logic();
	void Render();
};

// Some useful affectors
void Affector_Gravity(Particle_Sprite* pParticle);
