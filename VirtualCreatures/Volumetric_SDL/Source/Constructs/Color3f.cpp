#include <Constructs/Color3f.h>

#include <Renderer/SDL_OpenGL.h>

Color3f::Color3f()
{
}

Color3f::Color3f(float R, float G, float B)
	: r(R), g(G), b(B)
{
}

void Color3f::GL_SetColor()
{
	glColor3f(r, g, b);
}