#include <Scene/Scene.h>

#include <Renderer/SDL_OpenGL.h>

#include <algorithm>

#include <sstream>

#include <assert.h>

Scene::Scene()
	: m_frameTimer(5, 60.0f), m_created(false), m_treeSetup(false), m_pWin(NULL),
	m_usingDiffuseTexture(true), m_usingSpecularTexture(false),
	m_pRenderFunc(&Scene::Render),
	m_pSetDiffuseColorFunc(&Scene::SetDiffuseColor_NotUsingShader), m_pSetSpecularColorFunc(&Scene::SetSpecularColor_NotUsingShader), m_pSetEmissiveColorFunc(&Scene::SetEmissiveColor_NotUsingShader),
	m_renderingDeferred(false), m_clearingSceneObjects(false), m_clearingSceneEffects(false), m_clearingBatchRenderers(false),
	m_diffuseColor(1.0f, 1.0f, 1.0f), m_specularColor(0.0f), m_emissiveColor(0.0f, 0.0f, 0.0f),
	m_enableShaderSwitches(true), m_defaultLayer(0)
{
	// Default shader pointer
	m_pCurrentGBufferRenderShader = &m_gBufferRender;

	// Set up shader references
	m_pGBufferRenderShaders[e_plain] = &m_gBufferRender;
	m_pGBufferRenderShaders[e_bump] = &m_gBufferRender_bump;

	m_scene_Occlusion_Renderer.m_pScene = this;

	GL_ERROR_CHECK();
}

Scene::~Scene()
{
	Clear();
}

bool Scene::DistCompare(OctreeOccupant* first, OctreeOccupant* second)
{
	Scene* pScene = static_cast<SceneObject*>(first)->GetScene();

	// Compare distance to center. Can use magnitude squared, since comparing, and if magnitude squared is greater, magnitude would be greater as well
	return ((pScene->m_camera.m_position - first->GetAABB().GetCenter()).MagnitudeSquared()) < ((pScene->m_camera.m_position - second->GetAABB().GetCenter()).MagnitudeSquared()) ? true : false;
}

void Scene::Create(Window* pWin, 
	const std::string &gBufferRenderShaderFileName,
	const std::string &gBufferRenderBumpShaderFileName,
	const std::string &blankWhiteTextureFileName,
	const std::string &normalizedCubeFileName)
{
	assert(!m_created);

	m_pWin = pWin;

	std::ostringstream os;

	if(!m_gBufferRender.LoadAsset(gBufferRenderShaderFileName))
		abort();

	if(!m_gBufferRender_bump.LoadAsset(gBufferRenderBumpShaderFileName))
		abort();

	if(!m_whiteTexture.LoadAsset(blankWhiteTextureFileName))
		abort();

	if(!m_normalizedCube.LoadAsset(normalizedCubeFileName))
		abort();

	/*const char* uniformBufferNames[] =
	{
		"gDiffuseColor",
		"gDiffuseColor",
		"gDiffuseMap",
		"gSpecularColor",
		"gSpecularMap",
		"gEmissiveColor",
		"gEmissiveMap"
	};*/

	// Defaults
	m_gBufferRender.Bind();

	m_gBufferRender.SetUniformv3f("gDiffuseColor", m_diffuseColor);
	m_gBufferRender.SetUniformf("gSpecularColor", m_specularColor);
	m_gBufferRender.SetUniformv3f("gEmissiveColor", m_emissiveColor);

	m_gBufferRender.SetShaderTexture("gDiffuseMap", m_whiteTexture.GetTextureID(), GL_TEXTURE_2D); // GL_TEXTURE0
	m_gBufferRender.SetShaderTexture("gSpecularMap", m_whiteTexture.GetTextureID(), GL_TEXTURE_2D); // GL_TEXTURE1
	m_gBufferRender.SetShaderTexture("gEmissiveMap", m_whiteTexture.GetTextureID(), GL_TEXTURE_2D); // GL_TEXTURE2

	// Defaults - Bump version
	m_gBufferRender_bump.Bind();

	m_gBufferRender_bump.SetUniformv3f("gDiffuseColor", m_diffuseColor);
	m_gBufferRender_bump.SetUniformf("gSpecularColor", m_specularColor);
	m_gBufferRender_bump.SetUniformv3f("gEmissiveColor", m_emissiveColor);

	m_gBufferRender_bump.SetShaderTexture("gDiffuseMap", m_whiteTexture.GetTextureID(), GL_TEXTURE_2D); // GL_TEXTURE0
	m_gBufferRender_bump.SetShaderTexture("gSpecularMap", m_whiteTexture.GetTextureID(), GL_TEXTURE_2D); // GL_TEXTURE1
	m_gBufferRender_bump.SetShaderTexture("gEmissiveMap", m_whiteTexture.GetTextureID(), GL_TEXTURE_2D); // GL_TEXTURE2
	m_gBufferRender_bump.SetShaderTexture("gNormalMap", m_whiteTexture.GetTextureID(), GL_TEXTURE_2D); // GL_TEXTURE3

	m_gBufferRender_bump.Unbind();
	
	// Create G Buffer
	m_gBuffer.Create(static_cast<unsigned int>(m_pWin->m_projected_width), static_cast<unsigned int>(m_pWin->m_projected_height));

	m_created = true;
}

