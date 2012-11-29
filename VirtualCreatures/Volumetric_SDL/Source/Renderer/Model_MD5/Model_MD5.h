#pragma once

#include <AssetManager/Asset.h>

#include <Renderer/SDL_OpenGL.h>
#include <Renderer/Model_MD5/Mesh.h>
#include <Renderer/Model_MD5/Joint.h>

#include <Renderer/Animation_MD5/Animation_MD5.h>

#include <AssetManager/Asset_Texture.h>
#include <AssetManager/AssetManager.h>

#include <Renderer/Shader/Shader.h>

#include <Scene/BatchRenderer.h>

#include <Renderer/Material.h>

#define MODEL_MD5_BATCHRENDERER_NAME "md5"

class Model_MD5 :
	public Asset
{
public:
	enum VertexSkinningMode
	{
		e_CPU,
		e_GPU
	};

	enum ShaderCombination
	{
		e_plainOnly, e_bumpOnly, e_both
	};

private:
	int m_version;
	int m_numMeshes;
	int m_numJoints;

	std::vector<Mesh> m_meshes; 
	std::vector<Joint> m_joints;

	// Associates joint names with joint indices
	std::unordered_map<std::string, int> m_jointMap;

	AssetManager m_textures;

	std::vector<Material> m_materials;

	ShaderCombination m_shaderCombination;

	bool m_loaded;

	// Called once on load (initial preparation)
	void PrepareMeshVertices_FirstTime(Mesh &m); // With first time version sets up tex coords once as well as bone buffers and for all. Used for initial setup.
	void PrepareMeshVertices(Mesh &m); // Doesn't re-do texture coordinates. Used to reset to bind pos.
	void PrepareMeshNormals(Mesh &m);

	void CreateMeshBuffers(Mesh &m);

	// Called whenever animation is updated
	void PrepareMeshVertices(Mesh &m, const FrameSkeleton &skeleton);

	void RenderMesh_CPU(Mesh &m);
	void RenderMesh_GPU(Mesh &m);

	void RenderAllMeshes_CPU();
	void RenderAllMeshes_GPU();

	void Render_Immediate_CPU();
	void Render_Immediate_GPU();

	void RenderAllMeshes_Immediate_CPU();
	void RenderAllMeshes_Immediate_GPU();

	AABB m_noAnimationAABB;

	// For GPU Skinning
	std::vector<Matrix4x4f> m_bindPose;
	std::vector<Matrix4x4f> m_inverseBindPose;

	std::vector<Matrix4x4f> m_animatedBones;

	Scene* m_pScene;
	class Model_MD5_BatchRenderer* m_pBatchRenderer;

	// Build the bind-pose and the inverse bind-pose matrix array for the model.
	void BuildBindPose(const std::vector<Joint> &joints);

public:
	VertexSkinningMode m_vertexSkinningMode;

	Animation_MD5* m_pAnimation;

	Model_MD5();

	void SetSkinningMode(VertexSkinningMode mode);

	// Inherited from Asset
	bool LoadAsset(const std::string &name);

	void SetRenderer(Scene* pScene);

	void Update(float time);

	bool CheckAnimation();

	void Render(float time, const Matrix4x4f &transform);
	void Render_Immediate(); // Version that does NOT add the model to the batch renderer. Useful for temporary bone changes

	void DebugRender_Skeleton();

	void FindShaderCombination();

	ShaderCombination GetShaderCombination() const;

	const AABB &GetAABB();

	void SetBindPos();

	int GetNumMeshes() const;
	const Mesh &GetMesh(int index) const;

	// Asset factory
	static Asset* Asset_Factory();

	friend class Model_MD5_BatchRenderer;
};

class Model_MD5_BatchRenderer :
	public BatchRenderer
{
public:
	static const int s_numBoneMatrices = 20;

	enum GBufferRenderShader
	{
		e_plain = 0, e_bump
	} ;

private:
	struct TimeAndTransform
	{
		float m_time;
		Matrix4x4f m_transform;

		TimeAndTransform() {}
		TimeAndTransform(float time, const Matrix4x4f transform)
			: m_time(time), m_transform(transform)
		{
		}
	};

	Shader* m_pCurrentGBufferRenderShader;

	Shader m_skinning_gBufferRender;
	Shader m_skinning_gBufferRender_bump;

	int m_boneMatrixUniformLocations_plain[s_numBoneMatrices];
	int m_boneMatrixUniformLocations_bump[s_numBoneMatrices];

	struct ShaderGrouping
	{
		// 2 categories, each category sorts by instances (with hash maps)
		std::unordered_map<Model_MD5*, std::vector<TimeAndTransform>> m_subObjects_plainShaderOnly;
		std::unordered_map<Model_MD5*, std::vector<TimeAndTransform>> m_subObjects_bumpMappingShaderOnly;
	} m_CPU_group, m_GPU_group;

	void DrawGroups();

	// Last category doesn't differentiate between CPU and GPU skinning
	std::unordered_map<Model_MD5*, std::vector<TimeAndTransform>> m_subObjects_mutlipleShaders;

	static const unsigned int m_numGBufferRenderShaders = 2;

	Shader* m_pGBufferRenderShaders[m_numGBufferRenderShaders];

	// Material data (needed since these shaders are separate from normal rendering shaders)
	bool m_usingDiffuseTexture;
	bool m_usingSpecularTexture;
	bool m_usingEmissiveTexture;

	GBufferRenderShader m_currentGBufferRenderShader;

	// Use to completely deactivate materials when not running a frame (by assigning a function that does nothing)
	void (Model_MD5_BatchRenderer::*m_pSetDiffuseColorFunc)(const Color3f &color);
	void (Model_MD5_BatchRenderer::*m_pSetSpecularColorFunc)(float color);
	void (Model_MD5_BatchRenderer::*m_pSetEmissiveColorFunc)(const Color3f &color);

	void SetDiffuseColor_DeferredRender(const Color3f &color);
	void SetDiffuseColor_NotUsingShader(const Color3f &color);
	void SetSpecularColor_DeferredRender(float color);
	void SetSpecularColor_NotUsingShader(float color);
	void SetEmissiveColor_DeferredRender(const Color3f &color);
	void SetEmissiveColor_NotUsingShader(const Color3f &color);

	Color3f m_diffuseColor;
	float m_specularColor;
	Color3f m_emissiveColor;

	void SetDiffuseColor(const Color3f &color);
	void SetSpecularColor(float color);
	void SetEmissiveColor(const Color3f &color);

	void UseDiffuseTexture(bool use);
	void UseSpecularTexture(bool use);
	void UseEmissiveTexture(bool use);

	void SetMaterial_GPU(const Material &mat);

	void SetDeferredRenderFuncs();
	void SetNotUsingShaderFuncs();

	void SetCurrentGBufferRenderShader(Model_MD5_BatchRenderer::GBufferRenderShader gBufferRenderShader);

	bool m_created;

public:
	Model_MD5_BatchRenderer();

	bool Create(const std::string &gBufferRenderShaderName, const std::string &gBufferRenderBumpShaderName);

	bool Created() const
	{
		return m_created;
	}

	void Add(Model_MD5* pModel, float time, const Matrix4x4f &transform);

	// Inherited from BatchRenderer
	void Execute();
	void Clear();

	void SetMaterial(const Material &material);
	void SetBoneMatrices_Plain(const std::vector<Matrix4x4f> &matrices);
	void SetBoneMatrices_Bump(const std::vector<Matrix4x4f> &matrices);

	static BatchRenderer* Model_MD5_BatchRendererFactory();

	friend class Model_MD5;
};