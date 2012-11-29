#include <SceneObjects/SceneObject_Player.h>

#include <SceneObjects/SceneObject_Door.h>

#include <SceneObjects/Weapons/WeaponFactory.h>

#include <assert.h>

bool RayCallBackTest(OctreeOccupant* pOc, const float t[2])
{
	if(static_cast<SceneObject*>(pOc)->m_unmanagedName == "prop")
		return true;

	return false;
}

SceneObject_Player::SceneObject_Player()
	: m_mouseSensitivity(0.1f), m_xRot(0.0f), m_yRot(0.0f),
	m_acceleration_walk(1.5f), m_acceleration_run(2.5f), m_deceleration_walk(0.2f), m_deceleration_run(0.2f), m_maxSpeed_walk(8.0f), m_maxSpeed_run(18.0f),
	m_onGround(false), m_jumpVel(0.23f), m_mouseLocked(false),
	m_weaponOffset(0.0f, 0.0f, 0.0f), m_handDelayOffset(0.0f, 0.0f, 0.0f),
	m_shootingState(e_notShooting), m_shootingAnimationTimer(0.0f),
	m_pMuzzleFlashLight(NULL), m_handDelayTime(1.0f),
	m_pPhysicsFunc(&SceneObject_Player::RunNoClip), m_physicsMode(false),
	m_shellSpawnPointWorldVelocity(0.0f, 0.0f, 0.0f)
{
	m_collider.m_center.x = 0.0f;
	m_collider.m_center.y = 0.0f;
	m_collider.m_center.z = 0.0f;

	m_collider.m_halfDims.x = 0.4f;
	m_collider.m_halfDims.y = 0.4f;
	m_collider.m_halfDims.z = 0.4f;

	m_aabb.SetHalfDims(Vec3f(0.5f, 1.0f, 0.5f));
}

SceneObject_Player::~SceneObject_Player()
{
}

void SceneObject_Player::OnAdd()
{
	// Get reference to physics world
	m_pPhysicsWorld = static_cast<SceneObject_PhysicsWorld*>(GetScene()->GetNamed_SceneObject("physWrld"));

	assert(m_pPhysicsWorld != NULL);

	m_collider.m_pWorld = static_cast<World*>(GetScene()->GetNamed_SceneObject("world"));

	assert(m_collider.m_pWorld != NULL);

	if(!m_armModel.LoadAsset("data/weapons/arms/arm_L_holdGun_static_HD.obj"))
		abort();

	m_armModel.SetRenderer(GetScene());

	WeaponFactory weaponFactory;

	if(!weaponFactory.Create(&m_weapon, Weapon::e_ballistic, "data/weapons/pack1/descriptor.wepn", GetScene()))
		abort();

	m_gunSoundSource.SetSound(m_weapon.GetShootSound());
	m_gunSoundSource.SetGain(1.0f);
	m_gunSoundSource.SetRollOffFactor(0.5f);

	m_muzzleFlash.m_sprite.Create(m_weapon.GetMuzzleFlashSpriteSheet(), 3.0f, 2, 2, 4);

	m_muzzleFlash.m_pBarrelTipTransform = &m_barrelTipTransform;
	m_muzzleFlash.m_pScene = GetScene();

	m_pEmissiveRender = static_cast<SceneEffect_EmissiveRender*>(GetScene()->GetNamed_Effect("emRen"));

	assert(m_pEmissiveRender != NULL);

	m_pLighting = static_cast<SceneEffect_Lighting*>(GetScene()->GetNamed_Effect("lighting"));

	assert(m_pLighting != NULL);

	// Create flash light
	m_pFlashLight = new Light_Spot_Shadowed(512, 512);

	m_pFlashLight->SetIntensity(18.0f);
	m_pFlashLight->SetSpreadAngle(pif / 9.0f);

	m_pLighting->AddLight(m_pFlashLight);
}

