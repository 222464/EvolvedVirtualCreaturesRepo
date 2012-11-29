#pragma once

#include <SceneObjects/Physics/DynamicCharacterController.h>

#include <SceneObjects/Enemies/SceneObject_Enemy.h>

#include <Renderer/Model_MD5/Model_MD5.h>
#include <Renderer/Animation_MD5/Animation_MD5.h>

class SceneObject_Enemy_Flea :
	public SceneObject_Enemy
{
private:
	Model_MD5* m_pFleaModel;
	Animation_MD5* m_pFleaAnimation;

	DynamicCharacterController* m_pCharacterController;

public:
	SceneObject_Enemy_Flea(const Vec3f &position);
	~SceneObject_Enemy_Flea();

	// Inherited from SceneObject
	void OnAdd();
	void Logic();
	void Render();

	// Inherited from SceneObject_Enemy
	std::string GetTypeName();
};

