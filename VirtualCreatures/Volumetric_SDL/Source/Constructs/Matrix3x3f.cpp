#include <Constructs/Matrix3x3f.h>

#include <assert.h>

const float Matrix3x3f::s_directionMatrixNormalizationTolerance = 0.9999999f;

Matrix3x3f::Matrix3x3f()
{
}

float &Matrix3x3f::operator[](int i)
{
	assert(i >= 0 && i < 9);

	return m_elements[i];
}

Matrix3x3f Matrix3x3f::operator*(const Matrix3x3f &other) const
{
	Matrix3x3f product;

	for(int i = 0; i < 3; i++)
		for(int j = 0; j < 3; j++)
		{
			float sum = 0.0f;

			// jth row of this by ith column of other
			for(int d = 0; d < 3; d++)
				sum += Get(d, j) * other.Get(i, d);

			product.Set(i, j, sum);
		}

	return product;
}

Matrix3x3f Matrix3x3f::operator*=(const Matrix3x3f &other)
{
	return (*this) = (*this) * other;
}

bool Matrix3x3f::operator==(const Matrix3x3f &other) const
{
	for(int i = 0; i < 4; i++)
		for(int j = 0; j < 4; j++)
		{
			if(Get(i, j) != other.Get(i, j))
				return false;
		}

	return true;
}

bool Matrix3x3f::operator!=(const Matrix3x3f &other) const
{
	return !((*this) == other);
}

void Matrix3x3f::Set(int i, int j, float val)
{
	assert(i >= 0 && j >= 0 && i < 3 && j < 3);

	m_elements[3 * i + j] = val; // Row-major would be i + 4 * j
}

float Matrix3x3f::Get(int i, int j) const
{
	assert(i >= 0 && j >= 0 && i < 3 && j < 3);

	return m_elements[3 * i + j]; // Row-major would be i + 4 * j
}

void Matrix3x3f::SetIdentity()
{
	m_elements[0] = 1.0f; m_elements[3] = 0.0f; m_elements[6] = 0.0f;
	m_elements[1] = 0.0f; m_elements[4] = 1.0f; m_elements[7] = 0.0f;
	m_elements[2] = 0.0f; m_elements[5] = 0.0f; m_elements[8] = 1.0f;
}

Matrix3x3f Matrix3x3f::Transpose() const
{
	Matrix3x3f transpose;

	for(int i = 0; i < 3; i++)
		for(int j = 0; j < 3; j++)
			transpose.Set(j, i, Get(i, j));

	return transpose;
}

float Matrix3x3f::Determinant() const
{
	return m_elements[0] * m_elements[4] * m_elements[8] +
		m_elements[3] * m_elements[7] * m_elements[2] +
		m_elements[6] * m_elements[1] * m_elements[5] -
		m_elements[2] * m_elements[4] * m_elements[6] - 
		m_elements[5] * m_elements[7] * m_elements[0] -
		m_elements[8] * m_elements[2] * m_elements[3];
}

bool Matrix3x3f::Inverse(Matrix3x3f &inverse) const
{
	float det = Determinant();

	if(det == 0.0f)
		return false;

	float detInv = 1.0f / det;

	inverse.m_elements[0] = (m_elements[4] * m_elements[8] - m_elements[5] * m_elements[7]) * detInv;
	inverse.m_elements[1] = (m_elements[7] * m_elements[2] - m_elements[8] * m_elements[3]) * detInv;
	inverse.m_elements[2] = (m_elements[1] * m_elements[5] - m_elements[2] * m_elements[4]) * detInv;
	inverse.m_elements[3] = (m_elements[6] * m_elements[5] - m_elements[8] * m_elements[3]) * detInv;
	inverse.m_elements[4] = (m_elements[0] * m_elements[8] - m_elements[2] * m_elements[6]) * detInv;
	inverse.m_elements[5] = (m_elements[3] * m_elements[6] - m_elements[5] * m_elements[0]) * detInv;
	inverse.m_elements[6] = (m_elements[3] * m_elements[7] - m_elements[4] * m_elements[2]) * detInv;
	inverse.m_elements[7] = (m_elements[6] * m_elements[1] - m_elements[7] * m_elements[0]) * detInv;
	inverse.m_elements[8] = (m_elements[0] * m_elements[4] - m_elements[1] * m_elements[3]) * detInv;

	return true;
}

Matrix3x3f Matrix3x3f::ScaleMatrix(const Vec2f &scale)
{
	Matrix3x3f scaleMatrix;

	scaleMatrix.SetIdentity();

	scaleMatrix.Set(0, 0, scale.x);
	scaleMatrix.Set(1, 1, scale.y);

	return scaleMatrix;
}

Matrix3x3f Matrix3x3f::TranslateMatrix(const Vec2f translation)
{
	Matrix3x3f translateMatrix;

	translateMatrix.SetIdentity();

	translateMatrix.Set(2, 0, translation.x);
	translateMatrix.Set(2, 1, translation.y);

	return translateMatrix;
}

Matrix3x3f Matrix3x3f::RotateMatrix(float angle)
{
	float cosOfAngle = cosf(angle);
	float sinOfAngle = sinf(angle);

	Matrix3x3f rotationMatrix;

	rotationMatrix.SetIdentity();

	rotationMatrix.Set(0, 0, cosOfAngle);
	rotationMatrix.Set(1, 0, sinOfAngle);
	rotationMatrix.Set(0, 1, -sinOfAngle);
	rotationMatrix.Set(1, 0, cosOfAngle);

	return rotationMatrix;
}

Matrix3x3f Matrix3x3f::IdentityMatrix()
{
	Matrix3x3f identity;
	identity.SetIdentity();

	return identity;
}

Vec3f Matrix3x3f::operator*(const Vec3f &vec) const
{
	Vec3f result;
	
	result.x = m_elements[0] * vec.x + m_elements[3] * vec.y + m_elements[6] * vec.z;
	result.y = m_elements[1] * vec.x + m_elements[4] * vec.y + m_elements[7] * vec.z;
	result.z = m_elements[2] * vec.x + m_elements[5] * vec.y + m_elements[8] * vec.z;

	return result;
}

Vec2f Matrix3x3f::operator*(const Vec2f &vec) const
{
	Vec2f result;

	result.x = m_elements[0] * vec.x + m_elements[3] * vec.y + m_elements[6];
	result.y = m_elements[1] * vec.x + m_elements[4] * vec.y + m_elements[7];

	return result;
}

void Matrix3x3f::Print() const
{
	for(int j = 0; j < 3; j++)
	{
		for(int i = 0; i < 3; i++)
			std::cout << Get(i, j) << " ";

		std::cout << std::endl;
	}
}