#include <AssetManager/Asset_Texture.h>

#include <Renderer/SDL_OpenGL.h>
#include <SDL_image.h>

#include <iostream>

#include <assert.h>

Asset_Texture::Asset_Texture()
	: m_ID(0)
{
}

Asset_Texture::~Asset_Texture()
{
	if(m_ID != 0)
		glDeleteTextures(1, &m_ID);
}

bool Asset_Texture::LoadAsset(const std::string &name)
{
	assert(m_ID == 0);

	SDL_Surface* surface = IMG_Load(name.c_str());
	
	if(surface == NULL)
	{
		std::cerr << "Could not load image file " << name << std::endl;

		return false;
	}
 
	glGenTextures(1, &m_ID);
	glBindTexture(GL_TEXTURE_2D, m_ID);
 
	unsigned int mode;
 
	if(surface->format->BytesPerPixel == 4)
		mode = GL_RGBA;
	else if(surface->format->BytesPerPixel == 3)
		mode = GL_RGB;
	else
	{
		std::cerr << "Image " << name << " is not truecolor!" << std::endl;

		return false;
	}

	m_width = surface->w;
	m_height = surface->h;

	glTexImage2D(GL_TEXTURE_2D, 0, mode, m_width, m_height, 0, mode, GL_UNSIGNED_BYTE, surface->pixels);

	SDL_FreeSurface(surface);

	// Default settings
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GL_ERROR_CHECK();

	return true;
}

unsigned int Asset_Texture::GetTextureID() const
{
	return m_ID;
}

void Asset_Texture::GenMipMaps()
{
	glBindTexture(GL_TEXTURE_2D, m_ID);

	// Only min filter, since mipmapping is only for when zoomed out a lot
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
	glGenerateMipmap(GL_TEXTURE_2D);
}

void Asset_Texture::Bind() const
{
	glBindTexture(GL_TEXTURE_2D, m_ID);
}

int Asset_Texture::GetWidth() const
{
	return m_width;
}

int Asset_Texture::GetHeight() const
{
	return m_height;
}

Asset* Asset_Texture::Asset_Factory()
{
	return new Asset_Texture();
}