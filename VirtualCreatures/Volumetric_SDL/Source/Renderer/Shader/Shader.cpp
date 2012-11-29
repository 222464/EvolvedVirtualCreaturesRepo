#include <Renderer/Shader/Shader.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include <assert.h>

// Can update individual indices of uniform array by getting attribute location of each index

Shader::Shader()
	: m_progID(0), m_geomID(0), m_vertID(0), m_fragID(0),
	m_lastAttachment(0), m_created(false)
{
}

Shader::~Shader()
{
	if(m_created)
	{
		glDeleteProgram(m_progID);

		if(m_vertID != 0)
			glDeleteShader(m_vertID);

		if(m_fragID != 0)
			glDeleteShader(m_fragID);
	}
}

bool Shader::LoadShader(const std::string &name, unsigned int id)
{
	std::ifstream fromFile;

	fromFile.open(name);

	if(!fromFile.is_open())
	{
		std::cerr << "Could not open file: " << name << "!" << std::endl;
		return false;
	}

	// Load the shader code into one massive string
	std::string fileString((std::istreambuf_iterator<char>(fromFile)), std::istreambuf_iterator<char>());

	const char* code = fileString.c_str();
	glShaderSource(id, 1, &code, NULL);
	glCompileShader(id);

	if(CheckLog(id))
	{
		std::cerr << "- in " << name << std::endl;
		return false;
	}

	return true;
}

bool Shader::LoadGeometryShader(const std::string &name, unsigned int &id)
{
	id = glCreateShaderObjectARB(GL_GEOMETRY_SHADER_ARB);

	if(!LoadShader(name, id))
	{
		glDeleteShader(id);

		return false;
	}

	return true;
}

bool Shader::LoadVertexShader(const std::string &name, unsigned int &id)
{
	id = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);

	if(!LoadShader(name, id))
	{
		glDeleteShader(id);

		return false;
	}

	return true;
}

bool Shader::LoadFragmentShader(const std::string &name, unsigned int &id)
{
	id = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

	if(!LoadShader(name, id))
	{
		glDeleteShader(id);

		return false;
	}

	return true;
}

bool Shader::CheckLog(unsigned int id)
{
	int result;

	glGetShaderiv(id, GL_COMPILE_STATUS, &result);

	// If did not compile without error, show the log
	if(result == GL_FALSE)
	{
		int logLength;

		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength);

		if(logLength <= 0)
		{
			std::cerr << "Unable to compile shader: Error: Invalid log length \"" << logLength << "\": Could not retrieve error log!" << std::endl;

			return false;
		}

		// Allocate the string
		char *log = new char[logLength];

		glGetShaderInfoLog(id, logLength, &result, log);

		// Print the log
		std::cerr << "Unable to compile shader: " << log << std::endl;

		delete log;

		return true;
	}

	return false;
}

bool Shader::Link(unsigned int id)
{
	glLinkProgram(id);

	int result;

	// Check if linking was successful
	glGetProgramiv(id, GL_LINK_STATUS, &result);

	if(result == GL_FALSE)
	{
		// Not validated, print out the log
		int logLength;

		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logLength);

		if(logLength <= 0)
		{
			std::cerr << "Unable to link program: Error: Invalid log length \"" << logLength << "\": Could not retrieve error log!" << std::endl;

			return false;
		}

		// Allocate the string
		char* log = new char[logLength];

		glGetProgramInfoLog(id, logLength, &result, log);

		std::cerr << "Unable to link program: " << log << std::endl;

		delete log;

		return false;
	}

	return true;
}

bool Shader::Validate(unsigned int id)
{
	glValidateProgram(id);

	int result;

	// Check if validation was successful
	glGetProgramiv(id, GL_VALIDATE_STATUS, &result);

	if(result == GL_FALSE)
	{
		// Not validated, print out the log
		int logLength;
	
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logLength);

		if(logLength <= 0)
		{
			std::cerr << "Unable to validate program: Error: Invalid log length \"" << logLength << "\": Could not retrieve error log!" << std::endl;

			return true;
		}

		// Allocate the string
		char* log = new char[logLength];

		glGetProgramInfoLog(id, logLength, &result, log);

		std::cerr << "Unable to validate program: " << log << std::endl;

		delete log;

		return false;
	}

	return true;
}

