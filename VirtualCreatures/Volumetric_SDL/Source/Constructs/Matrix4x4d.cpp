#include <Constructs/Matrix4x4d.h>

#include <Renderer/SDL_OpenGL.h>

#include <assert.h>

Matrix4x4d::Matrix4x4d()
{
}

double &Matrix4x4d::operator[](int i)
{
	assert(i >= 0 && i < 16);

	return m_elements[i];
}

Matrix4x4d Matrix4x4d::operator*(const Matrix4x4d &other) const
{
	Matrix4x4d product;

	for(int i = 0; i < 4; i++)
		for(int j = 0; j < 4; j++)
		{
			double sum = 0.0;

			// jth row of this by ith column of other
			for(int d = 0; d < 4; d++)
				sum += Get(d, j) * other.Get(i, d);

			product.Set(i, j, sum);
		}

	return product;
}

Matrix4x4d Matrix4x4d::operator*=(const Matrix4x4d &other)
{
	return (*this) = (*this) * other;
}

bool Matrix4x4d::operator==(const Matrix4x4d &other) const
{
	for(int i = 0; i < 4; i++)
		for(int j = 0; j < 4; j++)
		{
			if(Get(i, j) != other.Get(i, j))
				return false;
		}

	return true;
}

bool Matrix4x4d::operator!=(const Matrix4x4d &other) const
{
	return !((*this) == other);
}

void Matrix4x4d::Set(int i, int j, double val)
{
	assert(i >= 0 && j >= 0 && i < 4 && j < 4);

	m_elements[4 * i + j] = val; // Row-major would be i + 4 * j
}

double Matrix4x4d::Get(int i, int j) const
{
	assert(i >= 0 && j >= 0 && i < 4 && j < 4);

	return m_elements[4 * i + j]; // Row-major would be i + 4 * j
}

void Matrix4x4d::SetIdentity()
{
	m_elements[0] = 1.0; m_elements[4] = 0.0; m_elements[8] = 0.0; m_elements[12] = 0.0;
	m_elements[1] = 0.0; m_elements[5] = 1.0; m_elements[9] = 0.0; m_elements[13] = 0.0;
	m_elements[2] = 0.0; m_elements[6] = 0.0; m_elements[10] = 1.0; m_elements[14] = 0.0;
	m_elements[3] = 0.0; m_elements[7] = 0.0; m_elements[11] = 0.0; m_elements[15] = 1.0;
}

Matrix4x4d Matrix4x4d::Transpose() const
{
	Matrix4x4d transpose;

	for(int i = 0; i < 4; i++)
		for(int j = 0; j < 4; j++)
			transpose.Set(j, i, Get(i, j));

	return transpose;
}

void Matrix4x4d::GL_Load()
{
	glLoadMatrixd(m_elements);
}

void Matrix4x4d::GL_Set_Modelview()
{
	glGetDoublev(GL_MODELVIEW_MATRIX, m_elements);
}

void Matrix4x4d::GL_Set_Projection()
{
	glGetDoublev(GL_PROJECTION_MATRIX, m_elements);
}

Matrix4x4d Matrix4x4d::GL_Get_Modelview()
{
	Matrix4x4d m;
	glGetDoublev(GL_MODELVIEW_MATRIX, m.m_elements);

	return m;
}

Matrix4x4d Matrix4x4d::GL_Get_Projection()
{
	Matrix4x4d m;
	glGetDoublev(GL_PROJECTION_MATRIX, m.m_elements);

	return m;
}

void Matrix4x4d::GL_Mult()
{
	glMultMatrixd(m_elements);
}

Matrix4x4d Matrix4x4d::ScaleMatrix(const Vec3f &scale)
{
	Matrix4x4d scaleMatrix;

	scaleMatrix.SetIdentity();

	scaleMatrix.Set(0, 0, scale.x);
	scaleMatrix.Set(1, 1, scale.y);
	scaleMatrix.Set(2, 2, scale.z);

	return scaleMatrix;
}

Matrix4x4d Matrix4x4d::TranslateMatrix(const Vec3f &translation)
{
	Matrix4x4d translateMatrix;

	translateMatrix.SetIdentity();

	translateMatrix.Set(3, 0, translation.x);
	translateMatrix.Set(3, 1, translation.y);
	translateMatrix.Set(3, 2, translation.z);

	return translateMatrix;
}

Vec3f Matrix4x4d::operator*(const Vec3f &vec) const
{
	Vec3f result;
	
	result.x = static_cast<float>(m_elements[0] * vec.x + m_elements[4] * vec.y + m_elements[8] * vec.z + m_elements[12]);
	result.y = static_cast<float>(m_elements[1] * vec.x + m_elements[5] * vec.y + m_elements[9] * vec.z + m_elements[13]);
	result.z = static_cast<float>(m_elements[2] * vec.x + m_elements[6] * vec.y + m_elements[10] * vec.z + m_elements[14]);

	return result;
}

void Matrix4x4d::Print() const
{
	for(int j = 0; j < 4; j++)
	{
		for(int i = 0; i < 4; i++)
			std::cout << Get(i, j) << " ";

		std::cout << std::endl;
	}
}