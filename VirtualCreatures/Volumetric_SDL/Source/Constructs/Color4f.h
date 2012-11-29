#pragma once

class Color4f
{
public:
	float r, g, b, a;

	Color4f();
	Color4f(float R, float G, float B, float A);

	void GL_SetColor();
};