bool Shader::LoadAsset(const std::string &name)
{
	// Name contains vertex shader file name and the fragment shader file name, so extract those
	std::istringstream is(name);

	std::string geomName, fragName, vertName;
	is >> geomName >> vertName >> fragName;

	// Load the shaders
	if(geomName != "NONE")
		if(!LoadGeometryShader(geomName, m_geomID))
		{
			std::cout << "Could not load geometry shader!" << std::endl;
			return false;
		}

	if(vertName != "NONE")
		if(!LoadVertexShader(vertName, m_vertID))
		{
			std::cout << "Could not load vertex shader!" << std::endl;
			return false;
		}

	if(fragName != "NONE")
		if(!LoadFragmentShader(fragName, m_fragID))
		{
			std::cout << "Could not load fragment shader!" << std::endl;
			return false;
		}

	// Create the main shader
	m_progID = glCreateProgramObjectARB();

	// Attach the shader components (vertex and fragment) to the program
	if(m_geomID != 0)
		glAttachObjectARB(m_progID, m_geomID);
	if(m_vertID != 0)
		glAttachObjectARB(m_progID, m_vertID);
	if(m_fragID != 0)
		glAttachObjectARB(m_progID, m_fragID);

	if(!Link(m_progID))
	{
		std::cerr << "- in " << name << std::endl;

		glDeleteShader(m_progID);

		return false;
	}

	if(CheckForGLError())
	{
		glDeleteShader(m_progID);
		return false;
	}

	m_created = true;

	return true;
}

void Shader::SetShaderTexture(const std::string &name, unsigned int textureID, unsigned int target)
{
	assert(m_created);

	CheckProgram();

	// See if the texture already exists, in which case it is simply updated
	std::unordered_map<std::string, TextureAndAttachment>::iterator it = m_textures.find(name);
		
	// If the texture does not already exist
	if(it == m_textures.end())
	{
		// Check that the uniform exists
		int baseImageLoc = glGetUniformLocationARB(m_progID, name.c_str());
			
		if(baseImageLoc == -1)
			std::cerr << "Could not find the uniform " << name << "!" << std::endl;
		else
		{
			glUniform1iARB(baseImageLoc, m_lastAttachment);
				
			// Add the texture to the hash
			m_textures[name] = TextureAndAttachment(textureID, m_lastAttachment, target);
				
			m_lastAttachment++;
		}
	}
	else
	{
		// Update the existing texture
		it->second.m_textureHandle = textureID;
	}
}

void  Shader::SetShaderTexture(const std::string &name, unsigned int textureAttachment, unsigned int textureID, unsigned int target)
{
	assert(m_created);

	CheckProgram();

	// See if the texture already exists, in which case it is simply updated
	std::unordered_map<std::string, TextureAndAttachment>::iterator it = m_textures.find(name);
		
	// If the texture does not already exist
	if(it == m_textures.end())
	{
		// Check that the uniform exists
		int baseImageLoc = glGetUniformLocationARB(m_progID, name.c_str());
			
		if(baseImageLoc == -1)
			std::cerr << "Could not find the uniform " << name << "!" << std::endl;
		else
		{
			glUniform1iARB(baseImageLoc, textureAttachment);
				
			// Add the texture to the hash
			m_textures[name] = TextureAndAttachment(textureID, textureAttachment, target);
		}
	}
	else
	{
		// Update the existing texture
		it->second.m_textureHandle = textureID;
		it->second.m_attachment = textureAttachment;
	}
}

void Shader::BindShaderTextures()
{
	// Bind all textures
	for(std::unordered_map<std::string, TextureAndAttachment>::iterator it = m_textures.begin(); it != m_textures.end(); it++)
	{
		glActiveTexture(GL_TEXTURE0 + it->second.m_attachment);
		glBindTexture(it->second.m_target, it->second.m_textureHandle);
	}

	// Set to normal one
	glActiveTexture(GL_TEXTURE0);
}

