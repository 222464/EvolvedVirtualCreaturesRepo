#include <Constructs/Color4f.h>

#include <Renderer/SDL_OpenGL.h>

Color4f::Color4f()
{
}

Color4f::Color4f(float R, float G, float B, float A)
	: r(R), g(G), b(B), a(A)
{
}

void Color4f::GL_SetColor()
{
	glColor4f(r, g, b, a);
}