void SceneObject_Player::Logic()
{
	// Control player if mouse is locked
	if(m_mouseLocked)
	{
		// Unlock
		if(GetScene()->m_inputHandler.KeyReleased(SDLK_ESCAPE))
		{
			m_mouseLocked = false;

			InputHandler::ShowMouse(true);
			InputHandler::KeepMouseInWindow(false);
		}
		else
		{
			int dmx = 100 - GetScene()->m_inputHandler.GetMouseX();
			int dmy = 100 - GetScene()->m_inputHandler.GetMouseY();
			InputHandler::SetMousePos(100, 100);

			m_xRot += dmx * m_mouseSensitivity;
			m_yRot += dmy * m_mouseSensitivity;

			// Wrap x
			m_xRot = Wrap(m_xRot, 360.0f);

			// Range checks for y
			if(m_yRot > 90.0f)
				m_yRot = 90.0f;
			else if(m_yRot < -90.0f)
				m_yRot = -90.0f;
		}
	}
	else
	{
		// Lock
		if(GetScene()->m_inputHandler.KeyReleased(SDLK_ESCAPE))
		{
			m_mouseLocked = true;

			InputHandler::ShowMouse(false);
			InputHandler::KeepMouseInWindow(true);

			// Set position now so don't get a view direction jump
			InputHandler::SetMousePos(100, 100);
		}
	}

	(this->*m_pPhysicsFunc)();

	// --------------------------------------------------------------

	if(GetScene()->m_inputHandler.KeyReleased(SDLK_e))
	{
		// Test for doors
		std::vector<OctreeOccupant*> result;

		GetScene()->m_spt.Query_Segment(result, m_collider.m_center, m_collider.m_center - m_facingDirection * 6.0f);

		for(unsigned int i = 0, size = result.size(); i < size; i++)
		{
			SceneObject* pSO = static_cast<SceneObject*>(result[i]);

			if(pSO->m_unmanagedName == "door")
			{
				SceneObject_Door* pDoor = static_cast<SceneObject_Door*>(pSO);

				pDoor->ToggleState();
			}
		}
	}

	if(GetScene()->m_inputHandler.KeyReleased(SDLK_n))
	{
		// Generate new gun
		WeaponFactory weaponFactory;

		// Reset
		m_weapon = Weapon();

		if(!weaponFactory.Create(&m_weapon, Weapon::e_ballistic, "data/weapons/pack1/descriptor.wepn", GetScene()))
			abort();

		m_gunSoundSource.SetSound(m_weapon.GetShootSound());
	}

	if(GetScene()->m_inputHandler.KeyReleased(SDLK_f))
	{
		// Toggle light
		m_pFlashLight->m_enabled = !m_pFlashLight->m_enabled;
	}

	// ----------------------------- Shooting ---------------------------------

	if(m_shootingState == e_notShooting && GetScene()->m_inputHandler.LMBDown())
	{
		m_shootingState = e_recoiling;

		m_weapon.Shoot(m_barrelTipPos, GetScene()->m_camera.m_rotation, m_collider.m_vel);
		m_weapon.EjectShell(m_baseWeaponTransform, GetScene()->m_camera.m_rotation, m_shellSpawnPointWorldVelocity);

		m_gunSoundSource.Play();

		// Next flash
		m_muzzleFlash.m_sprite.IncTime(1.0f);

		// Muzzle flash light
		if(m_pMuzzleFlashLight == NULL)
		{
			m_pMuzzleFlashLight = new Light_Point();
			m_pMuzzleFlashLight->SetCenter(m_barrelTipTransform * Vec3f(0.0f, 0.0f, 0.0f));
			m_pMuzzleFlashLight->SetIntensity(1.6f);
			m_pMuzzleFlashLight->m_color = Color3f(1.0f, 1.0f, 0.0f);
			m_pMuzzleFlashLight->CalculateRange();
		
			m_pLighting->AddLight(m_pMuzzleFlashLight);
		}
	}

	// Animate gun
	switch(m_shootingState)
	{
	case e_recoiling:

		m_weaponOffset = Vec3f(0.0f, 0.0f, m_shootingAnimationTimer * 0.2f);

		m_shootingAnimationTimer += GetScene()->m_frameTimer.GetTimeMultiplier();

		if(m_shootingAnimationTimer > m_weapon.GetRecoilingTime())
		{
			m_shootingState = e_returning;

			// Do not discard time overflow
			m_shootingAnimationTimer = 2.0f * m_weapon.GetRecoilingTime() - m_shootingAnimationTimer;

			// Remove light
			if(m_pMuzzleFlashLight != NULL)
			{
				m_pLighting->RemoveLight(m_pMuzzleFlashLight);
				m_pMuzzleFlashLight = NULL;
			}
		}
		else
		{
			if(m_shootingAnimationTimer < 2.5f)
			{
				// Add muzzle flash for rendering
				m_pEmissiveRender->AddForFrame(&m_muzzleFlash);
			}
			else
			{
				// Remove light
				if(m_pMuzzleFlashLight != NULL)
				{
					m_pLighting->RemoveLight(m_pMuzzleFlashLight);
					m_pMuzzleFlashLight = NULL;
				}
			}
		}

		break;

	case e_returning:

		m_weaponOffset = Vec3f(0.0f, 0.0f, m_shootingAnimationTimer * 0.2f);

		m_shootingAnimationTimer -= GetScene()->m_frameTimer.GetTimeMultiplier();

		if(m_shootingAnimationTimer <= 0.0f)
		{
			m_weaponOffset = Vec3f(0.0f, 0.0f, 0.0f);

			// Do not discard time overflow
			m_shootingAnimationTimer *= -1.0f;

			m_shootingState = e_notShooting;
		}

		break;
	};

	// Update transforms
	Vec3f previousShellEjectPos(m_baseWeaponTransform_noRecoil * m_weapon.GetEjectPos());

	m_playerTransform_noRecoil = Matrix4x4f::TranslateMatrix(m_collider.m_center + m_handDelayOffset) * GetScene()->m_camera.m_rotation.Conjugate().GetMatrix() * Matrix4x4f::ScaleMatrix(Vec3f(0.7f, 0.7f, 0.7f)) * Matrix4x4f::TranslateMatrix(Vec3f(0.4f, -0.5f, -1.7f));
	m_playerTransform = m_playerTransform_noRecoil * Matrix4x4f::TranslateMatrix(m_weaponOffset);
	m_baseWeaponTransform_noRecoil = m_playerTransform_noRecoil * Matrix4x4f::RotateMatrix_Y(-pif_over_2) * Matrix4x4f::ScaleMatrix(Vec3f(0.6f, 0.6f, 0.6f)) * Matrix4x4f::TranslateMatrix(-m_weapon.GetHandPos());
	m_baseWeaponTransform = m_playerTransform * Matrix4x4f::RotateMatrix_Y(-pif_over_2) * Matrix4x4f::ScaleMatrix(Vec3f(0.6f, 0.6f, 0.6f)) * Matrix4x4f::TranslateMatrix(-m_weapon.GetHandPos());
	m_barrelTipTransform = m_baseWeaponTransform * Matrix4x4f::TranslateMatrix(m_weapon.GetBarrelTipPos());

	// Current position - previous to get velocity of can at the shell ejection point
	m_shellSpawnPointWorldVelocity = (m_baseWeaponTransform_noRecoil * m_weapon.GetEjectPos() - previousShellEjectPos) / (GetScene()->m_frameTimer.GetTimeMultiplier() + 0.001f);

	// Remove old times
	int backSize = m_pastFrameBarrelPositions.size() - static_cast<int>(m_handDelayTime / GetScene()->m_frameTimer.GetTimeMultiplier() + 0.5f);

	if(backSize < 0)
		backSize = 0;

	std::list<Vec3f>::iterator end = m_pastFrameBarrelPositions.end();

	for(int i = 0; i < backSize; i++)
		end--;

	m_pastFrameBarrelPositions.erase(end, m_pastFrameBarrelPositions.end());

	m_barrelTipPos = m_barrelTipTransform * Vec3f(0.0f, 0.0f, 0.0f);

	m_pastFrameBarrelPositions.push_front(m_barrelTipPos);

	m_handDelayOffset += ((m_pastFrameBarrelPositions.back() - m_barrelTipPos) / 2.0f - m_handDelayOffset) / 4.0f * GetScene()->m_frameTimer.GetTimeMultiplier();

	float mag = m_handDelayOffset.Magnitude();

	if(mag > 0.25f)
		m_handDelayOffset *= 0.25f / mag;

	m_gunSoundSource.SetPosition(m_baseWeaponTransform * Vec3f(0.0f, 0.0f, 0.0f));
	m_gunSoundSource.SetVelocity(m_collider.m_vel);

	// Update light
	if(m_pFlashLight->m_enabled)
	{
		m_pFlashLight->SetCenter(m_barrelTipPos);
		m_pFlashLight->SetDirection(-m_facingDirection);
		m_pFlashLight->RenderToMap(GetScene());
	}
}

