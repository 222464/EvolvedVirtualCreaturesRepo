#pragma once

#include <AssetManager/AssetManager.h>
#include <AssetManager/Asset.h>

#include <Renderer/SDL_OpenGL.h>
#include <Renderer/BufferObjects/VBO.h>
#include <AssetManager/Asset_Texture.h>

#include <Constructs/AABB.h>
#include <constructs/Color3f.h>

#include <Scene/Scene.h>

struct IndexSet
{
	unsigned short vi, ti, ni;

	// Custom hash stuff - if this index set already exists, so must a vertex for it
	size_t operator()(const IndexSet &set) const;
	bool operator==(const IndexSet &other) const;
};

class Model_OBJ :
	public Asset
{
public:
	enum ShaderCombination
	{
		e_plainOnly, e_bumpOnly, e_both
	};

	struct Material
	{
		Color3f m_diffuseColor;
		float m_specularColor;
		Color3f m_emissiveColor;

		Asset_Texture* m_pDiffuseMap;
		Asset_Texture* m_pSpecularMap;
		Asset_Texture* m_pNormalMap;
		Asset_Texture* m_pEmissiveMap;

		Scene::GBufferRenderShader m_shader;

		Material()
			: m_diffuseColor(1.0f, 1.0f, 1.0f), m_specularColor(0.0f), m_emissiveColor(0.0f, 0.0f, 0.0f),
			m_pDiffuseMap(NULL), m_pSpecularMap(NULL), m_pNormalMap(NULL), m_pEmissiveMap(NULL),
			m_shader(Scene::e_plain)
		{
		}
	};

private:
	ShaderCombination m_shaderCombination;

protected:
	VBO m_positionBuffer;
	VBO m_texCoordBuffer;
	VBO m_normalBuffer;

	// Array of VBOs for different OBJ objects
	std::vector<VBO> m_indexBuffers;

	// Number of vertices for each OBJ object
	std::vector<unsigned short> m_numVertices;
	
	std::vector<Material> m_materials;

	// Array of material indices for different OBJ objects
	std::vector<unsigned int> m_objMaterialReferences;

	AssetManager m_textureManager;

	bool m_usingMTL;
	bool m_loaded;

	AABB m_aabb;
	Vec3f m_aabbOffsetFromModel;

	bool LoadMaterialLibrary(const std::string &name, std::unordered_map<std::string, unsigned int> &matReferences);

	void BatchRender_BindBuffers();
	void BatchRender_SetSubObjectStates(unsigned int subObjectIndex); // For rendering 1 sub object at a time for all instances
	void BatchRender_RenderSubObject(unsigned int subObjectIndex);
	void BatchRender_SetSubObjectShader(unsigned int subObjectIndex);
	void BatchRender_UnbindBuffers();
	void BatchRender_RenderAllSubObjects_NoShaderSwitch();
	void BatchRender_RenderAllSubObjects_ShaderSwitch();

	Scene* m_pScene;
	class Model_OBJ_BatchRenderer* m_pBatchRenderer;

public:
	Model_OBJ();
	virtual ~Model_OBJ() {}

	// Inherited from Asset
	bool LoadAsset(const std::string &name);

	void SetRenderer(Scene* pScene);

	void Render(const Matrix4x4f &transform);
	void Render(const Matrix4x4f &transform, Scene* pScene);

	bool Loaded() const;

	unsigned int GetNumObjects() const;

	unsigned int GetNumMaterials() const;
	Material* GetMaterial(unsigned int index);

	void FindShaderCombination();

	AABB GetAABB() const;

	const Vec3f &GetAABBOffsetFromModel() const;

	ShaderCombination GetShaderCombination() const;

	// Asset factory
	static Asset* Asset_Factory();

	friend class Model_OBJ_BatchRenderer;
};

class Model_OBJ_BatchRenderer :
	public BatchRenderer
{
private:
	// 3 categories, each category sorts by instances (with hash maps)
	std::unordered_map<Model_OBJ*, std::vector<Matrix4x4f>> m_subObjects_mutlipleShaders;
	std::unordered_map<Model_OBJ*, std::vector<Matrix4x4f>> m_subObjects_plainShaderOnly;
	std::unordered_map<Model_OBJ*, std::vector<Matrix4x4f>> m_subObjects_bumpMappingShaderOnly;

public:
	void Add(Model_OBJ* pModel, const Matrix4x4f &transform);

	// Inherited from BatchRenderer
	void Execute();
	void Clear();

	static BatchRenderer* Model_OBJ_BatchRendererFactory();
};