void Scene::TreeSetup(const AABB &rootRegion)
{
	assert(m_created);
	assert(!m_treeSetup);

	m_spt.Create(rootRegion);

	m_treeSetup = true;
}

bool Scene::Created()
{
	return m_created;
}

bool Scene::TreeWasSetup()
{
	return m_treeSetup;
}

void Scene::ClearSceneObjects()
{
	m_clearingSceneObjects = true;

	for(unsigned int i = 0, size = m_objects.size(); i < size; i++)
		for(std::list<SceneObject*>::iterator it = m_objects[i].begin(); it != m_objects[i].end(); it++)
			delete *it;

	m_objects.clear();

	m_namedObjects.clear();

	m_spt.Clear();

	m_clearingSceneObjects = false;
}

void Scene::ClearEffects()
{
	m_clearingSceneEffects = true;

	for(unsigned int i = 0, size = m_effects.size(); i < size; i++)
		delete m_effects[i];

	m_effects.clear();

	m_namedEffects.clear();

	m_clearingSceneEffects = false;
}

void Scene::ClearBatchRenderers()
{
	for(std::unordered_map<std::string, BatchRenderer*>::iterator it = m_pBatchRenderers.begin(); it != m_pBatchRenderers.end(); it++)
		delete it->second;

	m_pBatchRenderers.clear();
}

void Scene::ClearAssetManagers()
{
	for(std::unordered_map<std::string, AssetManager*>::iterator it = m_pAssetManagers.begin(); it != m_pAssetManagers.end(); it++)
		delete it->second;

	m_pAssetManagers.clear();
}

void Scene::Clear()
{
	ClearSceneObjects();
	ClearEffects();
	ClearBatchRenderers();
	ClearAssetManagers();
}

void Scene::Add(SceneObject* object, bool sptManaged)
{
	object->m_pScene = this;

	ResizeOnAdd(m_defaultLayer);

	m_objects[m_defaultLayer].push_back(object);

	if(sptManaged)
	{
		assert(m_treeSetup);

		m_spt.Add(object);
	}

	object->OnAdd();
}

void Scene::Add(SceneObject* object, bool sptManaged, const std::string &name)
{
	object->m_pScene = this;
	object->m_managedName = name;

	ResizeOnAdd(m_defaultLayer);

	m_objects[m_defaultLayer].push_back(object);

	if(sptManaged)
	{
		assert(m_treeSetup);

		m_spt.Add(object);
	}

	m_namedObjects[name] = object;

	object->OnAdd();
}

void Scene::Add_Layer(SceneObject* object, bool sptManaged, unsigned int layer)
{
	object->m_pScene = this;

	ResizeOnAdd(layer);

	m_objects[layer].push_back(object);

	if(sptManaged)
	{
		assert(m_treeSetup);

		m_spt.Add(object);
	}

	object->OnAdd();
}

void Scene::Add_Layer(SceneObject* object, bool sptManaged, const std::string &name, unsigned int layer)
{
	object->m_pScene = this;
	object->m_managedName = name;

	ResizeOnAdd(layer);

	m_objects[layer].push_back(object);

	if(sptManaged)
	{
		assert(m_treeSetup);

		m_spt.Add(object);
	}

	m_namedObjects[name] = object;

	object->OnAdd();
}

