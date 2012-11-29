#pragma once

#include <Constructs/Vec3f.h>
#include <vector>

/*
	Different from mathematics standard:

	i means column
	j means row
*/

// 1D array stores the matrix in column-major order, like OpenGL

class Matrix4x4d
{
public:
	double m_elements[16];

	Matrix4x4d();
	Matrix4x4d(std::vector<double> sourceArray);

	double &operator[](int i);

	Matrix4x4d operator*(const Matrix4x4d &other) const;
	Matrix4x4d operator*=(const Matrix4x4d &other);

	bool operator==(const Matrix4x4d &other) const;
	bool operator!=(const Matrix4x4d &other) const;

	void Set(int i, int j, double val);
	double Get(int i, int j) const;

	void SetIdentity();
	Matrix4x4d Transpose() const;

	// For use with OpenGL
	void GL_Load();
	void GL_Set_Modelview();
	void GL_Set_Projection();
	static Matrix4x4d GL_Get_Modelview();
	static Matrix4x4d GL_Get_Projection();
	void GL_Mult();

	// Transformation matrix generators
	static Matrix4x4d ScaleMatrix(const Vec3f &scale);
	static Matrix4x4d TranslateMatrix(const Vec3f &translation);

	Vec3f operator*(const Vec3f &vec) const;

	void Print() const;
};
