#pragma once

// Helper class for vertex buffer objects
class VBO
{
private:
	unsigned int m_ID;

	unsigned int m_usage;

public:
	VBO();
	~VBO();

	void Create();
	void Destroy();

	/*
		The "usage" argument can be either of the following values:

		GL_ELEMENT_ARRAY_BUFFER
		GL_ARRAY_BUFFER
		GL_UNIFORM_BUFFER
		GL_TEXTURE_BUFFER
	*/

	void Bind(unsigned int usage);
	void Unbind();

	static void Unbind(unsigned int usage);

	bool Created();

	unsigned int GetID();
};

