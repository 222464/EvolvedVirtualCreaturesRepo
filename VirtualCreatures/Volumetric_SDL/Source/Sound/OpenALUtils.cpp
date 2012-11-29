#include <sound/OpenALUtils.h>

#include <iostream>

bool CheckForALError()
{
	unsigned int errorCode = alGetError();

	if(errorCode != AL_NO_ERROR)
	{
		std::cerr << alGetString(errorCode) << std::endl;
		abort();
		return true;
	}

	return false;
}