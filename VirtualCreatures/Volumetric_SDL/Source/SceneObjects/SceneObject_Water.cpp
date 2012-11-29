#include <SceneObjects/SceneObject_Water.h>

#include <Constructs/Point2i.h>

#include <Utilities/UtilFuncs.h>

const float SceneObject_Water::s_waterSurfaceOffset = 0.2f;
const float SceneObject_Water::s_texScalar = 1.0f;
bool SceneObject_Water::s_waterShaderSetup = false;

SceneObject_Water::SceneObject_Water()
	: m_pDeferredWaterShader(NULL), m_bumpMapOffset(0.0f), m_bumpMapDriftSpeed(0.001f)
{
	m_unmanagedName = "water";
}

SceneObject_Water::~SceneObject_Water()
{
}

bool SceneObject_Water::Create(const std::string &bumpMap0Name, const std::string &bumpMap1Name, const Point3i worldStartVoxel)
{
	assert(m_pDeferredWaterShader == NULL);

	Scene* pScene = GetScene();

	assert(pScene != NULL);

	Asset* pAsset;

	if(!pScene->GetAssetManager_AutoCreate("tex", Asset_Texture::Asset_Factory)->GetAsset(bumpMap0Name, pAsset))
		return false;

	m_pBumpMap0 = static_cast<Asset_Texture*>(pAsset);

	if(!pScene->GetAssetManager("tex")->GetAsset(bumpMap1Name, pAsset))
		return false;

	m_pBumpMap1 = static_cast<Asset_Texture*>(pAsset);

	// Texture settings to wrap
	m_pBumpMap0->Bind();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	m_pBumpMap1->Bind();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Get special GBuffer render shader that overlaps the bump maps
	if(!pScene->GetAssetManager_AutoCreate("shader", Shader::Asset_Factory)->GetAsset("NONE data/shaders/gBufferWaterRender.vert data/shaders/gBufferWaterRender.frag", pAsset))
		return false;

	m_pDeferredWaterShader = static_cast<Shader*>(pAsset);

	if(!s_waterShaderSetup)
	{
		m_pDeferredWaterShader->Bind();

		m_pDeferredWaterShader->SetUniformv3f("gDiffuseColor", Color3f(0.2f, 1.0f, 0.2f));
		m_pDeferredWaterShader->SetUniformf("gSpecularColor", 1.0f);

		m_pDeferredWaterShader->Unbind();

		s_waterShaderSetup = true;
	}

	// Get pointer to world
	m_pWorld = static_cast<World*>(pScene->GetNamed_SceneObject("world"));
	
	assert(m_pWorld != NULL);

	ExpandInWorld(worldStartVoxel);

	GL_ERROR_CHECK();

	return true;
}

void SceneObject_Water::ExpandInWorld(const Point3i worldStartVoxel)
{
	std::vector<Vec3f> positions;
	std::vector<Vec3f> normals;
	std::vector<Vec2f> texCoords;

	std::vector<Point2i> surfaceVoxels;

	// Expand surface only first
	std::list<Point2i> openList;

	Point2i current;

	// Add starting point
	openList.push_back(Point2i(worldStartVoxel.x, worldStartVoxel.z));

	while(!openList.empty())
	{
		current = openList.back();
		openList.pop_back();

		// Set current voxel to visited (water)
		m_pWorld->SetVoxel(current.x, worldStartVoxel.y, current.y, s_waterBlockID);

		// Add to visited (closed) list
		surfaceVoxels.push_back(current);

		if(surfaceVoxels.size() > s_numWaterBlocksLimit)
			break;

		// Add neighbors
		for(int dx = -1; dx <= 1; dx++)
			for(int dz = -1; dz <= 1; dz++)
			{
				// Skip center
				if(dx == 0 && dz == 0)
					continue;

				Point2i newPoint(current.x + dx, current.y + dz);

				if(m_pWorld->GetVoxel(newPoint.x, worldStartVoxel.y, newPoint.y) == 0)
					openList.push_back(newPoint);
			}
	}

	Vec2f startPosf(static_cast<float>(worldStartVoxel.x), static_cast<float>(worldStartVoxel.z));

	float waterLevel = static_cast<float>(worldStartVoxel.y) + s_waterSurfaceOffset;

	m_aabb.m_upperBound = m_aabb.m_lowerBound = Vec3f(static_cast<float>(worldStartVoxel.x), waterLevel, static_cast<float>(worldStartVoxel.z));

	m_aabb.m_lowerBound.y -= 0.01f;

	// No go through surface voxels and propagate downwards and expand the AABB
	for(unsigned int i = 0, size = surfaceVoxels.size(); i < size; i++)
	{
		// Add geometry
		Vec3f waterPos(static_cast<float>(surfaceVoxels[i].x), waterLevel, static_cast<float>(surfaceVoxels[i].y));

		AddGeometry(positions, normals, texCoords, waterPos, Vec2f(waterPos.x, waterPos.z) - startPosf);

		// Expand AABB
		if(waterPos.x < m_aabb.m_lowerBound.x)
			m_aabb.m_lowerBound.x = waterPos.x;
		if(waterPos.z < m_aabb.m_lowerBound.z)
			m_aabb.m_lowerBound.z = waterPos.z;
		if(waterPos.x + 1.0f > m_aabb.m_upperBound.x)
			m_aabb.m_upperBound.x = waterPos.x + 1.0f;
		if(waterPos.z + 1.0f > m_aabb.m_upperBound.z)
			m_aabb.m_upperBound.z = waterPos.z + 1.0f;

		for(int h = -1; m_pWorld->GetVoxel(surfaceVoxels[i].x, worldStartVoxel.y + h, surfaceVoxels[i].y) == 0; h--)
			m_pWorld->SetVoxel(surfaceVoxels[i].x, worldStartVoxel.y + h, surfaceVoxels[i].y, s_waterBlockID);
	}

	m_aabb.CalculateHalfDims();
	m_aabb.CalculateCenter();

	if(IsSPTManaged())
		TreeUpdate();

	// Generate VBOs
	m_vertexBuffer.Create();
	m_texCoordBuffer.Create();
	m_normalBuffer.Create();

	// Vertex VBO
	m_numVertices = positions.size();

	m_vertexBuffer.Bind(GL_ARRAY_BUFFER);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * m_numVertices, &positions[0], GL_STATIC_DRAW);
	m_vertexBuffer.Unbind();

	// TexCoord VBO
	m_texCoordBuffer.Bind(GL_ARRAY_BUFFER);
	glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(float) * m_numVertices, &texCoords[0], GL_STATIC_DRAW);
	m_texCoordBuffer.Unbind();

	// Normal VBO
	m_normalBuffer.Bind(GL_ARRAY_BUFFER);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * m_numVertices, &normals[0], GL_STATIC_DRAW);
	m_normalBuffer.Unbind();

	GL_ERROR_CHECK();
}

