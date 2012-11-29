#include <Constructs/Quaternion.h>

#include <Utilities/UtilFuncs.h>

const float Quaternion::s_quaternionNormalizationTolerance = 0.00001f;
const float Quaternion::s_quaternionDotTolerance = 0.9995f;

Quaternion::Quaternion()
	: w(0.0f), x(1.0f), y(0.0f), z(0.0f)
{
}

Quaternion::Quaternion(float W, float X, float Y, float Z)
	: w(W), x(X), y(Y), z(Z)
{
}

Quaternion::Quaternion(const Vec3f &axis, float angle)
{
	angle /= 2.0f;

	Vec3f normalizedAxis = axis.Normalize();
 
	float sinAngle = sinf(angle);
 
	x = (normalizedAxis.x * sinAngle);
	y = (normalizedAxis.y * sinAngle);
	z = (normalizedAxis.z * sinAngle);
	w = cosf(angle);
}

Quaternion::Quaternion(const Vec3f &eulerAngles)
{
	SetFromEulerAngles(eulerAngles);
}

void Quaternion::NormalizeThis()
{
	float mag2 = x * x + y * y + z * z + w * w;

	if(fabs(mag2) > s_quaternionNormalizationTolerance && fabs(mag2 - 1.0f) > s_quaternionNormalizationTolerance)
	{
		float mag = sqrtf(mag2);

		w /= mag;
		x /= mag;
		y /= mag;
		z /= mag;
	}
}

Quaternion Quaternion::Normalize() const
{
	float mag2 = x * x + y * y + z * z + w * w;

	if(fabs(mag2) > s_quaternionNormalizationTolerance && fabs(mag2 - 1.0f) > s_quaternionNormalizationTolerance)
	{
		float mag = sqrtf(mag2);

		return Quaternion(w / mag, x / mag, y / mag, z / mag);
	}

	return *this;
}

float Quaternion::MagnitudeSquared() const
{
	return x * x + y * y + z * z + w * w;
}

float Quaternion::Magnitude() const
{
	return sqrtf(x * x + y * y + z * z + w * w);
}

Vec4f Quaternion::GetVec4f() const
{
	return Vec4f(x, y, z, w);
}

float Quaternion::Dot(const Quaternion &other) const
{
	return x * other.x + y * other.y + z * other.z + w * other.w;
}

Quaternion Quaternion::operator+(const Quaternion &other) const
{
	return Quaternion(w + other.w, x + other.x, y + other.y, z + other.z);
}

Quaternion Quaternion::operator-(const Quaternion &other) const
{
	return Quaternion(w - other.w, x - other.x, y - other.y, z - other.z);
}

Quaternion Quaternion::operator*(float scalar) const
{
	return Quaternion(w * scalar, x * scalar, y * scalar, z * scalar);
}

Quaternion Quaternion::operator/(float scalar) const
{
	return Quaternion(w / scalar, x / scalar, y / scalar, z / scalar);
}

Quaternion Quaternion::operator*(const Quaternion &other) const
{
	return Quaternion(w * other.w - x * other.x - y * other.y - z * other.z,
		w * other.x + x * other.w + y * other.z - z * other.y,
		w * other.y - x * other.z + y * other.w + z * other.x,
		w * other.z + x * other.y - y * other.x + z * other.w);
}

const Quaternion &Quaternion::operator*=(const Quaternion &other)
{
	return *this = *this * other;
}

const Quaternion &Quaternion::operator*=(float scalar)
{
	return *this = *this * scalar;
}

const Quaternion &Quaternion::operator/=(float scalar)
{
	return *this = *this / scalar;
}

void Quaternion::Rotate(float angle, const Vec3f &axis)
{
	float halfAngle = angle / 2.0f;
	w = cosf(halfAngle);
	x = axis.x * sinf(halfAngle);
	y = axis.y * sinf(halfAngle);
	z = axis.z * sinf(halfAngle);
}

Quaternion Quaternion::GetRotated(float angle, const Vec3f &axis)
{
	Quaternion quat;
	quat.Rotate(angle, axis);

	return quat;
}

Quaternion Quaternion::Conjugate() const
{
	Quaternion conjugate(*this);
	conjugate.x = -conjugate.x;
	conjugate.y = -conjugate.y;
	conjugate.z = -conjugate.z;

	return conjugate;
}

Quaternion Quaternion::Inverse() const
{
	return Conjugate() / MagnitudeSquared();
}

void Quaternion::Reset()
{
	w = 0.0f;
	x = 1.0f;
	y = 0.0f;
	z = 0.0f;
}

void Quaternion::MulMatrix()
{
	GetMatrix().GL_Mult();
}