void SceneObject_Player::RunNoClip()
{
	float xRotRads = DegToRad(m_xRot);
	float yRotRads = DegToRad(m_yRot);

	m_facingDirection = RotationToVector(xRotRads, yRotRads);

	// Set up scene camera from rotation
	Quaternion xRotQuad;
	xRotQuad.Rotate(xRotRads, Vec3f(0.0f, 1.0f, 0.0f));
	Quaternion yRotQuad;
	yRotQuad.Rotate(yRotRads, Vec3f(1.0f, 0.0f, 0.0f));

	GetScene()->m_camera.m_rotation = xRotQuad * yRotQuad;

	float noClipSlowDivider = 5.0f;

	if(GetScene()->m_inputHandler.GetCurrentKeyState(SDLK_LSHIFT))
	{
		if(m_collider.m_vel.Magnitude() < m_maxSpeed_run / noClipSlowDivider)
		{
			if(GetScene()->m_inputHandler.GetCurrentKeyState(SDLK_w))
				m_collider.m_vel -= m_facingDirection * m_acceleration_run * GetScene()->m_frameTimer.GetTimeMultiplier() / noClipSlowDivider;
			else if(GetScene()->m_inputHandler.GetCurrentKeyState(SDLK_s))
				m_collider.m_vel += m_facingDirection * m_acceleration_run * GetScene()->m_frameTimer.GetTimeMultiplier() / noClipSlowDivider;

			if(GetScene()->m_inputHandler.GetCurrentKeyState(SDLK_a))
				m_collider.m_vel -= Vec3f(m_acceleration_run * cosf(xRotRads), 0.0f, -m_acceleration_run * sinf(xRotRads)) * GetScene()->m_frameTimer.GetTimeMultiplier() / noClipSlowDivider;
			else if(GetScene()->m_inputHandler.GetCurrentKeyState(SDLK_d))
				m_collider.m_vel += Vec3f(m_acceleration_run * cosf(xRotRads), 0.0f, -m_acceleration_run * sinf(xRotRads)) * GetScene()->m_frameTimer.GetTimeMultiplier() / noClipSlowDivider;

			if(GetScene()->m_inputHandler.GetCurrentKeyState(SDLK_LCTRL))
				m_collider.m_vel -= Vec3f(0.0f, m_acceleration_run, 0.0f) * GetScene()->m_frameTimer.GetTimeMultiplier() / noClipSlowDivider;
			else if(GetScene()->m_inputHandler.GetCurrentKeyState(SDLK_SPACE))
				m_collider.m_vel += Vec3f(0.0f, m_acceleration_run, 0.0f) * GetScene()->m_frameTimer.GetTimeMultiplier() / noClipSlowDivider;
		}

		// Deceleration
		m_collider.m_vel -= m_collider.m_vel * m_deceleration_run * GetScene()->m_frameTimer.GetTimeMultiplier();
	}
	else
	{
		if(m_collider.m_vel.Magnitude() < m_maxSpeed_walk / noClipSlowDivider)
		{
			if(GetScene()->m_inputHandler.GetCurrentKeyState(SDLK_w))
				m_collider.m_vel -= m_facingDirection * m_acceleration_walk * GetScene()->m_frameTimer.GetTimeMultiplier() / noClipSlowDivider;
			else if(GetScene()->m_inputHandler.GetCurrentKeyState(SDLK_s))
				m_collider.m_vel += m_facingDirection * m_acceleration_walk * GetScene()->m_frameTimer.GetTimeMultiplier() / noClipSlowDivider;

			if(GetScene()->m_inputHandler.GetCurrentKeyState(SDLK_a))
				m_collider.m_vel -= Vec3f(m_acceleration_walk * cosf(xRotRads), 0.0f, -m_acceleration_walk * sinf(xRotRads)) * GetScene()->m_frameTimer.GetTimeMultiplier() / noClipSlowDivider;
			else if(GetScene()->m_inputHandler.GetCurrentKeyState(SDLK_d))
				m_collider.m_vel += Vec3f(m_acceleration_walk * cosf(xRotRads), 0.0f, -m_acceleration_walk * sinf(xRotRads)) * GetScene()->m_frameTimer.GetTimeMultiplier() / noClipSlowDivider;

			if(GetScene()->m_inputHandler.GetCurrentKeyState(SDLK_LCTRL))
				m_collider.m_vel -= Vec3f(0.0f, m_acceleration_walk, 0.0f) * GetScene()->m_frameTimer.GetTimeMultiplier() / noClipSlowDivider;
			else if(GetScene()->m_inputHandler.GetCurrentKeyState(SDLK_SPACE))
				m_collider.m_vel += Vec3f(0.0f, m_acceleration_walk, 0.0f) * GetScene()->m_frameTimer.GetTimeMultiplier() / noClipSlowDivider;
		}

		// Deceleration
		m_collider.m_vel -= m_collider.m_vel * m_deceleration_walk * GetScene()->m_frameTimer.GetTimeMultiplier();
	}

	// Get voxel looking at
	Point3i lookPos(m_collider.m_pWorld->HighlightVoxel(m_collider.m_center, -m_facingDirection));

	lookPos.x += static_cast<int>(m_facingDirection.x * 1.6f);
	lookPos.y += static_cast<int>(m_facingDirection.y * 1.6f);
	lookPos.z += static_cast<int>(m_facingDirection.z * 1.6f);

	m_collider.m_center += m_collider.m_vel * GetScene()->m_frameTimer.GetTimeMultiplier();

	//m_onGround = m_collider.Update();

	GetScene()->m_camera.m_position = m_collider.m_center + m_facingDirection * m_shootingAnimationTimer * 0.05f;

	m_aabb.SetCenter(m_collider.m_center);

	if(GetScene()->m_inputHandler.KeyReleased(SDLK_o))
	{
		end.x = static_cast<int>(m_collider.m_center.x);
		end.y = static_cast<int>(m_collider.m_center.y);
		end.z = static_cast<int>(m_collider.m_center.z);

		while(m_collider.m_pWorld->GetVoxel(end.x, end.y, end.z) == 0)
			end.y--;

		end.y++;
	}

	if(GetScene()->m_inputHandler.KeyReleased(SDLK_p))
	{
		Point3i start;
		start.x = static_cast<int>(m_collider.m_center.x);
		start.y = static_cast<int>(m_collider.m_center.y);
		start.z = static_cast<int>(m_collider.m_center.z);

		m_task.m_path.clear();

		m_task.m_start = start;
		m_task.m_end = end;

		m_collider.m_pWorld->StartPathFindingTask(&m_task);
	}
}

