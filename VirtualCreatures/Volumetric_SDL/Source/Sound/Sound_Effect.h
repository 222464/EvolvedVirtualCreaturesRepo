#pragma once

#include <AssetManager/Asset.h>
#include <Sound/Sound.h>

#include <al.h>
#include <alut.h>

class Sound_Effect :
	public Asset, public Sound
{
private:
	unsigned int m_bufferID;

	// Inherited from Sound
	void SetUpSoundSource(SoundSource* pSoundSource);

public:
	Sound_Effect();
	~Sound_Effect();

	// Inherited from Asset
	bool LoadAsset(const std::string &name);

	unsigned int GetBufferID() const;

	bool Play(class SoundSource* pSoundSource);

	// Asset factory
	static Asset* Asset_Factory();
};

