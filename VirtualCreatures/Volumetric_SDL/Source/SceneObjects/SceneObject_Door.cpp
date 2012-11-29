#include <SceneObjects/SceneObject_Door.h>

#include <Scene/Scene.h>

#include <Utilities/UtilFuncs.h>

#include <Sound/SoundSystem.h>

#include <assert.h>

const float SceneObject_Door::s_openAndCloseSpeed = 0.018f;

SceneObject_Door::SceneObject_Door()
	: m_created(false), m_action(e_none), m_state(e_closed)
{
	m_unmanagedName = "door";
}

bool SceneObject_Door::Create(const std::string &doorModelName, const Vec3f &pos, float angle, bool openCW)
{
	assert(!m_created);

	Scene* pScene = GetScene();

	assert(pScene != NULL);

	Asset* pDoorModelAsset;
	
	if(!pScene->GetAssetManager_AutoCreate("modelOBJ", Model_OBJ::Asset_Factory)->GetAsset(doorModelName, pDoorModelAsset))
		return false;

	m_pDoorModel = static_cast<Model_OBJ*>(pDoorModelAsset);
	m_pDoorModel->SetRenderer(GetScene());

	// Default texture setting: nearest filtering
	for(unsigned int i = 0, size = m_pDoorModel->GetNumMaterials(); i < size; i++)
	{
		Model_OBJ::Material* pMat = m_pDoorModel->GetMaterial(i);
		
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
	}

	// Sound effects
	SoundSystem* pSoundSystem = static_cast<SoundSystem*>(pScene->GetNamed_Effect("sndsys"));

	assert(pSoundSystem != NULL);

	Asset* pAsset;

	if(!pSoundSystem->m_sound_effect_manager.GetAsset("data/sounds/doorOpen.wav", pAsset))
		abort();

	m_pOpenSoundEffect = static_cast<Sound_Effect*>(pAsset);

	assert(m_pOpenSoundEffect != NULL);

	if(!pSoundSystem->m_sound_effect_manager.GetAsset("data/sounds/doorClose.wav", pAsset))
		abort();

	m_pCloseSoundEffect = static_cast<Sound_Effect*>(pAsset);

	assert(m_pCloseSoundEffect != NULL);

	m_pos = pos;
	m_angle = angle;

	m_openCW = openCW;

	RegenAABB();

	// Set up sound source
	m_doorSource.SetPosition(m_pos);
	m_doorSource.SetVelocity(Vec3f(0.0f, 0.0f, 0.0f));
	m_doorSource.SetGain(1.0f);
	m_doorSource.SetLooping(false);

	m_created = true;

	return true;
}

void SceneObject_Door::RegenAABB()
{
	m_aabb =  m_pDoorModel->GetAABB().GetTransformedAABB(Matrix4x4f::TranslateMatrix(m_pos) * Matrix4x4f::RotateMatrix_Y(m_angle));

	if(IsSPTManaged())
		TreeUpdate();
}

bool SceneObject_Door::Created()
{
	return m_created;
}

void SceneObject_Door::ToggleState()
{
	if(m_state == e_closed)
	{
		m_action = e_opening;

		m_doorSource.SetSound(m_pOpenSoundEffect);

		m_doorSource.Play();
	}
	else if(m_state == e_open)
	{
		m_action = e_closing;

		m_doorSource.SetSound(m_pCloseSoundEffect);

		m_doorSource.Play();
	}
}

void SceneObject_Door::Logic()
{
	switch(m_action)
	{
	case e_opening:
		if(m_openCW)
		{
			float endAngle = m_initAngle - pif_over_2;

			m_angle -= s_openAndCloseSpeed * GetScene()->m_frameTimer.GetTimeMultiplier();
			
			if(m_angle < endAngle)
			{
				m_angle = endAngle;

				m_action = e_none;

				m_state = e_open;
			}

			RegenAABB();
		}
		else
		{
			float endAngle = m_initAngle + pif_over_2;

			m_angle += s_openAndCloseSpeed * GetScene()->m_frameTimer.GetTimeMultiplier();

			if(m_angle > endAngle)
			{
				m_angle = endAngle;

				m_action = e_none;

				m_state = e_open;
			}

			RegenAABB();
		}

		break;

	case e_closing:
		if(m_openCW)
		{
			m_angle += s_openAndCloseSpeed * GetScene()->m_frameTimer.GetTimeMultiplier();

			if(m_angle > m_initAngle)
			{
				m_angle = m_initAngle;

				m_action = e_none;

				m_state = e_closed;
			}

			RegenAABB();
		}
		else
		{
			m_angle -= s_openAndCloseSpeed * GetScene()->m_frameTimer.GetTimeMultiplier();

			if(m_angle < m_initAngle)
			{
				m_angle = m_initAngle;

				m_action = e_none;

				m_state = e_closed;
			}

			RegenAABB();
		}

		break;
	}
}

void SceneObject_Door::Render()
{
	assert(m_created);

	Scene* pScene = GetScene();

	m_pDoorModel->Render(Matrix4x4f::TranslateMatrix(m_pos) * Matrix4x4f::RotateMatrix_Y(m_angle));
}

bool SceneObject_Door::IsOpen()
{
	return m_state == e_open;
}