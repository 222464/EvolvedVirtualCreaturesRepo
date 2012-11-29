#include <Sound/Sound_Streamed.h>

#include <Sound/SoundSystem.h>

#include <iostream>

#include <assert.h>

Sound_Streamed::Sound_Streamed()
	: m_streamOpened(false), m_pSoundSource(NULL), m_pSoundSystem(NULL),
	m_looping(false)
{
}

Sound_Streamed::~Sound_Streamed()
{
	if(m_streamOpened)
		CloseStream();

	if(m_pSoundSystem != NULL)
		m_pSoundSystem->Remove(this);
}

void Sound_Streamed::SetUpSoundSource(SoundSource* pSoundSource)
{
	m_pSoundSource = pSoundSource;
}

bool Sound_Streamed::OpenStream(const std::string &name)
{
	m_fileName = name;

	// Open file for streaming
	int result;

	if((result = ov_fopen(name.c_str(), &m_oggStream)) < 0)
	{
		std::cerr << "ERROR: Could not open stream from file \"" << name << "\"! Reason: " << VorbisErrorString(result) << std::endl;

		return false;
	}

	// Information about the sound
	m_pVorbisInfo = ov_info(&m_oggStream, -1);
	m_pVorbisComment = ov_comment(&m_oggStream, -1);

	if(m_pVorbisInfo->channels == 1)
		m_format = AL_FORMAT_MONO16;
	else
		m_format = AL_FORMAT_STEREO16;

	// Create front and back buffers
	alGenBuffers(2, m_buffers);

	m_streamOpened = true;

	AL_ERROR_CHECK();

	return true;
}

bool Sound_Streamed::StreamOpened() const
{
	return m_streamOpened;
}

void Sound_Streamed::SetStreamTime(float time)
{
	ov_time_seek(&m_oggStream, time);
}

void Sound_Streamed::CloseStream()
{
	assert(m_streamOpened);

	if(m_pSoundSource != NULL)
	{
		m_pSoundSource->Stop();
		m_pSoundSource->SetSound(NULL);
	}

	alDeleteBuffers(2, m_buffers);

	ov_clear(&m_oggStream);
}

void Sound_Streamed::PrintInfo()
{
	std::cout << "------------ VORBIS INFO -------------" << std::endl;
	std::cout << "Version: " << m_pVorbisInfo->version << std::endl;
	std::cout << "Channels: " << m_pVorbisInfo->channels << std::endl;
	std::cout << "Rate (hertz): " << m_pVorbisInfo->rate << std::endl;
	std::cout << "Upper bit rate: " << m_pVorbisInfo->bitrate_upper << std::endl;
	std::cout << "Nominal bit rate: " << m_pVorbisInfo->bitrate_nominal << std::endl;
	std::cout << "Lower bit rate: " << m_pVorbisInfo->bitrate_lower << std::endl;
	std::cout << "Window bit rate: " << m_pVorbisInfo->bitrate_window << std::endl;
	std::cout << "------------- COMMENTS ---------------" << std::endl;
	std::cout << "Vendor: " << m_pVorbisComment->vendor << std::endl;
	std::cout << "User comments:" << std::endl;

	for(int i = 0; i < m_pVorbisComment->comments; i++)
		std::cout << "	" << m_pVorbisComment->user_comments[i] << std::endl;
}

bool Sound_Streamed::Play()
{
	assert(m_pSoundSource != NULL);

	if(m_pSoundSource->Playing())
		return true;

	if(!Stream(m_buffers[0])) // Front buffer
		return false;

	if(!Stream(m_buffers[1])) // Back buffer
		return false;

	alSourceQueueBuffers(m_pSoundSource->GetSourceID(), 2, m_buffers);

	alSourcePlay(m_pSoundSource->GetSourceID());

	AL_ERROR_CHECK();

	return true;
}

bool Sound_Streamed::Play(SoundSource* pSoundSource)
{
	assert(pSoundSource == m_pSoundSource);

	return Play();
}

bool Sound_Streamed::Playing() const
{
	assert(m_pSoundSource != NULL);

	return m_pSoundSource->Playing();
}

bool Sound_Streamed::Update()
{
	assert(m_pSoundSource != NULL);

	int numBuffersProcessed;
	bool active = true;

	alGetSourcei(m_pSoundSource->GetSourceID(), AL_BUFFERS_PROCESSED, &numBuffersProcessed);

	for(; numBuffersProcessed != 0; numBuffersProcessed--)
	{
		unsigned int bufferID;

		alSourceUnqueueBuffers(m_pSoundSource->GetSourceID(), 1, &bufferID);

		active = Stream(bufferID);

		alSourceQueueBuffers(m_pSoundSource->GetSourceID(), 1, &bufferID);

		AL_ERROR_CHECK();
	}

	return active;
}

bool Sound_Streamed::Stream(unsigned int bufferID)
{
	char data[s_bufferSize];
	int size = 0;
	int section;
	int result;

	while(size < s_bufferSize)
	{
		result = ov_read(&m_oggStream, &data[size], s_bufferSize - size, 0, 2, 1, &section);

		if(result > 0)
			size += result;
		else if(result == 0)
			break;
		else
		{
			std::cerr << VorbisErrorString(result) << std::endl;

			return false;
		}
	}
	
	if(size == 0) // Empty
	{
		// If set to loop, restart
		if(m_looping)
		{
			SetStreamTime(0.0f);

			return true;
		}
		else // Otherwise, end the stream
			return false;
	}
	
	// Copy data into OpenAL buffer
	alBufferData(bufferID, m_format, data, size, m_pVorbisInfo->rate);

	AL_ERROR_CHECK();

	return true;
}

void Sound_Streamed::Empty()
{
	assert(m_pSoundSource != NULL);

	int numQueued;

	alGetSourcei(m_pSoundSource->GetSourceID(), AL_BUFFERS_QUEUED, &numQueued);
    
	for(; numQueued != 0; numQueued--)
	{
		unsigned int bufferID;
    
		alSourceUnqueueBuffers(m_pSoundSource->GetSourceID(), 1, &bufferID);
	}

	AL_ERROR_CHECK();
}

std::string Sound_Streamed::VorbisErrorString(int errorCode)
{
	switch(errorCode)
	{
	case OV_EREAD:
		return "Read from media";

	case OV_ENOTVORBIS:
		return "Not Vorbis data";

	case OV_EVERSION:
		return "Vorbis version mismatch";

	case OV_EBADHEADER:
		return "Invalid Vorbis header";

	case OV_EFAULT:
		return "Internal logic fault (bug or heap/stack corruption)";

	default:
		return "Unknown Ogg error";
	}
}