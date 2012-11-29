#pragma once

#include <AssetManager/Asset.h>

#include <Sound/OpenALUtils.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>
#include <vorbis/vorbisfile.h>

#include <Sound/SoundSource.h>
#include <Sound/Sound.h>

class Sound_Streamed :
	public Sound
{
private:
	// For stream restarts
	std::string m_fileName;

	OggVorbis_File m_oggStream;
	vorbis_info* m_pVorbisInfo;
	vorbis_comment* m_pVorbisComment;

	// Front buffer and back buffer
	unsigned int m_buffers[2];

	int m_format;

	bool m_streamOpened;

	SoundSource* m_pSoundSource;
	class SoundSystem* m_pSoundSystem;

	bool Stream(unsigned int bufferID);

	// Inherited from Sound
	void SetUpSoundSource(SoundSource* pSoundSource);

public:
	static const unsigned int s_bufferSize = 131072;

	bool m_looping;

	Sound_Streamed();
	~Sound_Streamed();

	bool OpenStream(const std::string &name);
	bool StreamOpened() const;

	void SetStreamTime(float time);

	void CloseStream();

	void PrintInfo();

	bool Play();

	// Inherited from Sound
	bool Play(SoundSource* pSoundSource);
	bool Playing() const;

	bool Update();

	void Empty();

	static std::string VorbisErrorString(int errorCode);

	friend class SoundSystem;
};

