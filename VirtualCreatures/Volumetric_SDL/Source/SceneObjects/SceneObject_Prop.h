#pragma once

#include <Renderer/Model_OBJ.h>
#include <Scene/SceneObject.h>

#include <Constructs/Quaternion.h>

class SceneObject_Prop :
	public SceneObject
{
private:
	bool m_created;

	Vec3f m_position;
	Quaternion m_rotation;
	Vec3f m_scale;

	Model_OBJ* m_pModel;

	void RegenAABB();

public:
	SceneObject_Prop();

	bool Create(const std::string &modelName);

	void SetPosition(const Vec3f &position);
	void IncPosition(const Vec3f &increment);
	void SetRotation(const Quaternion &quat);
	void SetRotation(const Vec3f &eulerAngles);
	void SetScale(const Vec3f &scale);

	const Vec3f &GetPosition();
	const Quaternion &GetRotation();
	const Vec3f GetScale();

	bool Created();

	// Inherited from SceneObject
	void Logic();
	void Render();
};

