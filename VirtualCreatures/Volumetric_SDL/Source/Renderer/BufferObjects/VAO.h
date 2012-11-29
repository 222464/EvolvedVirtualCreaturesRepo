#pragma once

// Helper class for vertex buffer objects
class VAO
{
private:
	unsigned int m_ID;

public:
	VAO();
	~VAO();

	void Create();
	void Destroy();

	void Bind();
	static void Unbind();

	bool Created();

	unsigned int GetID();
};

