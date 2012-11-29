#pragma once

#include <Scene/SceneObject.h>

#include <Sound/SoundSource.h>

class SceneObject_Sound :
	public SceneObject
{
public:
	bool m_autoDestroy;

	SoundSource m_soundSource;

	SceneObject_Sound();

	bool Create(const std::string &fileName);

	bool Created();

	// Inherited from SceneObject
	void Logic();
};

