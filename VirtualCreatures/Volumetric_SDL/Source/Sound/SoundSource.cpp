#include <Sound/SoundSource.h>

#include <Sound/OpenALUtils.h>

#include <assert.h>

SoundSource::SoundSource()
	: m_pSound(NULL)
{
	alGenSources(1, &m_sourceID);
}

SoundSource::~SoundSource()
{
	alDeleteSources(1, &m_sourceID);
}

void SoundSource::SetSound(Sound* pSound)
{
	m_pSound = pSound;

	if(m_pSound != NULL)
		m_pSound->SetUpSoundSource(this);
}

Sound* SoundSource::GetSound() const
{
	return m_pSound;
}

void SoundSource::SetPosition(const Vec3f &position)
{
	m_position = position;

	alSource3f(m_sourceID, AL_POSITION, m_position.x, m_position.y, m_position.z);
}

const Vec3f &SoundSource::GetPosition() const
{
	return m_position;
}

void SoundSource::SetVelocity(const Vec3f &velocity)
{
	m_velocity = velocity;

	alSource3f(m_sourceID, AL_VELOCITY, m_velocity.x, m_velocity.y, m_velocity.z);
}

const Vec3f &SoundSource::GetVelocity() const
{
	return m_velocity;
}

void SoundSource::SetPitch(float pitch)
{
	m_pitch = pitch;

	alSourcef(m_sourceID, AL_PITCH, m_pitch);
}

float SoundSource::GetPitch() const
{
	return m_pitch;
}

void SoundSource::SetGain(float gain)
{
	m_gain = gain;

	alSourcef(m_sourceID, AL_GAIN, m_gain);
}

float SoundSource::GetGain() const
{
	return m_gain;
}

void SoundSource::SetLooping(bool looping)
{
	m_looping = looping;

	alSourcei(m_sourceID, AL_LOOPING, m_looping);
}

bool SoundSource::GetLooping() const
{
	return m_looping;
}

void SoundSource::SetReferenceDistance(float referenceDistance)
{
	m_referenceDistance = referenceDistance;

	alSourcef(m_sourceID, AL_REFERENCE_DISTANCE, m_referenceDistance);
}

float SoundSource::GetReferenceDistance() const
{
	return m_referenceDistance;
}

void SoundSource::SetMaxDistance(float maxDistance)
{
	m_maxDistance = maxDistance;

	alSourcef(m_sourceID, AL_MAX_DISTANCE, maxDistance);
}

float SoundSource::GetMaxDistance() const
{
	return m_maxDistance;
}

bool SoundSource::Play()
{
	assert(m_pSound != NULL);

	return m_pSound->Play(this);
}

void SoundSource::Stop()
{
	alSourceStop(m_sourceID);
}

void SoundSource::Rewind()
{
	alSourceRewind(m_sourceID);
}

void SoundSource::Pause()
{
	alSourcePause(m_sourceID);
}

unsigned int SoundSource::GetSourceID()
{
	return m_sourceID;
}

void SoundSource::SetRollOffFactor(float rollOffFactor)
{
	m_rollOffFactor = rollOffFactor;

	alSourcef(m_sourceID, AL_ROLLOFF_FACTOR, m_rollOffFactor);
}

float SoundSource::GetRollOffFactor() const
{
	return m_rollOffFactor;
}

bool SoundSource::Playing() const
{
	int state;

	alGetSourcei(m_sourceID, AL_SOURCE_STATE, &state);

	return state == AL_PLAYING;
}