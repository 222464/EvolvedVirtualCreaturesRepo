#include <Constructs/Matrix4x4f.h>

#include <Renderer/SDL_OpenGL.h>

#include <assert.h>

const float Matrix4x4f::s_directionMatrixNormalizationTolerance = 0.9999999f;

Matrix4x4f::Matrix4x4f()
{
}

float &Matrix4x4f::operator[](int i)
{
	assert(i >= 0 && i < 16);

	return m_elements[i];
}

Matrix4x4f Matrix4x4f::operator*(const Matrix4x4f &other) const
{
	Matrix4x4f product;

	for(int i = 0; i < 4; i++)
		for(int j = 0; j < 4; j++)
		{
			float sum = 0.0f;

			// jth row of this by ith column of other
			for(int d = 0; d < 4; d++)
				sum += Get(d, j) * other.Get(i, d);

			product.Set(i, j, sum);
		}

	return product;
}

Matrix4x4f Matrix4x4f::operator*=(const Matrix4x4f &other)
{
	return (*this) = (*this) * other;
}

bool Matrix4x4f::operator==(const Matrix4x4f &other) const
{
	for(int i = 0; i < 4; i++)
		for(int j = 0; j < 4; j++)
		{
			if(Get(i, j) != other.Get(i, j))
				return false;
		}

	return true;
}

bool Matrix4x4f::operator!=(const Matrix4x4f &other) const
{
	return !((*this) == other);
}

void Matrix4x4f::Set(int i, int j, float val)
{
	assert(i >= 0 && j >= 0 && i < 4 && j < 4);

	m_elements[4 * i + j] = val; // Row-major would be i + 4 * j
}

float Matrix4x4f::Get(int i, int j) const
{
	assert(i >= 0 && j >= 0 && i < 4 && j < 4);

	return m_elements[4 * i + j]; // Row-major would be i + 4 * j
}

void Matrix4x4f::SetIdentity()
{
	m_elements[0] = 1.0f; m_elements[4] = 0.0f; m_elements[8] = 0.0f; m_elements[12] = 0.0f;
	m_elements[1] = 0.0f; m_elements[5] = 1.0f; m_elements[9] = 0.0f; m_elements[13] = 0.0f;
	m_elements[2] = 0.0f; m_elements[6] = 0.0f; m_elements[10] = 1.0f; m_elements[14] = 0.0f;
	m_elements[3] = 0.0f; m_elements[7] = 0.0f; m_elements[11] = 0.0f; m_elements[15] = 1.0f;
}

Matrix4x4f Matrix4x4f::Transpose() const
{
	Matrix4x4f transpose;

	for(int i = 0; i < 4; i++)
		for(int j = 0; j < 4; j++)
			transpose.Set(j, i, Get(i, j));

	return transpose;
}

