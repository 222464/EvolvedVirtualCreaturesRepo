#pragma once

#include <AssetManager/Asset_Texture.h>

#include <Renderer/Model_MD5/Vertex.h>
#include <Renderer/Model_MD5/Weight.h>

#include <Renderer/BufferObjects/VBO.h>

#include <vector>

typedef std::vector<unsigned short> Triangle;

struct BoneWeightSet
{
	float m_weights[4];
};

struct BoneIndexSet
{
	float m_indices[4];
};

struct Mesh
{
	unsigned int m_materialIndex; // Index of material in Model_MD5 material array

	std::vector<Vertex> m_vertices;
	std::vector<Triangle> m_triangles;
	std::vector<Weight> m_weights;

	// Buffers (not VBO)
	std::vector<Vec3f> m_positions;
	std::vector<Vec3f> m_normals;

	std::vector<Vec2f> m_texCoords;
	std::vector<unsigned short> m_indices;

	// For GPU Skinning
	std::vector<BoneWeightSet> m_boneWeights;
	std::vector<BoneIndexSet> m_boneIndices;

    VBO m_positionBuffer;
    VBO m_normalBuffer;
    VBO m_boneWeightBuffer;
    VBO m_boneIndexBuffer;
    VBO m_texCoordBuffer;
    VBO m_indexBuffer;     

	Mesh();
};

