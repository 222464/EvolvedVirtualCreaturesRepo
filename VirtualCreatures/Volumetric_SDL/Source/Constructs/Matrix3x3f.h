#pragma once

#include <Constructs/Vec3f.h>
#include <Constructs/Vec2f.h>
#include <vector>

/*
	Different from mathematics standard:

	i means column
	j means row
*/

// 1D array stores the matrix in column-major order, like OpenGL

class Matrix3x3f
{
public:
	static const float s_directionMatrixNormalizationTolerance;

	float m_elements[9];

	Matrix3x3f();
	Matrix3x3f(std::vector<float> sourceArray);

	float &operator[](int i);

	Matrix3x3f operator*(const Matrix3x3f &other) const;
	Matrix3x3f operator*=(const Matrix3x3f &other);

	bool operator==(const Matrix3x3f &other) const;
	bool operator!=(const Matrix3x3f &other) const;

	void Set(int i, int j, float val);
	float Get(int i, int j) const;

	void SetIdentity();
	Matrix3x3f Transpose() const;

	float Determinant() const;

	// Returns false if there is no inverse, true if there is one
	// Warning! Expensive function!
	bool Inverse(Matrix3x3f &inverse) const;

	// Transformation matrix generators
	static Matrix3x3f ScaleMatrix(const Vec2f &scale);
	static Matrix3x3f TranslateMatrix(const Vec2f translation);
	static Matrix3x3f RotateMatrix(float angle);
	static Matrix3x3f IdentityMatrix();

	Vec3f operator*(const Vec3f &vec) const;
	Vec2f operator*(const Vec2f &vec) const;

	void Print() const;
};