void Scene::Logic()
{
	assert(m_created);

	bool objectRemoved = false;

	for(unsigned int i = 0, size = m_objects.size(); i < size; i++)
		for(std::list<SceneObject*>::iterator it = m_objects[i].begin(); it != m_objects[i].end();)
		{
			SceneObject* pObject = *it;

			// Remove dead
			if(pObject->ShouldDestroy())
			{
				if(pObject->GetManagedName() != "")
					m_namedObjects.erase(pObject->GetManagedName());

				it = m_objects[i].erase(it);

				delete pObject;

				objectRemoved = true;
			}
			else
			{
				pObject->Logic();

				it++;
			}
		}

	if(objectRemoved)
		ResizeOnRemove();
}

void Scene::Render()
{
	assert(m_created);
	assert(m_treeSetup);

	for(unsigned int i = 0, size = m_objects.size(); i < size; i++)
		for(std::list<SceneObject*>::iterator it = m_objects[i].begin(); it != m_objects[i].end(); it++)
		{
			SceneObject* pObject = *it;

			if(!pObject->IsSPTManaged())
				pObject->Render();
		}

	// Query the tree
	std::vector<OctreeOccupant*> result;

	m_spt.Query_Frustum(result, m_frustum);

	// Sort results by depth
	// although not doing occlusion culling, this can reduce memory bandwidth for
	// deferred rendering buffer writes by not writing to occluded fragments)
	//std::sort(result.begin(), result.end(), &Scene::DistCompare);

	// Set all queried items to cull
	for(unsigned int i = 0, size = result.size(); i < size; i++)
	{
		SceneObject* pObject = static_cast<SceneObject*>(result[i]);

		pObject->Render();
	}

	ExecuteBatches();
}

void Scene::Render_OcclusionCull()
{
	assert(m_created);
	assert(m_treeSetup);

	for(unsigned int i = 0, size = m_objects.size(); i < size; i++)
		for(std::list<SceneObject*>::iterator it = m_objects[i].begin(); it != m_objects[i].end(); it++)
		{
			SceneObject* pObject = *it;

			if(!pObject->IsSPTManaged())
				pObject->Render();
		}

	m_occlusionCuller.Run(m_camera.m_position, m_frustum);
}

void Scene::Render_UseSetFunc()
{
	(this->*m_pRenderFunc)();
}

void Scene::Render_Distance(float distance)
{
	assert(m_created);
	assert(m_treeSetup);

	for(unsigned int i = 0, size = m_objects.size(); i < size; i++)
		for(std::list<SceneObject*>::iterator it = m_objects[i].begin(); it != m_objects[i].end(); it++)
		{
			SceneObject* pObject = *it;

			if(!pObject->IsSPTManaged())
				pObject->Render();
		}

	// Query the tree
	std::vector<OctreeOccupant*> result;

	m_spt.Query_Frustum(result, m_frustum);

	// Sort results by depth
	// although not doing occlusion culling, this can reduce memory bandwidth for
	// deferred rendering buffer writes by not writing to occluded fragments)
	std::sort(result.begin(), result.end(), &Scene::DistCompare);

	// Set all queried items to cull
	for(unsigned int i = 0, size = result.size(); i < size; i++)
	{
		SceneObject* pObject = static_cast<SceneObject*>(result[i]);

		// If in range (distance parameter)
		if((pObject->m_aabb.GetCenter() - m_camera.m_position).Magnitude() - distance <= distance)
			pObject->Render();
	}

	ExecuteBatches();
}

