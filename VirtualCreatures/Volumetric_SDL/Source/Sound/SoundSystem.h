#pragma once

#include <Scene/SceneEffect.h>

#include <AssetManager/AssetManager.h>

#include <Constructs/Vec3f.h>

#include <unordered_set>

#include <Sound/OpenALUtils.h>
#include <Sound/Sound.h>
#include <Sound/Sound_Effect.h>
#include <Sound/Sound_Streamed.h>

class SoundSystem :
	public SceneEffect
{
private:
	// Listener description
	void UpdateListener();

	Vec3f m_prevCameraPos;

	std::unordered_set<Sound_Streamed*> m_pStreamedSounds;

	void UpdateStreamedSounds();

	bool m_created;

public:
	AssetManager m_sound_effect_manager;

	SoundSystem();
	~SoundSystem();

	bool Create();

	void Add(Sound_Streamed* pStreamedSound);
	void Remove(Sound_Streamed* pStreamedSound);

	// Inherited from SceneEffect
	void RunEffect(); // Updates listener, Streams sound
};