Matrix4x4f Quaternion::GetMatrix() const
{
	Matrix4x4f matrix;

	float x2 = x * x;
	float y2 = y * y;
	float z2 = z * z;
	float xy = x * y;
	float xz = x * z;
	float yz = y * z;
	float wx = w * x;
	float wy = w * y;
	float wz = w * z;

	matrix[0] = 1.0f - 2.0f * (y2 + z2);
	matrix[1] = 2.0f * (xy - wz);
	matrix[2] = 2.0f * (xz + wy);
	matrix[3] = 0.0f;
	matrix[4] = 2.0f * (xy + wz);
	matrix[5] = 1.0f - 2.0f * (x2 + z2);
	matrix[6] = 2.0f * (yz - wx);
	matrix[7] = 0.0f;
	matrix[8] = 2.0f * (xz - wy);
	matrix[9] = 2.0f * (yz + wx);
	matrix[10] = 1.0f - 2.0f * (x2 + y2);
	matrix[11] = 0.0f;
	matrix[12] = 0.0f;
	matrix[13] = 0.0f;
	matrix[14] = 0.0f;
	matrix[15] = 1.0f;

	return matrix;
}

void Quaternion::SetFromEulerAngles(const Vec3f &eulerAngles)
{
	// Could also just multiply quaternions rotated along axes, can better optimize like this though
	float cx = cosf(eulerAngles.x / 2.0f);
	float cy = cosf(eulerAngles.y / 2.0f);
	float cz = cosf(eulerAngles.z / 2.0f);

	float sx = sinf(eulerAngles.x / 2.0f);
	float sy = sinf(eulerAngles.y / 2.0f);
	float sz = sinf(eulerAngles.z / 2.0f);

	w = cx * cy * cz - sx * sy * sz;
	x = cx * sy * sz + sx * cy * cz;
	y = cx * sy * cz + sx * cy * sz;
	z = cx * cy * sz - sx * sy * cz;
}

Vec3f Quaternion::GetEulerAngles() const
{
	float x2 = x * x;
	float y2 = y * y;
	float z2 = z * z;
	float w2 = w * w;

	float unitLength = x2 + y2 + z2 + w2;

	float abcd = w * x + y * z;

	if(abcd > (0.5f - s_quaternionNormalizationTolerance) * unitLength)
		return Vec3f(2.0f * atan2f(y, w), pif, 0.0f);
	else if(abcd < (-0.5f + s_quaternionNormalizationTolerance) * unitLength)
		return Vec3f(-2.0f * atan2f(y, w), -pif, 0.0f);
	else
	{
		float adbc = w * z - x * y;
		float acbd = w * y - x * z;

		return Vec3f(atan2f(2.0f * adbc, 1.0f - 2.0f * (z2 + x2)), asinf(2.0f * abcd / unitLength), atan2f(2.0f * acbd, 1.0f - 2.0f * (y2 + x2)));
	}
}

void Quaternion::CalculateWFromXYZ()
{
	float t = 1.0f - x * x - y * y - z * z;

	if(t < 0.0f)
		w = 0.0f;
	else
		w = -sqrtf(t);
}

Quaternion Quaternion::Lerp(const Quaternion &first, const Quaternion &second, float interpolationCoefficient)
{
	Vec4f firstVec(first.GetVec4f());
	Vec4f secondVec(second.GetVec4f());
	Vec4f interp(firstVec + (secondVec - firstVec) * interpolationCoefficient);
	interp.NormalizeThis();

	return Quaternion(interp.w, interp.x, interp.y, interp.z);
}

Quaternion Quaternion::Slerp(const Quaternion &first, const Quaternion &second, float interpolationCoefficient)
{
	float dot = first.Dot(second);

	if(dot > s_quaternionDotTolerance)
		return Lerp(first, second, interpolationCoefficient);

	dot = Clamp(dot, -1.0f, 1.0f);

	float theta = acosf(dot) * interpolationCoefficient;

	Quaternion third(second - first * dot);

	third.NormalizeThis();

	return first * cosf(theta) + third * sinf(theta);
}

Vec3f operator*(const Quaternion &quat, const Vec3f &vec)
{
	Vec3f quatVec(quat.x, quat.y, quat.z);
	Vec3f uv(quatVec.Cross(vec));
	Vec3f uuv(quatVec.Cross(uv));
	uv *= 2.0f * quat.w;
	uuv *= 2.0f;

	return vec + uv + uuv;
}

Vec3f operator*(const Vec3f &vec, const Quaternion &quat)
{
	return quat.Inverse() * vec;
}

std::ostream &operator<<(std::ostream &output, const Quaternion &quat)
{
	std::cout << "(" << quat.x << ", " << quat.y << ", " << quat.z << ", " << quat.w << ")";

	return output;
}