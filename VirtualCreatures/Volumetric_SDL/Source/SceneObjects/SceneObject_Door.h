#pragma once

#include <Renderer/Model_OBJ.h>
#include <Scene/SceneObject.h>

#include <Sound/SoundSource.h>
#include <Sound/Sound_Effect.h>

class SceneObject_Door :
	public SceneObject
{
private:
	Model_OBJ* m_pDoorModel;

	bool m_created;

	Vec3f m_pos;

	float m_initAngle;
	float m_angle;

	bool m_openCW;

	bool m_isOpen;

	enum Action
	{
		e_opening, e_closing, e_none
	} m_action;

	enum State
	{
		e_open, e_closed
	} m_state;

	void RegenAABB();

	SoundSource m_doorSource;

	Sound_Effect* m_pOpenSoundEffect;
	Sound_Effect* m_pCloseSoundEffect;

public:
	static const float s_openAndCloseSpeed;

	SceneObject_Door();

	bool Create(const std::string &doorModelName, const Vec3f &pos, float angle, bool openCW);

	bool Created();

	void ToggleState();

	// Inherited from SceneObject
	void Logic();
	void Render();

	bool IsOpen();
};

