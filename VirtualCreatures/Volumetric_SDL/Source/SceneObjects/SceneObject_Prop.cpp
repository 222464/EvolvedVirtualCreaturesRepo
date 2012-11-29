#include <SceneObjects/SceneObject_Prop.h>

#include <Scene/Scene.h>

#include <assert.h>

SceneObject_Prop::SceneObject_Prop()
	: m_created(false),
	m_position(0.0f, 0.0f, 0.0f),
	m_rotation(1.0f, 0.0f, 0.0f, 0.0f),
	m_scale(1.0f, 1.0f, 1.0f)
{
	m_unmanagedName = "prop";
}

bool SceneObject_Prop::Create(const std::string &modelName)
{
	assert(!m_created);
	assert(GetScene() != NULL);

	Asset* pModelAsset;

	if(!GetScene()->GetAssetManager_AutoCreate("modelOBJ", Model_OBJ::Asset_Factory)->GetAsset(modelName, pModelAsset))
		return false;

	m_pModel = static_cast<Model_OBJ*>(pModelAsset);

	m_pModel->SetRenderer(GetScene());

	m_aabb = m_pModel->GetAABB();

	if(IsSPTManaged())
		TreeUpdate();

	m_created = true;

	return true;
}

void SceneObject_Prop::RegenAABB()
{
	Matrix4x4f trans(Matrix4x4f::TranslateMatrix(m_position));

	trans *= m_rotation.GetMatrix();

	if(m_scale.x != 1.0f || m_scale.y != 1.0f || m_scale.z != 1.0f)
		trans *= Matrix4x4f::ScaleMatrix(m_scale);

	m_aabb = m_pModel->GetAABB().GetTransformedAABB(trans);

	if(IsSPTManaged())
		TreeUpdate();
}

void SceneObject_Prop::SetPosition(const Vec3f &position)
{
	assert(m_created);

	m_position = position;

	RegenAABB();
}

void SceneObject_Prop::IncPosition(const Vec3f &increment)
{
	assert(m_created);

	m_position += increment;

	RegenAABB();
}

void SceneObject_Prop::SetRotation(const Quaternion &quat)
{
	assert(m_created);

	m_rotation = quat;

	RegenAABB();
}

void SceneObject_Prop::SetRotation(const Vec3f &eulerAngles)
{
	assert(m_created);

	m_rotation.SetFromEulerAngles(eulerAngles);

	RegenAABB();
}

void SceneObject_Prop::SetScale(const Vec3f &scale)
{
	m_scale = scale;

	RegenAABB();
}

const Vec3f &SceneObject_Prop::GetPosition()
{
	return m_position;
}

const Quaternion &SceneObject_Prop::GetRotation()
{
	return m_rotation;
}

bool SceneObject_Prop::Created()
{
	return m_created;
}

void SceneObject_Prop::Logic()
{
	RegenAABB();
}

void SceneObject_Prop::Render()
{
	assert(m_created);

	Matrix4x4f trans(Matrix4x4f::TranslateMatrix(m_position));

	trans *= m_rotation.GetMatrix();

	if(m_scale.x != 1.0f || m_scale.y != 1.0f || m_scale.z != 1.0f)
		trans *= Matrix4x4f::ScaleMatrix(m_scale);

	m_pModel->Render(trans);
}