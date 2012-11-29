#pragma once

class Sound
{
public:
	virtual ~Sound() {}
	virtual bool Play(class SoundSource* pSoundSource) = 0;
	virtual void SetUpSoundSource(class SoundSource* pSoundSource) = 0;
};