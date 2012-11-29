#include <SceneObjects/SceneObject_Sound.h>

#include <Sound/SoundSystem.h>

#include <Scene/Scene.h>

#include <assert.h>

SceneObject_Sound::SceneObject_Sound()
	: m_autoDestroy(false)
{
}

bool SceneObject_Sound::Create(const std::string &fileName)
{
	assert(GetScene() != NULL);

	// Get reference to sound system
	SoundSystem* pSystem = static_cast<SoundSystem*>(GetScene()->GetNamed_Effect("sndsys"));

	assert(pSystem != NULL);

	Asset* pAsset;

	if(!pSystem->m_sound_effect_manager.GetAsset(fileName, pAsset))
		return false;

	m_soundSource.SetSound(static_cast<Sound_Effect*>(pAsset));

	return true;
}

bool SceneObject_Sound::Created()
{
	return m_soundSource.GetSound() != NULL;
}

void SceneObject_Sound::Logic()
{
	if(m_autoDestroy)
	{
		if(!m_soundSource.Playing())
			Destroy();
	}
}