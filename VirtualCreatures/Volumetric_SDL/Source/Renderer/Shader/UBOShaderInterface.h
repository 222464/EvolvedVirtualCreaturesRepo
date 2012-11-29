#pragma once

#include <Renderer/BufferObjects/VBO.h>

#include <Renderer/Shader/Shader.h>

#include <string>

#include <unordered_map>

class UBOShaderInterface
{
private:
	Shader* m_pShader;

	unsigned int m_blockIndex;
	unsigned int m_attributeLocation;
	unsigned int m_bufferBindIndex;

	int m_blockSize;

	std::unordered_map<std::string, unsigned int> m_uniformNameToOffset;

public:
	void Create(const std::string &uniformBlockName, Shader* shader, const char** uniformNames, unsigned int numUniformNames);
	void SetUpBuffer(VBO &buffer);
	void SetBindingIndex(unsigned int index);

	void BindBufferToSetIndex(VBO &buffer);
	void UnbindSetIndex();

	int GetBlockSize() const;
	Shader* GetShader() const;

	// Bind buffer you want to modify before using these
	void SetUniform(const std::string &name, GLintptr size, GLvoid* param);

	void SetUniformf(const std::string &name, float param);
	void SetUniformv2f(const std::string &name, const Vec2f &params);
	void SetUniformv3f(const std::string &name, const Vec3f &params);
	void SetUniformv3f(const std::string &name, const Color3f &params);
	void SetUniformv4f(const std::string &name, const Vec4f &params);
	void SetUniformv4f(const std::string &name, const Color4f &params);

	// TODO: Add more types!
};
