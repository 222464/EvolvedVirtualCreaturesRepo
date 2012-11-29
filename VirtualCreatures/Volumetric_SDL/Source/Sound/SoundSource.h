#pragma once

#include <Sound/Sound.h>

#include <Constructs/Vec3f.h>

class SoundSource
{
private:
	unsigned int m_sourceID;

	Sound* m_pSound;

	float m_pitch;
	float m_gain;
	Vec3f m_position;
	Vec3f m_velocity;
	float m_referenceDistance;
	float m_maxDistance;
	float m_rollOffFactor;

	bool m_looping;

public:
	SoundSource();
	~SoundSource();

	// Update sound source
	void SetSound(Sound* pSound);
	Sound* GetSound() const;
	void SetPosition(const Vec3f &position);
	const Vec3f &GetPosition() const;
	void SetVelocity(const Vec3f &velocity);
	const Vec3f &GetVelocity() const;
	void SetPitch(float pitch);
	float GetPitch() const;
	void SetGain(float gain);
	float GetGain() const;
	void SetLooping(bool looping);
	bool GetLooping() const;
	void SetReferenceDistance(float referenceDistance);
	float GetReferenceDistance() const;
	void SetMaxDistance(float maxDistance);
	float GetMaxDistance() const;
	void SetRollOffFactor(float rollOffFactor);
	float GetRollOffFactor() const;

	bool Play();
	void Stop();
	void Rewind();
	void Pause();

	bool Playing() const;

	unsigned int GetSourceID();
};

