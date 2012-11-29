#pragma once

#include <Constructs/Vec3f.h>
#include <Constructs/Color3f.h>
#include <Renderer/Sprite.h>

#include <Scene/Scene.h>

class Particle_Sprite
{
private:
	Scene* m_pScene;

	Sprite* m_pSprite;

	float m_radius;

	float m_age;
	float m_despawnTime;

public:
	Vec3f m_position;
	Vec3f m_velocity;
	float m_scale;

	Color3f m_color;

	float m_animationSpeed;

	Particle_Sprite();

	bool Logic(); // Returns true if particle should be destroyed
	void Render(const Matrix4x4f &particleDirectionMatrix);
	float GetRadius();

	Scene* GetScene() const
	{
		return m_pScene;
	}

	friend class SceneObject_ParticleEmitter_Sprite;
};

