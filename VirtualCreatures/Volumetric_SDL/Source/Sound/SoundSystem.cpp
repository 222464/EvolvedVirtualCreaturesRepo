#include <Sound/SoundSystem.h>

#include <Scene/Scene.h>

#include <iostream>

#include <assert.h>

SoundSystem::SoundSystem()
	: m_created(false),
	m_sound_effect_manager(Sound_Effect::Asset_Factory)
{
}

SoundSystem::~SoundSystem()
{
	for(std::unordered_set<Sound_Streamed*>::iterator it = m_pStreamedSounds.begin(); it != m_pStreamedSounds.end(); it++)
		(*it)->m_pSoundSystem = NULL;
	
	if(m_created)
		alutExit();
}

bool SoundSystem::Create()
{
	assert(!m_created);
	assert(GetScene() != NULL);

	alutInit(0, NULL);

	unsigned int errorCode = alGetError();

	if(errorCode != AL_NO_ERROR)
	{
		std::cerr << "Failed to initialize OpenAL: ";

		std::cerr << alutGetErrorString(errorCode) << std::endl;

		return false;
	}

	m_created = true;

	m_prevCameraPos = GetScene()->m_camera.m_position;
	UpdateListener();

	return true;
}

void SoundSystem::Add(Sound_Streamed* pStreamedSound)
{
	pStreamedSound->m_pSoundSystem = this;

	m_pStreamedSounds.insert(pStreamedSound);
}

void SoundSystem::Remove(Sound_Streamed* pStreamedSound)
{
	pStreamedSound->m_pSoundSystem = NULL;

	m_pStreamedSounds.erase(pStreamedSound);
}

void SoundSystem::UpdateListener()
{
	assert(m_created);
	
	const Camera &camera = GetScene()->m_camera;

	float positionv[] = {camera.m_position.x, camera.m_position.y, camera.m_position.z};

	Vec3f velocity((camera.m_position - m_prevCameraPos) * static_cast<float>(GetScene()->m_frameTimer.GetElapsedTime()));

	// Get forward vector of camera
	Vec3f forward(camera.m_rotation * Vec3f(0.0f, 0.0f, -1.0f));

	// Get up vector of camera
	Vec3f up(camera.m_rotation * Vec3f(0.0f, 1.0f, 0.0f));

	float orientationv[] = {forward.x, forward.y, forward.z, up.x, up.y, up.z};

	alListener3f(AL_POSITION, camera.m_position.x, camera.m_position.y, camera.m_position.z);
    alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
    alListenerfv(AL_ORIENTATION, orientationv);

	alDistanceModel(AL_INVERSE_DISTANCE);

	m_prevCameraPos = camera.m_position;
}

void SoundSystem::RunEffect()
{
	UpdateListener();
	UpdateStreamedSounds();
}

void SoundSystem::UpdateStreamedSounds()
{
	// Go through all streamed sounds
	for(std::unordered_set<Sound_Streamed*>::iterator it = m_pStreamedSounds.begin(); it != m_pStreamedSounds.end();)
	{
		Sound_Streamed* pStreamedSound = (*it);

		if(pStreamedSound->Update())
		{
			if(!pStreamedSound->Playing())
				pStreamedSound->Play();

			 it++;
		}
		else
		{
			pStreamedSound->Empty();
			pStreamedSound->CloseStream();
			pStreamedSound->m_pSoundSource->Stop();
			it = m_pStreamedSounds.erase(it);
		}
	}
}