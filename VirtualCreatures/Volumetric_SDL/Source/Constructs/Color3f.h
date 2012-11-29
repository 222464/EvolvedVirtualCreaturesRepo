#pragma once

class Color3f
{
public:
	float r, g, b;

	Color3f();
	Color3f(float R, float G, float B);

	void GL_SetColor();
};

