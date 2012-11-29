#include <Renderer/Shader/TextureAndAttachment.h>

TextureAndAttachment::TextureAndAttachment()
{
}

TextureAndAttachment::TextureAndAttachment(unsigned int textureHandle, unsigned int attachment, unsigned int target)
	: m_textureHandle(textureHandle), m_attachment(attachment), m_target(target)
{
}