void Shader::UnbindShaderTextures()
{
	// Bind all used textures
	for(unsigned int i = 0, size = m_textures.size(); i < size; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// Set to normal one
	glActiveTexture(GL_TEXTURE0);
}

void Shader::Bind()
{
	glUseProgramObjectARB(m_progID);
}

void Shader::Unbind()
{
	glUseProgramObjectARB(0);
}

int Shader::GetAttributeLocation(const std::string &name)
{
	int paramLoc;

	std::unordered_map<std::string, int>::iterator it = m_attributeLocations.find(name);

	if(it == m_attributeLocations.end())
		m_attributeLocations[name] = paramLoc = glGetUniformLocationARB(m_progID, name.c_str());
	else
		paramLoc = it->second;

	return paramLoc;
}

int Shader::SetUniformf(const std::string &name, float param)
{
	CheckProgram();

	int paramLoc = GetAttributeLocation(name);

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform " << name << "!" << std::endl;
	else
		glUniform1fARB(paramLoc, param);
#else
	glUniform1fARB(paramLoc, param);
#endif

	return paramLoc;
}

int Shader::SetUniformv2f(const std::string &name, float param1, float param2)
{
	CheckProgram();

	int paramLoc = GetAttributeLocation(name);

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform " << name << "!" << std::endl;
	else
		glUniform2fARB(paramLoc, param1, param2);
#else
	glUniform2fARB(paramLoc, param1, param2);
#endif
	
	return paramLoc;
}

int Shader::SetUniformv2f(const std::string &name, const Vec2f &param)
{
	return SetUniformv2f(name, param.x, param.y);
}

int Shader::SetUniformv3f(const std::string &name, float param1, float param2, float param3)
{
	CheckProgram();

	int paramLoc = GetAttributeLocation(name);

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform " << name << "!" << std::endl;
	else
		glUniform3fARB(paramLoc, param1, param2, param3);
#else
	glUniform3fARB(paramLoc, param1, param2, param3);
#endif
	
	return paramLoc;
}

int Shader::SetUniformv3f(const std::string &name, const Vec3f &param)
{
	return SetUniformv3f(name, param.x, param.y, param.z);
}

int Shader::SetUniformv3f(const std::string &name, const Color3f &param)
{
	return SetUniformv3f(name, param.r, param.g, param.b);
}

int Shader::SetUniformv4f(const std::string &name, float param1, float param2, float param3, float param4)
{
	CheckProgram();

	int paramLoc = GetAttributeLocation(name);

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform " << name << "!" << std::endl;
	else
		glUniform4fARB(paramLoc, param1, param2, param3, param4);
#else
	glUniform4fARB(paramLoc, param1, param2, param3, param4);
#endif
	
	return paramLoc;
}

int Shader::SetUniformv4f(const std::string &name, const Vec4f &param)
{
	return SetUniformv4f(name, param.x, param.y, param.z, param.w);
}

int Shader::SetUniformv4f(const std::string &name, const Color4f &param)
{
	return SetUniformv4f(name, param.r, param.g, param.b, param.a);
}

int Shader::SetUniformi(const std::string &name, int param)
{
	CheckProgram();

	int paramLoc = GetAttributeLocation(name);

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform " << name << "!" << std::endl;
	else
		glUniform1iARB(paramLoc, param);
#else
	glUniform1iARB(paramLoc, param);
#endif
	
	return paramLoc;
}

int Shader::SetUniformv2i(const std::string &name, int param1, int param2)
{
	CheckProgram();

	int paramLoc = GetAttributeLocation(name);

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform " << name << "!" << std::endl;
	else
		glUniform2iARB(paramLoc, param1, param2);
#else
	glUniform2iARB(paramLoc, param1, param2);
#endif
	
	return paramLoc;
}

int Shader::SetUniformmat4(const std::string &name, const Matrix4x4f &param)
{
	CheckProgram();

	int paramLoc = GetAttributeLocation(name);

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform " << name << "!" << std::endl;
	else
		glUniformMatrix4fvARB(paramLoc, 1, false, param.m_elements);
#else
	glUniformMatrix4fvARB(paramLoc, 1, false, param.m_elements);
#endif
	
	return paramLoc;
}

int Shader::SetTextureAttachmentToUniform(const std::string &name, unsigned int attachment)
{
	CheckProgram();

	int paramLoc = GetAttributeLocation(name);

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform " << name << "!" << std::endl;
	else
		glUniform1iARB(paramLoc, attachment);
#else
	glUniform1iARB(paramLoc, attachment);
#endif
	
	return paramLoc;
}

void Shader::SetTextureAttachmentToUniform(int paramLoc, unsigned int attachment)
{
	CheckProgram();

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform at location" << paramLoc << "!" << std::endl;
	else
		glUniform1iARB(paramLoc, attachment);
#else
	glUniform1iARB(paramLoc, attachment);
#endif
}

int Shader::SetUniform1iv(const std::string &name, unsigned int numParams, const int* params)
{
	CheckProgram();

	int paramLoc = GetAttributeLocation(name);

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform " << name << "!" << std::endl;
	else
		glUniform1ivARB(paramLoc, numParams, params);
#else
	glUniform1ivARB(paramLoc, numParams, params);
#endif
	
	return paramLoc;
}

int Shader::SetUniform2iv(const std::string &name, unsigned int numParams, const int* params)
{
	CheckProgram();

	int paramLoc = GetAttributeLocation(name);

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform " << name << "!" << std::endl;
	else
		glUniform2ivARB(paramLoc, numParams , params);
#else
	glUniform2ivARB(paramLoc, numParams, params);
#endif
	
	return paramLoc;
}

int Shader::SetUniform3iv(const std::string &name, unsigned int numParams, const int* params)
{
	CheckProgram();

	int paramLoc = GetAttributeLocation(name);

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform " << name << "!" << std::endl;
	else
		glUniform3ivARB(paramLoc, numParams, params);
#else
	glUniform3ivARB(paramLoc, numParams, params);
#endif
	
	return paramLoc;
}

int Shader::SetUniform4iv(const std::string &name, unsigned int numParams, const int* params)
{
	CheckProgram();

	int paramLoc = GetAttributeLocation(name);

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform " << name << "!" << std::endl;
	else
		glUniform4ivARB(paramLoc, numParams, params);
#else
	glUniform4ivARB(paramLoc, numParams, params);
#endif
	
	return paramLoc;
}

int Shader::SetUniform1fv(const std::string &name, unsigned int numParams, const float* params)
{
	CheckProgram();

	int paramLoc = GetAttributeLocation(name);

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform " << name << "!" << std::endl;
	else
		glUniform1fvARB(paramLoc, numParams, params);
#else
	glUniform1fvARB(paramLoc, numParams, params);
#endif
	
	return paramLoc;
}

int Shader::SetUniform2fv(const std::string &name, unsigned int numParams, const float* params)
{
	CheckProgram();

	int paramLoc = GetAttributeLocation(name);

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform " << name << "!" << std::endl;
	else
		glUniform2fvARB(paramLoc, numParams, params);
#else
	glUniform2fvARB(paramLoc, numParams, params);
#endif
	
	return paramLoc;
}

int Shader::SetUniform3fv(const std::string &name, unsigned int numParams, const float* params)
{
	CheckProgram();

	int paramLoc = GetAttributeLocation(name);

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform " << name << "!" << std::endl;
	else
		glUniform3fvARB(paramLoc, numParams, params);
#else
	glUniform3fvARB(paramLoc, numParams, params);
#endif
	
	return paramLoc;
}

int Shader::SetUniform4fv(const std::string &name, unsigned int numParams, const float* params)
{
	CheckProgram();

	int paramLoc = GetAttributeLocation(name);

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform " << name << "!" << std::endl;
	else
		glUniform4fvARB(paramLoc, numParams, params);
#else
	glUniform4fvARB(paramLoc, numParams, params);
#endif

	return paramLoc;
}

// -------------------- Attribute Location Versions ----------------------

void Shader::SetUniformf(int paramLoc, float param)
{
	CheckProgram();

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform at location" << paramLoc << "!" << std::endl;
	else
		glUniform1fARB(paramLoc, param);
#else
	glUniform1fARB(paramLoc, param);
#endif
}

void Shader::SetUniformv2f(int paramLoc, float param1, float param2)
{
	CheckProgram();

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform at location" << paramLoc << "!" << std::endl;
	else
		glUniform2fARB(paramLoc, param1, param2);
#else
	glUniform2fARB(paramLoc, param1, param2);
#endif
}

void Shader::SetUniformv2f(int paramLoc, const Vec2f &param)
{
	SetUniformv2f(paramLoc, param.x, param.y);
}

void Shader::SetUniformv3f(int paramLoc, float param1, float param2, float param3)
{
	CheckProgram();

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform at location" << paramLoc << "!" << std::endl;
	else
		glUniform3fARB(paramLoc, param1, param2, param3);
#else
	glUniform3fARB(paramLoc, param1, param2, param3);
#endif
}

void Shader::SetUniformv3f(int paramLoc, const Vec3f &param)
{
	SetUniformv3f(paramLoc, param.x, param.y, param.z);
}

void Shader::SetUniformv3f(int paramLoc, const Color3f &param)
{
	SetUniformv3f(paramLoc, param.r, param.g, param.b);
}

void Shader::SetUniformv4f(int paramLoc, float param1, float param2, float param3, float param4)
{
	CheckProgram();

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform at location" << paramLoc << "!" << std::endl;
	else
		glUniform4fARB(paramLoc, param1, param2, param3, param4);
#else
	glUniform4fARB(paramLoc, param1, param2, param3, param4);
#endif
}

void Shader::SetUniformv4f(int paramLoc, const Vec4f &param)
{
	SetUniformv4f(paramLoc, param.x, param.y, param.z, param.w);
}

void Shader::SetUniformv4f(int paramLoc, const Color4f &param)
{
	SetUniformv4f(paramLoc, param.r, param.g, param.b, param.a);
}

void Shader::SetUniformi(int paramLoc, int param)
{
	CheckProgram();

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform at location" << paramLoc << "!" << std::endl;
	else
		glUniform1iARB(paramLoc, param);
#else
	glUniform1iARB(paramLoc, param);
#endif
}

void Shader::SetUniformv2i(int paramLoc, int param1, int param2)
{
	CheckProgram();

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform at location" << paramLoc << "!" << std::endl;
	else
		glUniform2iARB(paramLoc, param1, param2);
#else
	glUniform2iARB(paramLoc, param1, param2);
#endif
}

void Shader::SetUniformmat4(int paramLoc, const Matrix4x4f &param)
{
	CheckProgram();

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform at location" << paramLoc << "!" << std::endl;
	else
		glUniformMatrix4fvARB(paramLoc, 1, false, param.m_elements);
#else
	glUniformMatrix4fvARB(paramLoc, 1, false, param.m_elements);
#endif
}

void Shader::SetUniform1iv(int paramLoc, unsigned int numParams, const int* params)
{
	CheckProgram();

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform at location" << paramLoc << "!" << std::endl;
	else
		glUniform1ivARB(paramLoc, numParams, params);
#else
	glUniform1ivARB(paramLoc, numParams, params);
#endif
}

void Shader::SetUniform2iv(int paramLoc, unsigned int numParams, const int* params)
{
	CheckProgram();

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform at location" << paramLoc << "!" << std::endl;
	else
		glUniform2ivARB(paramLoc, numParams, params);
#else
	glUniform2ivARB(paramLoc, numParams, params);
#endif
}

void Shader::SetUniform3iv(int paramLoc, unsigned int numParams, const int* params)
{
	CheckProgram();

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform at location" << paramLoc << "!" << std::endl;
	else
		glUniform3ivARB(paramLoc, numParams, params);
#else
	glUniform3ivARB(paramLoc, numParams, params);
#endif
}

void Shader::SetUniform4iv(int paramLoc, unsigned int numParams, const int* params)
{
	CheckProgram();

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform at location" << paramLoc << "!" << std::endl;
	else
		glUniform4ivARB(paramLoc, numParams, params);
#else
	glUniform4ivARB(paramLoc, numParams, params);
#endif
}

void Shader::SetUniform1fv(int paramLoc, unsigned int numParams, const float* params)
{
	CheckProgram();

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform at location" << paramLoc << "!" << std::endl;
	else
		glUniform1fvARB(paramLoc, numParams, params);
#else
	glUniform1fvARB(paramLoc, numParams, params);
#endif
}

void Shader::SetUniform2fv(int paramLoc, unsigned int numParams, const float* params)
{
	CheckProgram();

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform at location" << paramLoc << "!" << std::endl;
	else
		glUniform2fvARB(paramLoc, numParams, params);
#else
	glUniform2fvARB(paramLoc, numParams, params);
#endif
}

void Shader::SetUniform3fv(int paramLoc, unsigned int numParams, const float* params)
{
	CheckProgram();

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform at location" << paramLoc << "!" << std::endl;
	else
		glUniform3fvARB(paramLoc, numParams, params);
#else
	glUniform3fvARB(paramLoc, numParams, params);
#endif
}

void Shader::SetUniform4fv(int paramLoc, unsigned int numParams, const float* params)
{
	CheckProgram();

	// If location was not found
#ifdef DEBUG
	if(paramLoc == -1)
		std::cerr << "Could not find the uniform at location" << paramLoc << "!" << std::endl;
	else
		glUniform4fvARB(paramLoc, numParams, params);
#else
	glUniform4fvARB(paramLoc, numParams, params);
#endif
}

unsigned int Shader::GetProgID() const
{
	return m_progID;
}

Asset* Shader::Asset_Factory()
{
	return new Shader();
}

void SetVec3f(std::vector<float> &vals, int index, const Vec3f &vec)
{
	int realIndex = index * 3;

	vals[realIndex] = vec.x;
	vals[realIndex + 1] = vec.y;
	vals[realIndex + 2] = vec.z;
}

void SetColor3f(std::vector<float> &vals, int index, const Color3f &col)
{
	int realIndex = index * 3;

	vals[realIndex] = col.r;
	vals[realIndex + 1] = col.g;
	vals[realIndex + 2] = col.b;
}