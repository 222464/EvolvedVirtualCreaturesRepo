#include <SceneObjects/Particles/SceneObject_ParticleEmitter_Sprite.h>

#include <SceneObjects/SceneObject_Player.h>

#include <Utilities/UtilFuncs.h>

#include <assert.h>

SceneObject_ParticleEmitter_Sprite::SceneObject_ParticleEmitter_Sprite()
	: m_position(0.0f, 0.0f, 0.0f), m_spawnTimer(0.0f), m_minSpawnTime(0.25f), m_maxSpawnTime(0.5f),
	m_minDespawnTime(60.0f), m_maxDespawnTime(80.0f),
	m_emit(true), m_autoDestruct(false)
{
}

SceneObject_ParticleEmitter_Sprite::~SceneObject_ParticleEmitter_Sprite()
{
	Clear();
}

void SceneObject_ParticleEmitter_Sprite::AddSprite(const Sprite &sprite)
{
	SpriteAndRadius spriteAndRadius;
	spriteAndRadius.m_sprite = sprite;
	spriteAndRadius.m_radius = std::sqrtf(powf(sprite.GetHalfWidth(), 2.0f) + powf(sprite.GetHalfHeight(), 2.0f));

	m_sprites.push_back(spriteAndRadius);
}

void SceneObject_ParticleEmitter_Sprite::AddAffector(ParticleAffectorFunc pAffectorFunc)
{
	m_pAffectorFuncs.push_back(pAffectorFunc);
}

void SceneObject_ParticleEmitter_Sprite::SpawnBurst(int numParticles)
{
	for(int i = 0; i < numParticles; i++)
		m_pParticles.push_back(EmitParticle());
}

Particle_Sprite* SceneObject_ParticleEmitter_Sprite::EmitParticle()
{
	assert(GetScene() != NULL);

	Particle_Sprite* pParticle = new Particle_Sprite();

	pParticle->m_pScene = GetScene();

	// Select random sprite
	assert(!m_sprites.empty());

	int index = rand() % m_sprites.size();

	pParticle->m_pSprite = &m_sprites[index].m_sprite;
	pParticle->m_radius = m_sprites[index].m_radius;

	pParticle->m_despawnTime = Randf(m_minDespawnTime, m_maxDespawnTime);

	SetParticle(pParticle);

	return pParticle;
}

void SceneObject_ParticleEmitter_Sprite::Clear()
{
	for(std::list<Particle_Sprite*>::iterator it = m_pParticles.begin(); it != m_pParticles.end(); it++)
		delete *it;

	m_pParticles.clear();
}

void SceneObject_ParticleEmitter_Sprite::Logic()
{
	// ----------------------- Spawning -----------------------

	if(m_emit)
	{
		m_spawnTimer += GetScene()->m_frameTimer.GetTimeMultiplier();

		if(m_spawnTimer > m_spawnTime)
		{
			// Spawn multiple per frame, if went over more than one multiple of the spawn time
			int numSpawn = static_cast<int>(m_spawnTimer / m_spawnTime);

			// Spawn particles
			for(int i = 0; i < numSpawn; i++)
				m_pParticles.push_back(EmitParticle());

			// Reset timer, keep overflow (fmodf equivalent)
			m_spawnTimer -= numSpawn * m_spawnTime;

			// Pick new random spawn time
			m_spawnTime = Randf(m_minSpawnTime, m_maxSpawnTime);
		}
	}

	// ----------------------- Updating -----------------------

	if(m_pParticles.empty())
	{
		if(m_autoDestruct)
			Destroy();
	}
	else
	{
		// Recalculate AABB while at it
		m_aabb.m_lowerBound = m_pParticles.front()->m_position;
		m_aabb.m_upperBound = m_aabb.m_lowerBound;

		for(std::list<Particle_Sprite*>::iterator it = m_pParticles.begin(); it != m_pParticles.end();)
		{
			Particle_Sprite* pParticle = *it;

			if(pParticle->Logic())
			{
				// Destroy particle
				delete pParticle;

				it = m_pParticles.erase(it);
			}
			else
			{
				// Affect particle
				for(unsigned int i = 0, size = m_pAffectorFuncs.size(); i < size; i++)
					m_pAffectorFuncs[i](pParticle);

				// Grow AABB
				float radius = pParticle->GetRadius();

				Vec3f particleLower(pParticle->m_position - Vec3f(radius, radius, radius));
				Vec3f particleUpper(pParticle->m_position + Vec3f(radius, radius, radius));
				
				if(particleLower.x < m_aabb.m_lowerBound.x)
					m_aabb.m_lowerBound.x = pParticle->m_position.x - pParticle->m_radius;

				if(particleLower.y < m_aabb.m_lowerBound.y)
					m_aabb.m_lowerBound.y = pParticle->m_position.y - pParticle->m_radius;

				if(particleLower.z < m_aabb.m_lowerBound.z)
					m_aabb.m_lowerBound.z = pParticle->m_position.z - pParticle->m_radius;

				if(particleUpper.x > m_aabb.m_upperBound.x)
					m_aabb.m_upperBound.x = pParticle->m_position.x + pParticle->m_radius;

				if(particleUpper.y > m_aabb.m_upperBound.y)
					m_aabb.m_upperBound.y = pParticle->m_position.y + pParticle->m_radius;

				if(particleUpper.z > m_aabb.m_upperBound.z)
					m_aabb.m_upperBound.z = pParticle->m_position.z + pParticle->m_radius;

				it++;
			}
		}

		if(IsSPTManaged())
			TreeUpdate();
	}
}

void SceneObject_ParticleEmitter_Sprite::Render()
{
	// Calculate camera direction matrix
	Matrix4x4f cameraDirectionMatrix(Matrix4x4f::DirectionMatrix(GetScene()->m_camera.m_position - m_position, Vec3f(0.0f, 1.0f, 0.0f)));

	for(std::list<Particle_Sprite*>::iterator it = m_pParticles.begin(); it != m_pParticles.end(); it++)
		(*it)->Render(cameraDirectionMatrix);
}

void Affector_Gravity(Particle_Sprite* pParticle)
{
	pParticle->m_velocity.y -= pParticle->GetScene()->m_frameTimer.GetTimeMultiplier() * 0.027f;
}