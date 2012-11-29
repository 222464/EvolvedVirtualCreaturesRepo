#pragma once

#include <AssetManager/Asset.h>

#include <Renderer/Shader/TextureAndAttachment.h>
#include <Renderer/SDL_OpenGL.h>

#include <Constructs/Matrix4x4f.h>
#include <Constructs/Vec2f.h>
#include <Constructs/Color3f.h>
#include <Constructs/Color4f.h>

#include <string>
#include <unordered_map>

// Can be used as an asset in the asset manager
class Shader :
	public Asset
{
private:
	unsigned int m_progID, m_geomID, m_vertID, m_fragID;

	std::unordered_map<std::string, TextureAndAttachment> m_textures;

	// Cache attribute locations for speed
	std::unordered_map<std::string, int> m_attributeLocations;

	unsigned int m_lastAttachment;

	bool m_created;

	inline void CheckProgram()
	{
#ifdef DEBUG
		int program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);

		if(program != m_progID)
		{
			std::cerr << "Attempted to set shader parameter before binding!" << std::endl;
			abort();
		}
#endif
	}

public:
	// Returns true if loaded
	static bool LoadShader(const std::string &name, unsigned int id);
	static bool LoadGeometryShader(const std::string &name, unsigned int &id);
	static bool LoadVertexShader(const std::string &name, unsigned int &id);
	static bool LoadFragmentShader(const std::string &name, unsigned int &id);

	// Returns true if there were compilation errors
	static bool CheckLog(unsigned int id);

	static bool Link(unsigned int id);

	static bool Validate(unsigned int id);

	Shader();
	~Shader();

	// Name contains both shader file names, in format: "geomName vertName fragName". "NONE" can be specified to indicate no shader
	// Inherited from Asset
	bool LoadAsset(const std::string &name);

	void SetShaderTexture(const std::string &name, unsigned int textureID, unsigned int target);
	void SetShaderTexture(const std::string &name, unsigned int textureAttachment, unsigned int textureID, unsigned int target);

	int GetAttributeLocation(const std::string &name);

	// ---------------------------- Using Parameter Name ----------------------------

	int SetUniformf(const std::string &name, float param);
	int SetUniformv2f(const std::string &name, float param1, float param2);
	int SetUniformv2f(const std::string &name, const Vec2f &param);
	int SetUniformv3f(const std::string &name, float param1, float param2, float param3);
	int SetUniformv3f(const std::string &name, const Vec3f &param);
	int SetUniformv3f(const std::string &name, const Color3f &param);
	int SetUniformv4f(const std::string &name, float param1, float param2, float param3, float param4);
	int SetUniformv4f(const std::string &name, const Vec4f &param);
	int SetUniformv4f(const std::string &name, const Color4f &param);
	int SetUniformi(const std::string &name, int param);
	int SetUniformv2i(const std::string &name, int param1, int param2);
	int SetUniformmat4(const std::string &name, const Matrix4x4f &param);

	// Array uniforms
	int SetUniform1iv(const std::string &name, unsigned int numParams, const int* params);
	int SetUniform2iv(const std::string &name, unsigned int numParams, const int* params);
	int SetUniform3iv(const std::string &name, unsigned int numParams, const int* params);
	int SetUniform4iv(const std::string &name, unsigned int numParams, const int* params);
	int SetUniform1fv(const std::string &name, unsigned int numParams, const float* params);
	int SetUniform2fv(const std::string &name, unsigned int numParams, const float* params);
	int SetUniform3fv(const std::string &name, unsigned int numParams, const float* params);
	int SetUniform4fv(const std::string &name, unsigned int numParams, const float* params);

	// ---------------------------- Using Attribute Location ----------------------------

	void SetUniformf(int paramLoc, float param);
	void SetUniformv2f(int paramLoc, float param1, float param2);
	void SetUniformv2f(int paramLoc, const Vec2f &param);
	void SetUniformv3f(int paramLoc, float param1, float param2, float param3);
	void SetUniformv3f(int paramLoc, const Vec3f &param);
	void SetUniformv3f(int paramLoc, const Color3f &param);
	void SetUniformv4f(int paramLoc, float param1, float param2, float param3, float param4);
	void SetUniformv4f(int paramLoc, const Vec4f &param);
	void SetUniformv4f(int paramLoc, const Color4f &param);
	void SetUniformi(int paramLoc, int param);
	void SetUniformv2i(int paramLoc, int param1, int param2);
	void SetUniformmat4(int paramLoc, const Matrix4x4f &param);

	// Array uniforms
	void SetUniform1iv(int paramLoc, unsigned int numParams, const int* params);
	void SetUniform2iv(int paramLoc, unsigned int numParams, const int* params);
	void SetUniform3iv(int paramLoc, unsigned int numParams, const int* params);
	void SetUniform4iv(int paramLoc, unsigned int numParams, const int* params);
	void SetUniform1fv(int paramLoc, unsigned int numParams, const float* params);
	void SetUniform2fv(int paramLoc, unsigned int numParams, const float* params);
	void SetUniform3fv(int paramLoc, unsigned int numParams, const float* params);
	void SetUniform4fv(int paramLoc, unsigned int numParams, const float* params);

	void Bind();
	static void Unbind();

	unsigned int GetProgID() const;

	// Manual texture unit setting
	int SetTextureAttachmentToUniform(const std::string &name, unsigned int attachment);
	void SetTextureAttachmentToUniform(int paramLoc, unsigned int attachment);
	void BindShaderTextures();
	void UnbindShaderTextures();

	// Asset factory
	static Asset* Asset_Factory();
};

// Helper functions for setting up uniform arrays
void SetVec3f(std::vector<float> &vals, int index, const Vec3f &vec);

void SetColor3f(std::vector<float> &vals, int index, const Color3f &col);