bool Matrix4x4f::Inverse(Matrix4x4f &inverse) const
{
	inverse.m_elements[0] = m_elements[5] * m_elements[10] * m_elements[15] -
		m_elements[5] * m_elements[11] * m_elements[14] -
		m_elements[9] * m_elements[6] * m_elements[15] +
        m_elements[9] * m_elements[7] * m_elements[14] +
        m_elements[13] * m_elements[6] * m_elements[11] -
        m_elements[13] * m_elements[7] * m_elements[10];

	inverse.m_elements[4] = -m_elements[4] * m_elements[10] * m_elements[15] +
		m_elements[4] * m_elements[11] * m_elements[14] +
		m_elements[8] * m_elements[6] * m_elements[15] -
		m_elements[8] * m_elements[7] * m_elements[14] -
		m_elements[12] * m_elements[6] * m_elements[11] +
		m_elements[12] * m_elements[7] * m_elements[10];

	inverse.m_elements[8] = m_elements[4] * m_elements[9] * m_elements[15] -
		m_elements[4] * m_elements[11] * m_elements[13] -
		m_elements[8] * m_elements[5] * m_elements[15] +
		m_elements[8] * m_elements[7] * m_elements[13] +
		m_elements[12] * m_elements[5] * m_elements[11] -
		m_elements[12] * m_elements[7] * m_elements[9];

	inverse.m_elements[12] = -m_elements[4] * m_elements[9] * m_elements[14] +
		m_elements[4] * m_elements[10] * m_elements[13] +
		m_elements[8] * m_elements[5] * m_elements[14] -
		m_elements[8] * m_elements[6] * m_elements[13] -
		m_elements[12] * m_elements[5] * m_elements[10] +
		m_elements[12] * m_elements[6] * m_elements[9];

	inverse.m_elements[1] = -m_elements[1] * m_elements[10] * m_elements[15] +
		m_elements[1] * m_elements[11] * m_elements[14] +
		m_elements[9] * m_elements[2] * m_elements[15] -
		m_elements[9] * m_elements[3] * m_elements[14] -
		m_elements[13] * m_elements[2] * m_elements[11] + 
		m_elements[13] * m_elements[3] * m_elements[10];

	inverse.m_elements[5] = m_elements[0] * m_elements[10] * m_elements[15] -
		m_elements[0] * m_elements[11] * m_elements[14] -
		m_elements[8] * m_elements[2] * m_elements[15] +
		m_elements[8] * m_elements[3] * m_elements[14] +
		m_elements[12] * m_elements[2] * m_elements[11] -
		m_elements[12] * m_elements[3] * m_elements[10];

	inverse.m_elements[9] = -m_elements[0] * m_elements[9] * m_elements[15] +
		m_elements[0] * m_elements[11] * m_elements[13] +
		m_elements[8] * m_elements[1] * m_elements[15] -
		m_elements[8] * m_elements[3] * m_elements[13] -
		m_elements[12] * m_elements[1] * m_elements[11] +
		m_elements[12] * m_elements[3] * m_elements[9];

	inverse.m_elements[13] = m_elements[0] * m_elements[9] * m_elements[14] -
		m_elements[0] * m_elements[10] * m_elements[13] -
		m_elements[8] * m_elements[1] * m_elements[14] +
		m_elements[8] * m_elements[2] * m_elements[13] +
		m_elements[12] * m_elements[1] * m_elements[10] -
		m_elements[12] * m_elements[2] * m_elements[9];

	inverse.m_elements[2] = m_elements[1] * m_elements[6] * m_elements[15] -
		m_elements[1] * m_elements[7] * m_elements[14] -
		m_elements[5] * m_elements[2] * m_elements[15] +
		m_elements[5] * m_elements[3] * m_elements[14] +
		m_elements[13] * m_elements[2] * m_elements[7] -
		m_elements[13] * m_elements[3] * m_elements[6];

	inverse.m_elements[6] = -m_elements[0] * m_elements[6] * m_elements[15] +
		m_elements[0] * m_elements[7] * m_elements[14] +
		m_elements[4] * m_elements[2] * m_elements[15] -
		m_elements[4] * m_elements[3] * m_elements[14] -
		m_elements[12] * m_elements[2] * m_elements[7] +
		m_elements[12] * m_elements[3] * m_elements[6];

	inverse.m_elements[10] = m_elements[0] * m_elements[5] * m_elements[15] -
		m_elements[0] * m_elements[7] * m_elements[13] -
		m_elements[4] * m_elements[1] * m_elements[15] +
		m_elements[4] * m_elements[3] * m_elements[13] +
		m_elements[12] * m_elements[1] * m_elements[7] -
		m_elements[12] * m_elements[3] * m_elements[5];

	inverse.m_elements[14] = -m_elements[0] * m_elements[5] * m_elements[14] +
		m_elements[0] * m_elements[6] * m_elements[13] +
		m_elements[4] * m_elements[1] * m_elements[14] -
		m_elements[4] * m_elements[2] * m_elements[13] -
		m_elements[12] * m_elements[1] * m_elements[6] +
		m_elements[12] * m_elements[2] * m_elements[5];

	inverse.m_elements[3] = -m_elements[1] * m_elements[6] * m_elements[11] +
		m_elements[1] * m_elements[7] * m_elements[10] +
		m_elements[5] * m_elements[2] * m_elements[11] -
		m_elements[5] * m_elements[3] * m_elements[10] -
		m_elements[9] * m_elements[2] * m_elements[7] +
		m_elements[9] * m_elements[3] * m_elements[6];

	inverse.m_elements[7] = m_elements[0] * m_elements[6] * m_elements[11] -
		m_elements[0] * m_elements[7] * m_elements[10] -
		m_elements[4] * m_elements[2] * m_elements[11] +
		m_elements[4] * m_elements[3] * m_elements[10] +
		m_elements[8] * m_elements[2] * m_elements[7] -
		m_elements[8] * m_elements[3] * m_elements[6];

	inverse.m_elements[11] = -m_elements[0] * m_elements[5] * m_elements[11] +
		m_elements[0] * m_elements[7] * m_elements[9] +
		m_elements[4] * m_elements[1] * m_elements[11] -
		m_elements[4] * m_elements[3] * m_elements[9] -
		m_elements[8] * m_elements[1] * m_elements[7] +
		m_elements[8] * m_elements[3] * m_elements[5];

	inverse.m_elements[15] = m_elements[0] * m_elements[5] * m_elements[10] -
		m_elements[0] * m_elements[6] * m_elements[9] -
		m_elements[4] * m_elements[1] * m_elements[10] +
		m_elements[4] * m_elements[2] * m_elements[9] +
		m_elements[8] * m_elements[1] * m_elements[6] -
		m_elements[8] * m_elements[2] * m_elements[5];

	// Easier to calculate determinant using inverse matrix so far
	float det = m_elements[0] * inverse.m_elements[0] +
		m_elements[1] * inverse.m_elements[4] +
		m_elements[2] * inverse.m_elements[8] +
		m_elements[3] * inverse.m_elements[12];

	if(det == 0.0f)
		return false;

	det = 1.0f / det;

	for(int i = 0; i < 16; i++)
		inverse.m_elements[i] *= det;

	return true;
}