void SceneObject_Water::AddGeometry(std::vector<Vec3f> &positions, std::vector<Vec3f> &normals, std::vector<Vec2f> &texCoords, const Vec3f &position, const Vec2f &changeInPosition)
{
	positions.push_back(position + Vec3f(0.0f, 0.0f, 1.0f));
	positions.push_back(position + Vec3f(1.0f, 0.0f, 1.0f));
	positions.push_back(position + Vec3f(1.0f, 0.0f, 0.0f));
	positions.push_back(position);

	for(int i = 0; i < 4; i++)
		normals.push_back(Vec3f(0.0f, 1.0f, 0.0f));

	Vec2f bottomLeft(changeInPosition.x * s_texScalar, changeInPosition.y * s_texScalar);
	Vec2f topRight((changeInPosition.x + 1.0f) * s_texScalar, (changeInPosition.y + 1.0f) * s_texScalar);

	texCoords.push_back(Vec2f(bottomLeft.x, bottomLeft.y));
	texCoords.push_back(Vec2f(topRight.x, bottomLeft.y));
	texCoords.push_back(Vec2f(topRight.x, topRight.y));
	texCoords.push_back(Vec2f(bottomLeft.x, topRight.y));
}

void SceneObject_Water::Logic()
{
	m_bumpMapOffset = fmodf(m_bumpMapOffset + m_bumpMapDriftSpeed * GetScene()->m_frameTimer.GetTimeMultiplier(), 1.0f);
}

void SceneObject_Water::Render()
{
	glEnable(GL_VERTEX_ARRAY);
	glEnable(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_NORMAL_ARRAY);

	m_vertexBuffer.Bind(GL_ARRAY_BUFFER);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	m_texCoordBuffer.Bind(GL_ARRAY_BUFFER);
	glTexCoordPointer(2, GL_FLOAT, 0, NULL);

	m_normalBuffer.Bind(GL_ARRAY_BUFFER);
	glNormalPointer(GL_FLOAT, 0, NULL);

	if(GetScene()->m_renderingDeferred)
	{
		// Draw geometry
		m_pDeferredWaterShader->Bind();
		m_pDeferredWaterShader->SetShaderTexture("gBumpMap0", m_pBumpMap0->GetTextureID(), GL_TEXTURE_2D);
		m_pDeferredWaterShader->SetShaderTexture("gBumpMap1", m_pBumpMap1->GetTextureID(), GL_TEXTURE_2D);
		m_pDeferredWaterShader->BindShaderTextures();
		m_pDeferredWaterShader->SetUniformf("bumpMapOffset", m_bumpMapOffset);

		glDrawArrays(GL_QUADS, 0, m_numVertices); // NULL at end makes it use the bound VBO

		// Bind normal gBuffer render again
		GetScene()->RebindGBufferRenderShader();
	}
	else
		glDrawArrays(GL_QUADS, 0, m_numVertices); // NULL at end makes it use the bound VBO

	m_normalBuffer.Unbind();
	m_texCoordBuffer.Unbind();
	m_vertexBuffer.Unbind();

	glDisable(GL_VERTEX_ARRAY);
	glDisable(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_NORMAL_ARRAY);

	GL_ERROR_CHECK();
}