void SceneObject_Player::RunPhysics()
{
	float xRotRads = DegToRad(m_xRot);
	float yRotRads = DegToRad(m_yRot);

	m_facingDirection = RotationToVector(xRotRads, yRotRads);

	// Set up scene camera from rotation
	Quaternion xRotQuad;
	xRotQuad.Rotate(xRotRads, Vec3f(0.0f, 1.0f, 0.0f));
	Quaternion yRotQuad;
	yRotQuad.Rotate(yRotRads, Vec3f(1.0f, 0.0f, 0.0f));

	GetScene()->m_camera.m_rotation = xRotQuad * yRotQuad;

	Vec2f xzPlaneForwardDirection(sinf(xRotRads), cosf(xRotRads));
	Vec2f xzPlaneSideDirection(xzPlaneForwardDirection.y, -xzPlaneForwardDirection.x);

	if(GetScene()->m_inputHandler.GetCurrentKeyState(SDLK_LSHIFT))
	{
		m_pCharacterController->m_maxSpeed = m_maxSpeed_run;
		m_pCharacterController->m_deceleration = m_deceleration_run;

		if(GetScene()->m_inputHandler.GetCurrentKeyState(SDLK_w))
			m_pCharacterController->Walk(-xzPlaneForwardDirection * m_acceleration_run * GetScene()->m_frameTimer.GetTimeMultiplier());
		else if(GetScene()->m_inputHandler.GetCurrentKeyState(SDLK_s))
			m_pCharacterController->Walk(xzPlaneForwardDirection * m_acceleration_run * GetScene()->m_frameTimer.GetTimeMultiplier());

		if(GetScene()->m_inputHandler.GetCurrentKeyState(SDLK_a))
			m_pCharacterController->Walk(-xzPlaneSideDirection * m_acceleration_run * GetScene()->m_frameTimer.GetTimeMultiplier());
		else if(GetScene()->m_inputHandler.GetCurrentKeyState(SDLK_d))
			m_pCharacterController->Walk(xzPlaneSideDirection * m_acceleration_run * GetScene()->m_frameTimer.GetTimeMultiplier());
	}
	else
	{
		m_pCharacterController->m_maxSpeed = m_maxSpeed_walk;
		m_pCharacterController->m_deceleration = m_deceleration_walk;

		if(GetScene()->m_inputHandler.GetCurrentKeyState(SDLK_w))
			m_pCharacterController->Walk(-xzPlaneForwardDirection * m_acceleration_walk * GetScene()->m_frameTimer.GetTimeMultiplier());
		else if(GetScene()->m_inputHandler.GetCurrentKeyState(SDLK_s))
			m_pCharacterController->Walk(xzPlaneForwardDirection * m_acceleration_walk * GetScene()->m_frameTimer.GetTimeMultiplier());

		if(GetScene()->m_inputHandler.GetCurrentKeyState(SDLK_a))
			m_pCharacterController->Walk(-xzPlaneSideDirection * m_acceleration_walk * GetScene()->m_frameTimer.GetTimeMultiplier());
		else if(GetScene()->m_inputHandler.GetCurrentKeyState(SDLK_d))
			m_pCharacterController->Walk(xzPlaneSideDirection * m_acceleration_walk * GetScene()->m_frameTimer.GetTimeMultiplier());
	}

	// Jump
	if(GetScene()->m_inputHandler.GetCurrentKeyState(SDLK_SPACE))
		m_pCharacterController->Jump();

	m_pCharacterController->Update();

	GetScene()->m_camera.m_position = m_collider.m_center += (m_pCharacterController->GetPosition() + Vec3f (0.0f, 0.9f, 0.0f) - m_collider.m_center) / 2.0f * GetScene()->m_frameTimer.GetTimeMultiplier();

	m_collider.m_vel = m_pCharacterController->GetVelocity();

	m_aabb.SetCenter(m_collider.m_center);
}