void Scene::Frame()
{
	m_inputHandler.GetInputs();

	Logic();

	// Default world matrix
	m_camera.GetViewMatrix(m_viewMatrix);

	CalculateInverseAndNormalMatrix();

	Matrix4x4f viewProjection(Matrix4x4f::GL_Get_Projection() * m_viewMatrix);
	ExtractFrustum(viewProjection);

	Matrix4x4f inverseViewProjection;
	if(!viewProjection.Inverse(inverseViewProjection))
		std::cout << "????";
	m_frustum.CalculateCorners(inverseViewProjection);

	m_gBuffer.Bind_Draw();
	m_gBuffer.Set_DrawGeom();
	
	RebindGBufferRenderShader();

	SetWorldMatrix(Matrix4x4f::IdentityMatrix());

	glDepthMask(true);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set material color functions for deferred rendering
	SetDeferredRenderFuncs();

	m_renderingDeferred = true;

	(this->*m_pRenderFunc)();

	m_renderingDeferred = false;

	// Set material color functions to no material
	SetNotUsingShaderFuncs();

	glDepthMask(false);

	Shader::Unbind();

	m_gBuffer.Finish_Draw();
	m_gBuffer.Bind_Read();
	m_gBuffer.SetRead_Effect();
	m_gBuffer.SetDraw_Effect();

	RunEffects();

	m_gBuffer.Finish_Draw();
	m_gBuffer.Unbind_Draw();

	// Copy effect buffer to main framebuffer (effect composition complete)
	m_gBuffer.CopyEffectToMainFrameBuffer();

	m_gBuffer.Unbind_Read();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GL_ERROR_CHECK();

	glDepthMask(true);

	m_frameTimer.EndFrameUpdate();
}

const Frustum &Scene::GetFrustum()
{
	return m_frustum;
}

void Scene::ExtractFrustum(const Matrix4x4f &viewProjection)
{
	m_frustum.ExtractFromMatrix(viewProjection);
}

void Scene::ExtractFrustum()
{
	m_frustum.ExtractFromMatrix(Matrix4x4f::GL_Get_Projection() * Matrix4x4f::GL_Get_Modelview());
}

SceneObject* Scene::GetNamed_SceneObject(const std::string &name)
{
	assert(m_created);

	std::unordered_map<std::string, SceneObject*>::iterator it = m_namedObjects.find(name);

	if(it == m_namedObjects.end())
		return NULL;

	return it->second;
}

SceneEffect* Scene::GetNamed_Effect(const std::string &name)
{
	assert(m_created);

	std::unordered_map<std::string, SceneEffect*>::iterator it = m_namedEffects.find(name);

	if(it == m_namedEffects.end())
		return NULL;

	return it->second;
}

void Scene::Add(SceneEffect* effect, unsigned int layer)
{
	if(layer >= m_effects.size())
	{
		// Resize
		for(unsigned int i = m_effects.size(); i < layer; i++)
			m_effects.push_back(NULL);

		m_effects.push_back(effect);
	}
	else
		m_effects[layer] = effect;

	effect->m_layer = layer;
	effect->m_pScene = this;

	effect->OnAdd();
}

void Scene::Add(SceneEffect* effect, unsigned int layer, const std::string &name)
{
	if(layer >= m_effects.size())
	{
		// Resize
		for(unsigned int i = m_effects.size(); i < layer; i++)
			m_effects.push_back(NULL);

		m_effects.push_back(effect);
	}
	else
		m_effects[layer] = effect;

	effect->m_layer = layer;
	effect->m_pScene = this;

	m_namedEffects[name] = effect;

	effect->OnAdd();
}

void Scene::RemoveEffect(unsigned int layer)
{
	const unsigned int lastIndex = m_effects.size() - 1;

	if(m_effects[layer]->GetManagedName() != "")
		m_namedEffects.erase(m_effects[layer]->GetManagedName());

	assert(layer >= lastIndex);

	delete m_effects[layer];

	if(layer == lastIndex)
	{
		// Shrink effects array as much as possible
		unsigned int i;

		for(i = lastIndex; i >= 0 && m_effects[i] != NULL; i--);

		m_effects.resize(i);
	}
	else
		m_effects[layer] = NULL;
}

unsigned int Scene::GetNumLayers() const
{
	return m_objects.size();
}

unsigned int Scene::GetNumSceneObjects(unsigned int layer) const
{
	assert(layer < m_objects.size());

	return m_objects[layer].size();
}

void Scene::RunEffects()
{
	for(unsigned int i = 0, size = m_effects.size(); i < size; i++)
	{
		if(m_effects[i] != NULL)
			m_effects[i]->RunEffect();
	}
}

