#pragma once

struct TextureAndAttachment
{
	unsigned int m_textureHandle, m_attachment, m_target;

	TextureAndAttachment();
	TextureAndAttachment(unsigned int textureHandle, unsigned int attachment, unsigned int target);
};

