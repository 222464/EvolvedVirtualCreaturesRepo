#include <SceneObjects/SceneObject_SkyBox.h>

#include <Scene/Scene.h>

#include <sstream>

SceneObject_SkyBox::SceneObject_SkyBox()
	: m_radius(1000.0f),
	m_brightness(0.4f)
{
}

bool SceneObject_SkyBox::Create(const std::string &textureRootName, const std::string &extension)
{
	std::ostringstream os;

	os << textureRootName << "_front" << extension;

	if(!m_texFront.LoadAsset(os.str()))
		return false;

	os.str("");
	os << textureRootName << "_back" << extension;

	if(!m_texBack.LoadAsset(os.str()))
		return false;

	os.str("");
	os << textureRootName << "_left" << extension;

	if(!m_texLeft.LoadAsset(os.str()))
		return false;

	os.str("");
	os << textureRootName << "_right" << extension;

	if(!m_texRight.LoadAsset(os.str()))
		return false;

	os.str("");
	os << textureRootName << "_top" << extension;

	if(!m_texTop.LoadAsset(os.str()))
		return false;

	// Texture settings
	m_texFront.Bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	m_texBack.Bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	m_texLeft.Bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	m_texRight.Bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	m_texTop.Bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

void SceneObject_SkyBox::Render()
{
	// Draw skybox
	GetScene()->SetWorldMatrix(Matrix4x4f::TranslateMatrix(GetScene()->m_camera.m_position) * Matrix4x4f::ScaleMatrix(Vec3f(m_radius, m_radius, m_radius)));

	GetScene()->SetEmissiveColor(Color3f(m_brightness, m_brightness, m_brightness));

	m_texFront.Bind();

	glBegin(GL_QUADS);

	// Front
	glTexCoord2i(0, 0); glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2i(1, 0); glVertex3f(1.0f, -1.0f, -1.0f);
	glTexCoord2i(1, 1); glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2i(0, 1); glVertex3f(-1.0f, 1.0f, -1.0f);

	glEnd();

	m_texBack.Bind();

	glBegin(GL_QUADS);

	// Back
	glTexCoord2i(0, 0); glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2i(1, 0); glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2i(1, 1); glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2i(0, 1); glVertex3f(1.0f, 1.0f, 1.0f);

	glEnd();

	m_texLeft.Bind();

	glBegin(GL_QUADS);

	// Left
	glTexCoord2i(0, 0); glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2i(1, 0); glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2i(1, 1); glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2i(0, 1); glVertex3f(-1.0f, 1.0f, 1.0f);

	glEnd();

	m_texRight.Bind();

	glBegin(GL_QUADS);

	// Right
	glTexCoord2i(0, 0); glVertex3f(1.0f, -1.0f, -1.0f);
	glTexCoord2i(1, 0); glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2i(1, 1); glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2i(0, 1); glVertex3f(1.0f, 1.0f, -1.0f);

	glEnd();

	m_texTop.Bind();

	glBegin(GL_QUADS);

	// Top
	glTexCoord2i(0, 0); glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2i(1, 0); glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2i(1, 1); glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2i(0, 1); glVertex3f(-1.0f, 1.0f, 1.0f);

	glEnd();

	GetScene()->SetEmissiveColor(Color3f(0.0f, 0.0f, 0.0f));
	GetScene()->SetWorldMatrix(Matrix4x4f::IdentityMatrix());
}