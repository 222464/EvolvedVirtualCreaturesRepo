#include <SceneObjects/Enemies/SceneObject_Enemy_Spawner.h>

#include <Utilities/UtilFuncs.h>

SceneObject_Enemy_Spawner::SceneObject_Enemy_Spawner(const Vec3f &position)
	: m_position(position), m_animationTime(0.0f)
{
}

SceneObject_Enemy_Spawner::~SceneObject_Enemy_Spawner()
{
}

void SceneObject_Enemy_Spawner::OnAdd()
{
	Asset* pAsset;

	if(!GetScene()->GetAssetManager_AutoCreate("md5model", Model_MD5::Asset_Factory)->GetAsset("data/models/enemies/spawner.md5mesh", pAsset))
		abort();

	m_pModel = static_cast<Model_MD5*>(pAsset);

	if(!GetScene()->GetAssetManager_AutoCreate("md5anim", Animation_MD5::Asset_Factory)->GetAsset("data/models/enemies/spawner.md5anim", pAsset))
		abort();

	m_pIdleAnimation = static_cast<Animation_MD5*>(pAsset);

	m_pModel->m_pAnimation = m_pIdleAnimation;

	m_pModel->CheckAnimation();

	m_pModel->SetRenderer(GetScene());
}

void SceneObject_Enemy_Spawner::Logic()
{
	m_animationTime = Wrap(m_animationTime + GetScene()->m_frameTimer.GetTimeMultiplier() / 50.0f, m_pIdleAnimation->GetAnimationDuration());
	m_pModel->Update(m_animationTime);

	m_aabb = m_pModel->GetAABB();

	m_aabb.IncCenter(m_position);

	if(IsSPTManaged())
		TreeUpdate();
}

void SceneObject_Enemy_Spawner::Render()
{
	m_pModel->Render(m_animationTime, Matrix4x4f::TranslateMatrix(m_position));
}

std::string SceneObject_Enemy_Spawner::GetTypeName()
{
	return "spawner";
}