#include <Renderer/SDL_OpenGL.h>

#include <iostream>

bool CheckForGLError()
{
	unsigned int errorCode = glGetError();

	if(errorCode != GL_NO_ERROR)
	{
		std::cerr << gluErrorString(errorCode) << std::endl;
		abort();
		return true;
	}

	return false;
}