void Matrix4x4f::GL_Load()
{
	glLoadMatrixf(m_elements);
}

void Matrix4x4f::GL_Set_Modelview()
{
	glGetFloatv(GL_MODELVIEW_MATRIX, m_elements);
}

void Matrix4x4f::GL_Set_Projection()
{
	glGetFloatv(GL_PROJECTION_MATRIX, m_elements);
}

Matrix4x4f Matrix4x4f::GL_Get_Modelview()
{
	Matrix4x4f m;
	glGetFloatv(GL_MODELVIEW_MATRIX, m.m_elements);

	return m;
}

Matrix4x4f Matrix4x4f::GL_Get_Projection()
{
	Matrix4x4f m;
	glGetFloatv(GL_PROJECTION_MATRIX, m.m_elements);

	return m;
}

void Matrix4x4f::GL_Mult()
{
	glMultMatrixf(m_elements);
}

Matrix4x4f Matrix4x4f::ScaleMatrix(const Vec3f &scale)
{
	Matrix4x4f scaleMatrix;

	scaleMatrix.SetIdentity();

	scaleMatrix.Set(0, 0, scale.x);
	scaleMatrix.Set(1, 1, scale.y);
	scaleMatrix.Set(2, 2, scale.z);

	return scaleMatrix;
}

Matrix4x4f Matrix4x4f::TranslateMatrix(const Vec3f translation)
{
	Matrix4x4f translateMatrix;

	translateMatrix.SetIdentity();

	translateMatrix.Set(3, 0, translation.x);
	translateMatrix.Set(3, 1, translation.y);
	translateMatrix.Set(3, 2, translation.z);

	return translateMatrix;
}

Matrix4x4f Matrix4x4f::RotateMatrix_X(float angle)
{
	float cosOfAngle = cosf(angle);
	float sinOfAngle = sinf(angle);

	Matrix4x4f rotationMatrix;

	rotationMatrix.SetIdentity();

	rotationMatrix.Set(1, 1, cosOfAngle);
	rotationMatrix.Set(2, 1, sinOfAngle);
	rotationMatrix.Set(1, 2, -sinOfAngle);
	rotationMatrix.Set(2, 2, cosOfAngle);

	return rotationMatrix;
}

Matrix4x4f Matrix4x4f::RotateMatrix_Y(float angle)
{
	float cosOfAngle = cosf(angle);
	float sinOfAngle = sinf(angle);

	Matrix4x4f rotationMatrix;

	rotationMatrix.SetIdentity();

	rotationMatrix.Set(0, 0, cosOfAngle);
	rotationMatrix.Set(2, 0, -sinOfAngle);
	rotationMatrix.Set(0, 2, sinOfAngle);
	rotationMatrix.Set(2, 2, cosOfAngle);

	return rotationMatrix;
}

Matrix4x4f Matrix4x4f::RotateMatrix_Z(float angle)
{
	float cosOfAngle = cosf(angle);
	float sinOfAngle = sinf(angle);

	Matrix4x4f rotationMatrix;

	rotationMatrix.SetIdentity();

	rotationMatrix.Set(0, 0, cosOfAngle);
	rotationMatrix.Set(1, 0, sinOfAngle);
	rotationMatrix.Set(0, 1, -sinOfAngle);
	rotationMatrix.Set(1, 1, cosOfAngle);

	return rotationMatrix;
}

Matrix4x4f Matrix4x4f::RotateMatrix(const Vec3f &eulerAngles)
{
	return RotateMatrix_X(eulerAngles.x) * RotateMatrix_Y(eulerAngles.y) * RotateMatrix_Z(eulerAngles.z);
}

