#include <Renderer/Window.h>

#include <Renderer/SDL_OpenGL.h>

#include <assert.h>

Window::Window()
	: m_created(false),
	m_fov(45.0f),
	m_zNear(0.1f), m_zFar(2000.0f)
{
}

Window::~Window()
{
	if(m_created)
		SDL_Quit();
}

bool Window::Create(unsigned int pixel_width, unsigned int pixel_height, bool fullScreen)
{
	m_pixel_width = pixel_width;
	m_pixel_height = pixel_height;
	m_projected_width = static_cast<float>(m_pixel_width);
	m_projected_height = static_cast<float>(m_pixel_height);

	if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
		return false;

	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);

	if(fullScreen)
	{
		if(SDL_SetVideoMode(pixel_width, pixel_height, 32, SDL_HWSURFACE | SDL_GL_DOUBLEBUFFER | SDL_OPENGL | SDL_FULLSCREEN) == NULL)
			return false;
	}
	else
	{
		if(SDL_SetVideoMode(pixel_width, pixel_height, 32, SDL_HWSURFACE | SDL_GL_DOUBLEBUFFER | SDL_OPENGL) == NULL)
			return false;
	}

	if(glewInit() != GLEW_OK)
		return false;

	glEnable(GL_MULTISAMPLE);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glFrontFace(GL_CCW);

	m_created = true;

	return true;
}

void Window::SetViewport()
{
	glViewport(0, 0, m_pixel_width, m_pixel_height);
}

void Window::SetProjection()
{
	assert(m_created);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(m_fov, m_projected_width / m_projected_height, m_zNear, m_zFar);
	glMatrixMode(GL_MODELVIEW);
}

void Window::SetOrtho()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// Doesn't use m_zNear and m_zFar since ortho projections never really need anything adjustable
	glOrtho(0, m_projected_width, 0, m_projected_height, -1, 1); 
	glMatrixMode(GL_MODELVIEW);
}

unsigned int Window::GetPixelWidth() const
{
	return m_pixel_width;
}

unsigned int Window::GetPixelHeight() const
{
	return m_pixel_height;
}