void Scene::SetCurrentGBufferRenderShader(Scene::GBufferRenderShader gBufferRenderShader)
{
	assert(gBufferRenderShader >= 0 && gBufferRenderShader < 2);

	if(!m_enableShaderSwitches)
		return;

	if(m_currentGBufferRenderShader != gBufferRenderShader)
	{
		m_currentGBufferRenderShader = gBufferRenderShader;

		m_pCurrentGBufferRenderShader = m_pGBufferRenderShaders[gBufferRenderShader];

		m_pCurrentGBufferRenderShader->Bind();
		m_pCurrentGBufferRenderShader->BindShaderTextures();

		// Set color uniforms, since they easily carry if not carefully reset after every shader change
		m_pCurrentGBufferRenderShader->SetUniformv3f("gDiffuseColor", m_diffuseColor);
		m_pCurrentGBufferRenderShader->SetUniformf("gSpecularColor", m_specularColor);
		m_pCurrentGBufferRenderShader->SetUniformv3f("gEmissiveColor", m_emissiveColor);
	}
}

void Scene::SetCurrentGBufferRenderShader_ForceBind(Scene::GBufferRenderShader gBufferRenderShader)
{
	assert(gBufferRenderShader >= 0 && gBufferRenderShader < 2);

	if(!m_enableShaderSwitches)
		return;

	m_currentGBufferRenderShader = gBufferRenderShader;

	m_pCurrentGBufferRenderShader = m_pGBufferRenderShaders[gBufferRenderShader];

	m_pCurrentGBufferRenderShader->Bind();
	m_pCurrentGBufferRenderShader->BindShaderTextures();

	// Set color uniforms, since they easily carry if not carefully reset after every shader change
	m_pCurrentGBufferRenderShader->SetUniformv3f("gDiffuseColor", m_diffuseColor);
	m_pCurrentGBufferRenderShader->SetUniformf("gSpecularColor", m_specularColor);
	m_pCurrentGBufferRenderShader->SetUniformv3f("gEmissiveColor", m_emissiveColor);
}

Scene::GBufferRenderShader Scene::GetCurrentGBufferRenderShader() const
{
	return m_currentGBufferRenderShader;
}

void Scene::SetDiffuseColor_DeferredRender(const Color3f &color)
{
	m_diffuseColor = color;

	m_pCurrentGBufferRenderShader->SetUniformv3f("gDiffuseColor", m_diffuseColor);
}

void Scene::SetDiffuseColor_NotUsingShader(const Color3f &color)
{
	m_diffuseColor = color;
}

void Scene::SetSpecularColor_DeferredRender(float color)
{
	m_specularColor = color;

	m_pCurrentGBufferRenderShader->SetUniformf("gSpecularColor", m_specularColor);
}

void Scene::SetSpecularColor_NotUsingShader(float color)
{
	m_specularColor = color;
}

void Scene::SetEmissiveColor_DeferredRender(const Color3f &color)
{
	m_emissiveColor = color;

	m_pCurrentGBufferRenderShader->SetUniformv3f("gEmissiveColor", m_emissiveColor);
}

void Scene::SetEmissiveColor_NotUsingShader(const Color3f &color)
{
	m_emissiveColor = color;
}

void Scene::SetDiffuseColor(const Color3f &color)
{
	(this->*m_pSetDiffuseColorFunc)(color);
}

void Scene::SetSpecularColor(float color)
{
	(this->*m_pSetSpecularColorFunc)(color);
}

void Scene::SetEmissiveColor(const Color3f &color)
{
	(this->*m_pSetEmissiveColorFunc)(color);
}

const Color3f &Scene::GetDiffuseColor() const
{
	return m_diffuseColor;
}

float Scene::GetSpecularColor() const
{
	return m_specularColor;
}

const Color3f &Scene::GetEmissiveColor() const
{
	return m_emissiveColor;
}

void Scene::UseDiffuseTexture(bool use)
{
	if(use)
		m_usingDiffuseTexture = true;
	else
	{
		if(m_usingDiffuseTexture)
		{
			// White texture
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_whiteTexture.GetTextureID());
			//m_gBufferRender.SetShaderTexture("gDiffuseMap", m_whiteTexture.GetTextureID(), GL_TEXTURE_2D);

			m_usingDiffuseTexture = false;
		}
	}
}