Matrix4x4f Matrix4x4f::DirectionMatrix(const Vec3f &direction, const Vec3f &up)
{
	Vec3f tangent_0 = direction.Cross(up);

	tangent_0.NormalizeThis();

	Vec3f tangent_1(direction.Cross(tangent_0).Normalize());

	Matrix4x4f mat;

	mat.m_elements[0] = direction.x;
	mat.m_elements[1] = direction.y;
	mat.m_elements[2] = direction.z;
	mat.m_elements[3] = 0.0f;

	mat.m_elements[4] = tangent_1.x;
	mat.m_elements[5] = tangent_1.y;
	mat.m_elements[6] = tangent_1.z;
	mat.m_elements[7] = 0.0f;

	mat.m_elements[8] = tangent_0.x;
	mat.m_elements[9] = tangent_0.y;
	mat.m_elements[10] = tangent_0.z;
	mat.m_elements[11] = 0.0f;

	mat.m_elements[12] = 0.0f;
	mat.m_elements[13] = 0.0f;
	mat.m_elements[14] = 0.0f;
	mat.m_elements[15] = 1.0f;

	return mat;
}

Matrix4x4f Matrix4x4f::DirectionMatrix_AutoUp(const Vec3f &direction)
{
	Vec3f tangent_0 = direction.Cross(Vec3f(1.0f, 0.0f, 0.0f)); // Try x as up vector

	if(tangent_0.MagnitudeSquared() < 0.00001f) // x up vector didn't work (the direction is too close along x axis, cannot cross parallel vecs), try another
		tangent_0 = direction.Cross(Vec3f(0.0f, 1.0f, 0.0f));

	tangent_0.NormalizeThis();

	Vec3f tangent_1(direction.Cross(tangent_0).Normalize());

	Matrix4x4f mat;

	mat.m_elements[0] = direction.x;
	mat.m_elements[1] = direction.y;
	mat.m_elements[2] = direction.z;
	mat.m_elements[3] = 0.0f;

	mat.m_elements[4] = tangent_0.x;
	mat.m_elements[5] = tangent_0.y;
	mat.m_elements[6] = tangent_0.z;
	mat.m_elements[7] = 0.0f;

	mat.m_elements[8] = tangent_1.x;
	mat.m_elements[9] = tangent_1.y;
	mat.m_elements[10] = tangent_1.z;
	mat.m_elements[11] = 0.0f;

	mat.m_elements[12] = 0.0f;
	mat.m_elements[13] = 0.0f;
	mat.m_elements[14] = 0.0f;
	mat.m_elements[15] = 1.0f;
	
	return mat;
}

Matrix4x4f Matrix4x4f::CameraDirectionMatrix(const Vec3f &direction, const Vec3f &up)
{
	Vec3f tangent_0 = direction.Cross(up);

	tangent_0.NormalizeThis();

	Vec3f tangent_1(tangent_0.Cross(direction).Normalize());

	Matrix4x4f mat;

	mat.m_elements[0] = tangent_0.x;
	mat.m_elements[4] = tangent_0.y;
	mat.m_elements[8] = tangent_0.z;
	mat.m_elements[12] = 0.0f;

	mat.m_elements[1] = tangent_1.x;
	mat.m_elements[5] = tangent_1.y;
	mat.m_elements[9] = tangent_1.z;
	mat.m_elements[13] = 0.0f;

	mat.m_elements[2] = -direction.x;
	mat.m_elements[6] = -direction.y;
	mat.m_elements[10] = -direction.z;
	mat.m_elements[14] = 0.0f;

	mat.m_elements[3] = 0.0f;
	mat.m_elements[7] = 0.0f;
	mat.m_elements[11] = 0.0f;
	mat.m_elements[15] = 1.0f;

	return mat;
}

Matrix4x4f Matrix4x4f::CameraDirectionMatrix_AutoUp(const Vec3f &direction)
{
	Vec3f tangent_0 = direction.Cross(Vec3f(1.0f, 0.0f, 0.0f)); // Try x as up vector
	
	if(tangent_0.MagnitudeSquared() < 0.001f) // x up vector didn't work (the direction is too close along x axis, cannot cross parallel vecs), try another
		tangent_0 = direction.Cross(Vec3f(0.0f, 1.0f, 0.0f));

	tangent_0.NormalizeThis();

	Vec3f tangent_1(tangent_0.Cross(direction).Normalize());

	Matrix4x4f mat;

	mat.m_elements[0] = tangent_0.x;
	mat.m_elements[4] = tangent_0.y;
	mat.m_elements[8] = tangent_0.z;
	mat.m_elements[12] = 0.0f;

	mat.m_elements[1] = tangent_1.x;
	mat.m_elements[5] = tangent_1.y;
	mat.m_elements[9] = tangent_1.z;
	mat.m_elements[13] = 0.0f;

	mat.m_elements[2] = -direction.x;
	mat.m_elements[6] = -direction.y;
	mat.m_elements[10] = -direction.z;
	mat.m_elements[14] = 0.0f;

	mat.m_elements[3] = 0.0f;
	mat.m_elements[7] = 0.0f;
	mat.m_elements[11] = 0.0f;
	mat.m_elements[15] = 1.0f;

	return mat;
}

