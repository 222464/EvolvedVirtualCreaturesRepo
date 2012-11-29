#include <Sound/Sound_Effect.h>

#include <Sound/SoundSource.h>

#include <iostream>

Sound_Effect::Sound_Effect()
	: m_bufferID(0)
{
}

Sound_Effect::~Sound_Effect()
{
	if(m_bufferID != 0)
		alDeleteBuffers(1, &m_bufferID);
}

void Sound_Effect::SetUpSoundSource(SoundSource* pSoundSource)
{
	alSourcei(pSoundSource->GetSourceID(), AL_BUFFER, m_bufferID);
}

bool Sound_Effect::LoadAsset(const std::string &name)
{
	m_bufferID = alutCreateBufferFromFile(name.c_str());
	
	unsigned int errorCode = alGetError();

	if(errorCode != AL_NO_ERROR)
	{
		std::cerr << "Could not load sound chunk \"" << name << "\": " << alutGetErrorString(errorCode) << std::endl;

		return false;
	}

	if(m_bufferID == 0)
	{
		std::cerr << "Could not load sound chunk \"" << name << "\": Unkown reason!" << std::endl;

		return false;
	}

	return true;
}

bool Sound_Effect::Play(class SoundSource* pSoundSource)
{
	alSourcePlay(pSoundSource->GetSourceID());

	return true;
}

unsigned int Sound_Effect::GetBufferID() const
{
	return m_bufferID;
}

Asset* Sound_Effect::Asset_Factory()
{
	return new Sound_Effect();
}