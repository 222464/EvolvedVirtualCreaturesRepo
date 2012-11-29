#include <Renderer/Shader/UBOShaderInterface.h>

#include <assert.h>

void UBOShaderInterface::Create(const std::string &uniformBlockName, Shader* shader, const char** uniformNames, unsigned int numUniformNames)
{
	m_pShader = shader;

	m_blockIndex = glGetUniformBlockIndex(m_pShader->GetProgID(), uniformBlockName.c_str());

	assert(m_blockIndex != GL_INVALID_INDEX);

	glGetActiveUniformBlockiv(m_pShader->GetProgID(), m_blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &m_blockSize);

	unsigned int* indices = new unsigned int[numUniformNames];

	glGetUniformIndices(m_pShader->GetProgID(), numUniformNames, uniformNames, indices);

	// Query offsets and associate names to the offsets
	int* offsets = new int[numUniformNames];

	glGetActiveUniformsiv(m_pShader->GetProgID(), numUniformNames, indices, GL_UNIFORM_OFFSET, offsets);

	delete[] indices;

	for(unsigned int i = 0; i < numUniformNames; i++)
		m_uniformNameToOffset[uniformNames[i]] = offsets[i];

	delete[] offsets;

	GL_ERROR_CHECK();
}

void UBOShaderInterface::SetUpBuffer(VBO &buffer)
{
	assert(!buffer.Created());

	buffer.Create();

	buffer.Bind(GL_UNIFORM_BUFFER);

	glBufferData(GL_UNIFORM_BUFFER, m_blockSize, NULL, GL_STREAM_DRAW);

	// Allocate VBO using this size
	buffer.Unbind();
}

void UBOShaderInterface::SetBindingIndex(unsigned int index)
{
	m_bufferBindIndex = index;

	glUniformBlockBinding(m_pShader->GetProgID(), m_blockIndex, m_bufferBindIndex);
}

int UBOShaderInterface::GetBlockSize() const
{
	return m_blockSize;
}

Shader* UBOShaderInterface::GetShader() const
{
	return m_pShader;
}

void UBOShaderInterface::BindBufferToSetIndex(VBO &buffer)
{
	glBindBufferBase(GL_UNIFORM_BUFFER, m_bufferBindIndex, buffer.GetID());
}

void UBOShaderInterface::UnbindSetIndex()
{
	glBindBufferBase(GL_UNIFORM_BUFFER, m_bufferBindIndex, 0);
}

void UBOShaderInterface::SetUniform(const std::string &name, GLintptr size, GLvoid* param)
{
	glBufferSubData(GL_UNIFORM_BUFFER, m_uniformNameToOffset[name], size, param);
}

void UBOShaderInterface::SetUniformf(const std::string &name, float param)
{
	glBufferSubData(GL_UNIFORM_BUFFER, m_uniformNameToOffset[name], sizeof(float), &param);
}

void UBOShaderInterface::SetUniformv2f(const std::string &name, const Vec2f &params)
{
	glBufferSubData(GL_UNIFORM_BUFFER, m_uniformNameToOffset[name], sizeof(Vec2f), &params);
}

void UBOShaderInterface::SetUniformv3f(const std::string &name, const Vec3f &params)
{
	glBufferSubData(GL_UNIFORM_BUFFER, m_uniformNameToOffset[name], sizeof(Vec3f), &params);
}

void UBOShaderInterface::SetUniformv3f(const std::string &name, const Color3f &params)
{
	glBufferSubData(GL_UNIFORM_BUFFER, m_uniformNameToOffset[name], sizeof(Color3f), &params);
}

void UBOShaderInterface::SetUniformv4f(const std::string &name, const Vec4f &params)
{
	glBufferSubData(GL_UNIFORM_BUFFER, m_uniformNameToOffset[name], sizeof(Vec4f), &params);
}

void UBOShaderInterface::SetUniformv4f(const std::string &name, const Color4f &params)
{
	glBufferSubData(GL_UNIFORM_BUFFER, m_uniformNameToOffset[name], sizeof(Color4f), &params);
}