Matrix4x4f Matrix4x4f::IdentityMatrix()
{
	Matrix4x4f identity;
	identity.SetIdentity();

	return identity;
}

Matrix4x4f Matrix4x4f::PerspectiveMatrix(float fovY, float aspectRatio, float zNear, float zFar)
{
	float f = 1.0f / tanf(fovY);

	float nearMinusFar = zNear - zFar;

	Matrix4x4f mat;

	mat.m_elements[0] = f / aspectRatio;
	mat.m_elements[4] = 0.0f;
	mat.m_elements[8] = 0.0f;
	mat.m_elements[12] = 0.0f;

	mat.m_elements[1] = 0.0f;
	mat.m_elements[5] = f;
	mat.m_elements[9] = 0.0f;
	mat.m_elements[13] = 0.0f;

	mat.m_elements[2] = 0.0f;
	mat.m_elements[6] = 0.0f;
	mat.m_elements[10] = (zNear + zFar) / nearMinusFar;
	mat.m_elements[14] = (2.0f * zNear * zFar) / nearMinusFar;

	mat.m_elements[3] = 0.0f;
	mat.m_elements[7] = 0.0f;
	mat.m_elements[11] = -1.0f;
	mat.m_elements[15] = 0.0f;

	return mat;
}

Matrix4x4f Matrix4x4f::OrthoMatrix(float left, float right, float bottom, float top, float zNear, float zFar)
{
	float rightMinusLeft = right - left;
	float topMinusBottom = top - bottom;
	float farMinusNear = zFar - zNear;

	Matrix4x4f mat;

	mat.m_elements[0] = 2.0f / rightMinusLeft;
	mat.m_elements[1] = 0.0f;
	mat.m_elements[2] = 0.0f;
	mat.m_elements[3] = 0.0f;
	mat.m_elements[4] = 0.0f;
	mat.m_elements[5] = 2.0f / topMinusBottom;
	mat.m_elements[6] = 0.0f;
	mat.m_elements[7] = 0.0f;
	mat.m_elements[8] = 0.0f;
	mat.m_elements[9] = 0.0f;
	mat.m_elements[10] = -2.0f / farMinusNear;
	mat.m_elements[11] = 0.0f;
	mat.m_elements[12] = -(right + left) / rightMinusLeft;
	mat.m_elements[13] = -(top + bottom) / topMinusBottom;
	mat.m_elements[14] = -(zFar + zNear) / farMinusNear;
	mat.m_elements[15] = 1.0f;

	return mat;
}

Vec4f Matrix4x4f::operator*(const Vec4f &vec) const
{
	Vec4f result;
	
	result.x = m_elements[0] * vec.x + m_elements[4] * vec.y + m_elements[8] * vec.z + m_elements[12];
	result.y = m_elements[1] * vec.x + m_elements[5] * vec.y + m_elements[9] * vec.z + m_elements[13];
	result.z = m_elements[2] * vec.x + m_elements[6] * vec.y + m_elements[10] * vec.z + m_elements[14];
	result.w = m_elements[3] * vec.x + m_elements[7] * vec.y + m_elements[11] * vec.z + m_elements[15];

	return result;
}

Vec3f Matrix4x4f::operator*(const Vec3f &vec) const
{
	Vec3f result;

	result.x = m_elements[0] * vec.x + m_elements[4] * vec.y + m_elements[8] * vec.z + m_elements[12];
	result.y = m_elements[1] * vec.x + m_elements[5] * vec.y + m_elements[9] * vec.z + m_elements[13];
	result.z = m_elements[2] * vec.x + m_elements[6] * vec.y + m_elements[10] * vec.z + m_elements[14];

	return result;
}

void Matrix4x4f::Print() const
{
	for(int j = 0; j < 4; j++)
	{
		for(int i = 0; i < 4; i++)
			std::cout << Get(i, j) << " ";

		std::cout << std::endl;
	}
}