void Scene::UseSpecularTexture(bool use)
{
	if(use)
		m_usingSpecularTexture = true;
	else
	{
		if(m_usingSpecularTexture)
		{
			// White texture
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, m_whiteTexture.GetTextureID());
			glActiveTexture(GL_TEXTURE0);
			//m_gBufferRender.SetShaderTexture("gSpecularMap", m_whiteTexture.GetTextureID(), GL_TEXTURE_2D);

			m_usingSpecularTexture = false;
		}
	}
}

void Scene::UseEmissiveTexture(bool use)
{
	if(use)
		m_usingEmissiveTexture = true;
	else
	{
		if(m_usingEmissiveTexture)
		{
			// White texture
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, m_whiteTexture.GetTextureID());
			glActiveTexture(GL_TEXTURE0);
	
			m_usingEmissiveTexture = false;
		}
	}
}

bool Scene::UsingDiffuseTexture() const
{
	return m_usingDiffuseTexture;
}

bool Scene::UsingSpecularTexture() const
{
	return m_usingSpecularTexture;
}

bool Scene::UsingEmissiveTexture() const
{
	return m_usingEmissiveTexture;
}

void Scene::RebindGBufferRenderShader()
{
	assert(m_pCurrentGBufferRenderShader != NULL);
	m_pCurrentGBufferRenderShader->Bind();
	m_pCurrentGBufferRenderShader->BindShaderTextures();
}

void Scene::SetRenderMode(bool occlusionCull)
{
	if(occlusionCull)
	{
		m_pRenderFunc = &Scene::Render_OcclusionCull;

		if(!m_occlusionCuller.Created())
			m_occlusionCuller.Create(&m_spt, &m_scene_Occlusion_Renderer);
	}
	else
		m_pRenderFunc = &Scene::Render;
}

void Scene::SetWorldMatrix(const Matrix4x4f &world)
{
	(m_viewMatrix * world).GL_Load();
}

void Scene::SetCustomViewMatrix(const Matrix4x4f &view)
{
	m_viewMatrix = view;
}

void Scene::CalculateInverseAndNormalMatrix()
{
	if(!m_viewMatrix.Inverse(m_inverseViewMatrix))
		return;

	m_normalMatrix = m_inverseViewMatrix.Transpose();
}

void Scene::SetDeferredRenderFuncs()
{
	m_pSetDiffuseColorFunc = &Scene::SetDiffuseColor_DeferredRender;
	m_pSetSpecularColorFunc = &Scene::SetSpecularColor_DeferredRender;
	m_pSetEmissiveColorFunc = &Scene::SetEmissiveColor_DeferredRender;
}

void Scene::SetNotUsingShaderFuncs()
{
	m_pSetDiffuseColorFunc = &Scene::SetDiffuseColor_NotUsingShader;
	m_pSetSpecularColorFunc = &Scene::SetSpecularColor_NotUsingShader;
	m_pSetEmissiveColorFunc = &Scene::SetEmissiveColor_NotUsingShader;
}

const Matrix4x4f &Scene::GetViewMatrix()
{
	return m_viewMatrix;
}

const Matrix4x4f &Scene::GetInverseViewMatrix()
{
	return m_inverseViewMatrix;
}

const Matrix4x4f &Scene::GetNormalMatrix()
{
	return m_normalMatrix;
}

const Asset_Texture &Scene::GetWhiteTexture() const
{
	return m_whiteTexture;
}

const Model_OBJ_VertexOnly &Scene::GetNormalizedCube() const
{
	return m_normalizedCube;
}

void Scene::DrawAABB(const AABB &aabb)
{
	SetWorldMatrix(Matrix4x4f::TranslateMatrix(aabb.GetCenter()) * Matrix4x4f::ScaleMatrix(aabb.GetHalfDims()));

	m_normalizedCube.Render();
}

void Scene::AddAssetManager(const std::string &name, Asset* (*assetFactory)())
{
	m_pAssetManagers[name] = new AssetManager(assetFactory);
}

AssetManager* Scene::GetAssetManager(const std::string &name)
{
	std::unordered_map<std::string, AssetManager*>::iterator it = m_pAssetManagers.find(name);

	if(it == m_pAssetManagers.end())
		return NULL;

	return it->second;
}

