#pragma once

#include <System/Uncopyable.h>
#include <Renderer/Window.h>
#include <SDL.h>

class InputHandler :
	public Uncopyable
{
private:
	unsigned int m_mouseX, m_mouseY;

	bool m_quit;

	bool m_lmbDown, m_rmbDown;
	bool m_lmbClicked, m_rmbClicked;

	Uint8 *m_pPastKeyStates;
	Uint8 *m_pCurrentKeyStates;

	int m_numKeys;
	int m_keyStateArraySizeBytes;

public:
	InputHandler();
	~InputHandler();
	
	// If set as NULL, will not invert Y
	Window* m_pWindow;

	void GetInputs();

	bool Quit();
	bool LMBDown();
	bool RMBDown();

	bool LMBClicked();
	bool RMBClicked();

	Uint8 GetCurrentKeyState(Uint8 key);
	Uint8 GetCurrentKeyState(SDLKey key);
	Uint8 GetPastKeyState(Uint8 key);
	Uint8 GetPastKeyState(SDLKey key);

	bool KeyPressed(Uint8 key);
	bool KeyPressed(SDLKey key);
	bool KeyReleased(Uint8 key);
	bool KeyReleased(SDLKey key);

	unsigned int GetMouseX();
	unsigned int GetMouseY();

	static void SetMousePos(unsigned int x, unsigned int y);
	static void KeepMouseInWindow(bool keepInWindow);
	static void ShowMouse(bool show);
};

