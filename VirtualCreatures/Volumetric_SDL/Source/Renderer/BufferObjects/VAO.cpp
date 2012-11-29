#include <Renderer/BufferObjects/VAO.h>
#include <Renderer/SDL_OpenGL.h>

#include <assert.h>

#include <iostream>

VAO::VAO()
	: m_ID(0)
{
}

VAO::~VAO()
{
	if(m_ID != 0)
		glDeleteVertexArrays(1, &m_ID);
}

void VAO::Create()
{
	assert(m_ID == 0);

	glGenVertexArrays(1, &m_ID);

	if(m_ID == 0)
		std::cout << "Could not create VAO!" << std::endl;
}

void VAO::Destroy()
{
	assert(m_ID != 0);

	glDeleteVertexArrays(1, &m_ID);

	m_ID = 0;
}

void VAO::Bind()
{
	glBindVertexArray(m_ID);
}

void VAO::Unbind()
{
	glBindVertexArray(0);
}

bool VAO::Created()
{
	return m_ID != 0;
}

unsigned int VAO::GetID()
{
	return m_ID;
}