AssetManager* Scene::GetAssetManager_AutoCreate(const std::string &name, Asset* (*assetFactory)())
{
	std::unordered_map<std::string, AssetManager*>::iterator it = m_pAssetManagers.find(name);

	if(it == m_pAssetManagers.end())
	{
		// Create the asset manager
		AssetManager* pAssetManager = new AssetManager(assetFactory);

		m_pAssetManagers[name] = pAssetManager;

		return pAssetManager;
	}

	return it->second;
}

void Scene::ClearAssets()
{
	for(std::unordered_map<std::string, AssetManager*>::iterator it = m_pAssetManagers.begin(); it != m_pAssetManagers.end(); it++)
		it->second->ClearAssets();
}

void Scene::ResizeOnAdd(unsigned int newLayer)
{
	unsigned int numLayers = m_objects.size();
		
	if(newLayer >= numLayers)
		for(; numLayers <= newLayer; numLayers++)
			m_objects.push_back(std::list<SceneObject*>());
}
	
void Scene::ResizeOnRemove()
{
	const unsigned int numLayers = m_objects.size();
	const int lastLayerIndex = numLayers - 1;
		
	// Find the index of the highest used layer
	int lastUsedLayer;
		
	for(lastUsedLayer = lastLayerIndex; lastUsedLayer >= 0 && m_objects[lastUsedLayer].empty(); lastUsedLayer--);
		
	// Add one, since it will have gone one below the empty layers in order to check if it was empty
	lastUsedLayer++;
		
	// Remove all layers from the end of the array to the highest used layer
	if(lastUsedLayer != lastLayerIndex)
		m_objects.resize(lastUsedLayer);
}

BatchRenderer* Scene::GetBatchRenderer(const std::string &batchRendererName, BatchRenderer* (*batchRendererFactory)())
{
	// Look for batch renderer
	std::unordered_map<std::string, BatchRenderer*>::iterator it = m_pBatchRenderers.find(batchRendererName);

	BatchRenderer* pBatchRenderer;

	if(it == m_pBatchRenderers.end())
	{
		// Batch renderer does not exist, must be created first
		pBatchRenderer = batchRendererFactory();

		m_pBatchRenderers[batchRendererName] = pBatchRenderer;

		pBatchRenderer->m_pScene = this;
	}
	else
		pBatchRenderer = it->second;

	return pBatchRenderer;
}

void Scene::ExecuteBatches()
{
	for(std::unordered_map<std::string, BatchRenderer*>::iterator it = m_pBatchRenderers.begin(); it != m_pBatchRenderers.end(); it++)
	{
		it->second->Execute();
		it->second->Clear();
	}
}

Scene::Scene_Occlusion_Renderer::Scene_Occlusion_Renderer()
	: m_renderingAABBs(false)
{
}

void Scene::Scene_Occlusion_Renderer::ResetForFrame()
{
	glColorMask(true, true, true, true);
	glDepthMask(true);

	m_pScene->RebindGBufferRenderShader();

	glDisable(GL_VERTEX_ARRAY);

	m_renderingAABBs = false;
}

void Scene::Scene_Occlusion_Renderer::DrawAABB(const AABB &aabb)
{
	if(!m_renderingAABBs)
	{
		glColorMask(false, false, false, false);
		glDepthMask(false);

		Shader::Unbind();

		glEnable(GL_VERTEX_ARRAY);

		m_renderingAABBs = true;
	}

	m_pScene->DrawAABB(aabb);
}

void Scene::Scene_Occlusion_Renderer::Draw(OctreeOccupant* pOc)
{
	if(m_renderingAABBs)
	{
		glColorMask(true, true, true, true);
		glDepthMask(true);

		m_pScene->RebindGBufferRenderShader();

		glDisable(GL_VERTEX_ARRAY);

		m_renderingAABBs = false;
	}

	static_cast<SceneObject*>(pOc)->Render();
}

void Scene::Scene_Occlusion_Renderer::FinishDraw()
{
	if(m_renderingAABBs)
	{
		glColorMask(true, true, true, true);
		glDepthMask(true);

		m_pScene->RebindGBufferRenderShader();

		glDisable(GL_VERTEX_ARRAY);

		m_renderingAABBs = false;
	}

	m_pScene->ExecuteBatches();
}