#pragma once

#include <Renderer/Model_OBJ.h>
#include <SceneEffects/Transparency/TransparentRenderable.h>

#include <Constructs/Quaternion.h>

class SceneObject_Prop_Transparent :
	public TransparentRenderable
{
private:
	bool m_created;

	Vec3f m_position;
	Quaternion m_rotation;
	Vec3f m_scale;

	Model_OBJ* m_pModel;

	bool m_solid;

	void RegenAABB();

public:
	SceneObject_Prop_Transparent();

	bool Create(const std::string &modelName, bool solid);

	void SetPosition(const Vec3f &position);
	void IncPosition(const Vec3f &increment);
	void SetRotation(const Quaternion &quat);
	void SetRotation(const Vec3f &eulerAngles);
	void SetScale(const Vec3f &scale);

	const Vec3f &GetPosition();
	const Quaternion &GetRotation();
	const Vec3f GetScale();

	bool Created();

	// Inherited from SceneObject/TransparentRenderable
	void Logic();

	// Inherited from TransparentRenderable
	void Render_Transparent();
};

