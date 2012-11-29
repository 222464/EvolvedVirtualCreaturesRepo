#include <SceneObjects/SceneObject_Prop_Transparent.h>

#include <Scene/Scene.h>

#include <assert.h>

SceneObject_Prop_Transparent::SceneObject_Prop_Transparent()
	: m_created(false),
	m_position(0.0f, 0.0f, 0.0f),
	m_rotation(1.0f, 0.0f, 0.0f, 0.0f),
	m_scale(1.0f, 1.0f, 1.0f)
{
	m_unmanagedName = "prop_trans";
}

bool SceneObject_Prop_Transparent::Create(const std::string &modelName, bool solid)
{
	assert(!m_created);
	assert(GetScene() != NULL);

	Asset* pModelAsset;

	if(!GetScene()->GetAssetManager_AutoCreate("modelOBJ", Model_OBJ::Asset_Factory)->GetAsset(modelName, pModelAsset))
		return false;

	m_pModel = static_cast<Model_OBJ*>(pModelAsset);

	m_pModel->SetRenderer(GetScene());

	m_aabb = m_pModel->GetAABB();

	m_solid = solid;

	if(IsSPTManaged())
		TreeUpdate();

	// Default texture setting: nearest filtering
	/*for(unsigned int i = 0, size = m_pModel->GetNumMaterials(); i < size; i++)
	{
		Model_OBJ::Material* pMat = m_pModel->GetMaterial(i);
		
		pMat->m_pDiffuseMap->Bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		if(pMat->m_pSpecularMap != NULL)
		{
			pMat->m_pSpecularMap->Bind();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}

		if(pMat->m_pNormalMap != NULL)
		{
			pMat->m_pNormalMap->Bind();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
	}*/

	m_created = true;

	return true;
}

void SceneObject_Prop_Transparent::RegenAABB()
{
	Matrix4x4f trans(Matrix4x4f::TranslateMatrix(m_position));

	trans *= m_rotation.GetMatrix();

	if(m_scale.x != 1.0f || m_scale.y != 1.0f || m_scale.z != 1.0f)
		trans *= Matrix4x4f::ScaleMatrix(m_scale);

	m_aabb = m_pModel->GetAABB().GetTransformedAABB(trans);

	if(IsSPTManaged())
		TreeUpdate();
}

void SceneObject_Prop_Transparent::SetPosition(const Vec3f &position)
{
	assert(m_created);

	m_position = position;

	RegenAABB();
}

void SceneObject_Prop_Transparent::IncPosition(const Vec3f &increment)
{
	assert(m_created);

	m_position += increment;

	RegenAABB();
}

void SceneObject_Prop_Transparent::SetRotation(const Quaternion &quat)
{
	assert(m_created);

	m_rotation = quat;

	RegenAABB();
}

void SceneObject_Prop_Transparent::SetRotation(const Vec3f &eulerAngles)
{
	assert(m_created);

	m_rotation.SetFromEulerAngles(eulerAngles);

	RegenAABB();
}

void SceneObject_Prop_Transparent::SetScale(const Vec3f &scale)
{
	m_scale = scale;

	RegenAABB();
}

const Vec3f &SceneObject_Prop_Transparent::GetPosition()
{
	return m_position;
}

const Quaternion &SceneObject_Prop_Transparent::GetRotation()
{
	return m_rotation;
}

bool SceneObject_Prop_Transparent::Created()
{
	return m_created;
}

void SceneObject_Prop_Transparent::Logic()
{
}

void SceneObject_Prop_Transparent::Render_Transparent()
{
	assert(m_created);

	glDisable(GL_CULL_FACE);

	Matrix4x4f trans(Matrix4x4f::TranslateMatrix(m_position));

	trans *= m_rotation.GetMatrix();

	if(m_scale.x != 1.0f || m_scale.y != 1.0f || m_scale.z != 1.0f)
		trans *= Matrix4x4f::ScaleMatrix(m_scale);

	GetScene()->SetWorldMatrix(trans);

	m_pModel->Render(trans);

	glEnable(GL_CULL_FACE);

	GetScene()->SetWorldMatrix(Matrix4x4f::IdentityMatrix());
}