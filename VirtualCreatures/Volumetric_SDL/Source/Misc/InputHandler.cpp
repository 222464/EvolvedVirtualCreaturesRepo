#include <Misc/InputHandler.h>

#include <assert.h>

InputHandler::InputHandler()
	: m_pWindow(NULL), m_quit(false),
	m_lmbDown(false), m_rmbDown(false),
	m_lmbClicked(false), m_rmbClicked(false)
{
	// Get the number of keys
	SDL_GetKeyState(&m_numKeys);
	m_keyStateArraySizeBytes = m_numKeys * sizeof(Uint8);
	m_pPastKeyStates = new Uint8[m_numKeys];
	m_pCurrentKeyStates = new Uint8[m_numKeys];

	// Default to all false
	memset(m_pPastKeyStates, 0, m_keyStateArraySizeBytes);
	memset(m_pCurrentKeyStates, 0, m_keyStateArraySizeBytes);
}

InputHandler::~InputHandler()
{
	// Delete key states
	delete m_pPastKeyStates;
	delete m_pCurrentKeyStates;
}

void InputHandler::GetInputs()
{
	SDL_Event sdlEvent;

	m_lmbClicked = false;
	m_rmbClicked = false;

	while(SDL_PollEvent(&sdlEvent))
	{
		switch(sdlEvent.type)
		{
		case SDL_QUIT:
			m_quit = true;
			break;

		case SDL_MOUSEMOTION:
			m_mouseX = sdlEvent.motion.x;

			if(m_pWindow == NULL)
				m_mouseY = sdlEvent.motion.y;
			else
				m_mouseY = m_pWindow->GetPixelHeight() - sdlEvent.motion.y;
			break;

		case SDL_MOUSEBUTTONDOWN:
			if(sdlEvent.button.button == SDL_BUTTON_LEFT)
			{
				if(!m_lmbDown)
					m_lmbClicked = true;	

				m_lmbDown = true;
			}
			else if(sdlEvent.button.button == SDL_BUTTON_RIGHT)
			{
				if(!m_rmbDown)
					m_rmbClicked = true;

				m_rmbDown = true;
			}

			break;

		case SDL_MOUSEBUTTONUP:
			if(sdlEvent.button.button == SDL_BUTTON_LEFT)
				m_lmbDown = false;
			else if(sdlEvent.button.button == SDL_BUTTON_RIGHT)
				m_rmbDown = false;

			break;
		}
	}

	// Copy states to past state buffer
	memcpy(m_pPastKeyStates, m_pCurrentKeyStates, m_keyStateArraySizeBytes);

	// Get key states
	Uint8* pStates = SDL_GetKeyState(NULL);

	// Copy new states
	memcpy(m_pCurrentKeyStates, pStates, m_keyStateArraySizeBytes);
}

bool InputHandler::Quit()
{
	return m_quit;
}

bool InputHandler::LMBDown()
{
	return m_lmbDown;
}

bool InputHandler::RMBDown()
{
	return m_rmbDown;
}

bool InputHandler::LMBClicked()
{
	return m_lmbClicked;
}

bool InputHandler::RMBClicked()
{
	return m_rmbClicked;
}

unsigned int InputHandler::GetMouseX()
{
	return m_mouseX;
}

unsigned int InputHandler::GetMouseY()
{
	return m_mouseY;
}

Uint8 InputHandler::GetCurrentKeyState(Uint8 key)
{
	return m_pCurrentKeyStates[key];
}

Uint8 InputHandler::GetCurrentKeyState(SDLKey key)
{
	return m_pCurrentKeyStates[key];
}

Uint8 InputHandler::GetPastKeyState(Uint8 key)
{
	return m_pPastKeyStates[key];
}

Uint8 InputHandler::GetPastKeyState(SDLKey key)
{
	return m_pPastKeyStates[key];
}

bool InputHandler::KeyPressed(Uint8 key)
{
	return !m_pPastKeyStates[key] && m_pCurrentKeyStates[key];
}

bool InputHandler::KeyPressed(SDLKey key)
{
	return !m_pPastKeyStates[key] && m_pCurrentKeyStates[key];
}

bool InputHandler::KeyReleased(Uint8 key)
{
	return m_pPastKeyStates[key] && !m_pCurrentKeyStates[key];
}

bool InputHandler::KeyReleased(SDLKey key)
{
	return m_pPastKeyStates[key] && !m_pCurrentKeyStates[key];
}

void InputHandler::SetMousePos(unsigned int x, unsigned int y)
{
	SDL_WarpMouse(x, y);
}

void InputHandler::KeepMouseInWindow(bool keepInWindow)
{
	if(keepInWindow)
		SDL_WM_GrabInput(SDL_GRAB_ON);
	else
		SDL_WM_GrabInput(SDL_GRAB_OFF);
}

void InputHandler::ShowMouse(bool show)
{
	SDL_ShowCursor(show);
}