void SceneObject_Player::CreatePhysicsShape()
{
	assert(!m_physicsMode);

	m_pCharacterController.reset(new DynamicCharacterController(GetScene(), m_pPhysicsWorld, m_collider.m_center, 0.5f, 1.2f, 70.0f, 0.5f));
}

void SceneObject_Player::DestroyPhysicsShape()
{
	assert(m_physicsMode);

	m_pCharacterController.reset();
}

void SceneObject_Player::Render()
{
	m_armModel.Render(m_playerTransform);
	m_weapon.Render(m_baseWeaponTransform);

	if(m_task.m_taskDone)
	{
		glBegin(GL_LINES);

		for(unsigned int i = 1; i < m_task.m_path.size(); i++)
		{
			glVertex3f((float)m_task.m_path[i-1].x + 0.5f, (float)m_task.m_path[i-1].y + 0.5f, (float)m_task.m_path[i-1].z + 0.5f);
			glVertex3f((float)m_task.m_path[i].x + 0.5f, (float)m_task.m_path[i].y + 0.5f, (float)m_task.m_path[i].z + 0.5f);
		}

		glEnd();
	}
}

Vec3f SceneObject_Player::GetViewVec()
{
	return m_facingDirection;
}

Vec3f SceneObject_Player::GetPosition()
{
	return m_collider.m_center;
}

void SceneObject_Player::UsePhysics(bool use)
{
	if(use)
	{
		if(!m_physicsMode)
			CreatePhysicsShape();

		m_pPhysicsFunc = &SceneObject_Player::RunPhysics;
	}
	else
	{
		if(m_physicsMode)
			DestroyPhysicsShape();

		m_pPhysicsFunc = &SceneObject_Player::RunNoClip;
	}

	m_physicsMode = use;
}

bool SceneObject_Player::UsingPhysics() const
{
	return m_physicsMode;
}