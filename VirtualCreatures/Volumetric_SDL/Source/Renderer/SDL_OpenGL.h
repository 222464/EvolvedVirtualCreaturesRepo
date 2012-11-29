#pragma once

// To avoid GLEW and SDL conflicts
#define NO_SDL_GLEXT

#include <SDL.h>
#include <gl/glew.h>
#include <SDL_opengl.h>

#include <string>

// Vertex attribute locations
#define ATTRIB_POSITION 0
#define ATTRIB_NORMAL 1
#define ATTRIB_TEXCOORD 2

bool CheckForGLError();

// So only runs debug function when in debug mode
#ifdef DEBUG
#define GL_ERROR_CHECK() CheckForGLError()
#else
#define GL_ERROR_CHECK()
#endif

void GetLogFromID(int id, std::string &info);