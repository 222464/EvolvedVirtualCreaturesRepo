#include <SceneObjects/Weapons/SceneObject_Shell.h>

#include <Scene/Scene.h>

const float SceneObject_Shell::s_shellMaxAge = 50.0f;
const float SceneObject_Shell::s_gravityAccel = -9.8f / (60.0f * 60.0f); // 9.8 (meters / (sec * sec)) -> 9.8 / 60 * 60 (meters / (frame * frame))

const float SceneObject_Shell::s_randomVelInfluence = 0.5f;
const float SceneObject_Shell::s_randomRotateSpeedMax = 0.5f;

SceneObject_Shell::SceneObject_Shell(Model_OBJ* pShellModel, const Vec3f &spawnPosition, const Vec3f &spawnVelocity, const Quaternion &spawnRotation)
	: m_pShellModel(pShellModel), m_position(spawnPosition), m_velocity(spawnVelocity), m_spawnRotation(spawnRotation),
	m_rotateSpeed((500 - rand() % 1000) / 500.0f * s_randomRotateSpeedMax), m_rotateAngle(0.0f),
	m_age(0.0f)
{
	// Random velocity changes
	m_velocity += Vec3f((500 - rand() % 1000) / 500.0f, (500 - rand() % 1000) / 500.0f + 2.0f, (500 - rand() % 1000) / 500.0f) * 0.01f;

	// Random rotation axis
	m_rotationAxis = Vec3f((500 - rand() % 1000) / 500.0f, (500 - rand() % 1000) / 500.0f, (500 - rand() % 1000) / 500.0f);

	// Prevent / 0
	if(m_rotationAxis == Vec3f(0.0f, 0.0f, 0.0f))
		m_rotationAxis = Vec3f(1.0f, 0.0f ,0.0f);
	else
		m_rotationAxis.NormalizeThis();
}

void SceneObject_Shell::Logic()
{
	float timeMultiplier = GetScene()->m_frameTimer.GetTimeMultiplier();

	m_velocity.y += s_gravityAccel * timeMultiplier;
	m_position += m_velocity * timeMultiplier;

	m_rotateAngle += m_rotateSpeed * timeMultiplier;

	if(m_age > s_shellMaxAge)
		Destroy();
	else
		m_age += timeMultiplier;
}

void SceneObject_Shell::Render()
{
	m_pShellModel->Render(Matrix4x4f::TranslateMatrix(m_position) * (m_spawnRotation * Quaternion(m_rotationAxis, m_rotateAngle)).GetMatrix());
}