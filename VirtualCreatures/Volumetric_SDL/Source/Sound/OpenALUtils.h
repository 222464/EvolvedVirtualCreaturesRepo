#pragma once

#include <al.h>
#include <alut.h>

bool CheckForALError();

// So only runs debug function when in debug mode
#ifdef DEBUG
#define AL_ERROR_CHECK() CheckForALError()
#else
#define AL_ERROR_CHECK() false
#endif
