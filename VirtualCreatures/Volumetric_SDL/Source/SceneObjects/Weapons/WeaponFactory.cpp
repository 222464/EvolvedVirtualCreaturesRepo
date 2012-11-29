#include <SceneObjects/Weapons/WeaponFactory.h>

#include <Scene/Scene.h>

#include <SceneObjects/Weapons/SceneObject_Shell.h>

#include <sstream>

#include <assert.h>

WeaponFactory::WeaponFactory()
	: m_scalar(1.0f, 1.0f, 1.0f), m_minBranchScale(0.6f), m_branchShrinkFactor(0.1f), m_generatedGeomScalar(0.15f), 
	m_preferBackIncreaseFactor(0.1f), m_maxBranchDecreaseFactor(0.7f), m_minBranchDecreaseFactor(0.7f),
	m_doNotPreferSidesDecreaseFactor(0.1f), m_genGeomTexScalar(600.0f), m_minTriggerOffsetFromStock(0.4f), m_maxTriggerOffsetFromStock(0.8f),
	m_minHandTriggerOffset(0.5f), m_maxHandTriggerOffset(1.2f), m_positionTestIncrement(0.1f)
{
}

bool WeaponFactory::Create(Weapon* weapon, Weapon::WeaponType type, const std::string &weaponAssetPackName, Scene* pScene)
{
	m_pWeapon = weapon;

	assert(!m_pWeapon->m_created);

	m_pWeapon->m_pScene = pScene;

	m_pWeapon->m_type = type;

	m_pWeapon->m_pPhysicsWorld = static_cast<SceneObject_PhysicsWorld*>(pScene->GetNamed_SceneObject("physWrld"));

	assert(m_pWeapon->m_pPhysicsWorld != NULL);

	// Load asset pack
	Asset* pWeaponAsset;

	if(!m_pWeapon->m_pScene->GetAssetManager_AutoCreate("weaponPack", WeaponAssetPack::Asset_Factory)->GetAsset(weaponAssetPackName, pWeaponAsset))
		return false;

	m_pWeapon->m_pAssetPack = static_cast<WeaponAssetPack*>(pWeaponAsset);

	m_pWeapon->m_pAssetPack->SetModelRenderers(pScene);

	AABB bounds;

	Asset* pModelAsset = NULL;

	SoundSystem* pSound = static_cast<SoundSystem*>(m_pWeapon->m_pScene->GetNamed_Effect("sndsys"));

	assert(pSound != NULL);

	Asset* pShootSoundAsset;
	Asset* pMuzzleFlashTextureAsset;

	switch(m_pWeapon->m_type)
	{
	case Weapon::e_ballistic:
		bounds.SetCenter(Vec3f(0.0f, 0.0f, 0.0f));
		bounds.SetHalfDims(Vec3f(3.0f, 0.5f, 0.3f));
		Create("ballistic", false, 10, 3.0f, 1.0f, 0.3f, 0.2f, Vec3f(4.5f, 1.2f, 0.7f), 0.3f, bounds);

		// Load shell model
		if(!m_pWeapon->m_pScene->GetAssetManager_AutoCreate("modelOBJ", Model_OBJ::Asset_Factory)->GetAsset("data/weapons/shells/ballisticShell1.obj", pModelAsset))
			return false;

		switch(rand() % 2)
		{
		case 0:
			if(!pSound->m_sound_effect_manager.GetAsset("data/weapons/sounds/ballisticShot1.wav", pShootSoundAsset))
				abort();
		case 1:
			if(!pSound->m_sound_effect_manager.GetAsset("data/weapons/sounds/ballisticShot2.wav", pShootSoundAsset))
				abort();
		}

		if(!m_pWeapon->m_pScene->GetAssetManager_AutoCreate("tex", Asset_Texture::Asset_Factory)->GetAsset("data/weapons/flashes/flashes1_HD.png", pMuzzleFlashTextureAsset))
			abort();

		break;
	default:
		std::cout << "Not a weapon type supported yet!" << std::endl;
		return false;
	}

	// Set up shell model
	if(pModelAsset != NULL)
	{
		m_pWeapon->m_pShellModel = static_cast<Model_OBJ*>(pModelAsset);
		m_pWeapon->m_pShellModel->SetRenderer(pScene);

		for(unsigned int i = 0, size = m_pWeapon->m_pShellModel->GetNumMaterials(); i < size; i++)
		{
			Model_OBJ::Material* pMat = m_pWeapon->m_pShellModel->GetMaterial(i);
		
			pMat->m_pDiffuseMap->Bind();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			if(pMat->m_pSpecularMap != NULL)
			{
				pMat->m_pSpecularMap->Bind();
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}

			if(pMat->m_pNormalMap != NULL)
			{
				pMat->m_pNormalMap->Bind();
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}
		}
	}

	m_pWeapon->m_pShootSound = static_cast<Sound_Effect*>(pShootSoundAsset);

	m_pWeapon->m_pMuzzleFlashSpriteSheet = static_cast<Asset_Texture*>(pMuzzleFlashTextureAsset);

	// Texture settings for muzzle flash sprite sheet
	glBindTexture(GL_TEXTURE_2D, m_pWeapon->m_pMuzzleFlashSpriteSheet->GetTextureID());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 

	assert(m_pWeapon->m_pShootSound != NULL);

	m_pWeapon->m_created = true;

	return true;
}

bool Weapon::Created()
{
	return m_created;
}

void WeaponFactory::GenGeom_Box(const Matrix4x4f &transform, const Vec3f &texScalar, const Vec2f &texOffset,
		std::vector<Vec3f> &positions,
		std::vector<Vec2f> &texCoords,
		std::vector<Vec3f> &normals)
{
	// Use to find current indices (offset)
	unsigned short lastIndex = positions.size();

	Vec2f invDims_undimensioned = m_pWeapon->m_pAssetPack->GetRandomTextureDimsInverse() * m_genGeomTexScalar;

	/* 
		Texture Coord / Vertex Layout
		    __
		 __|__|_____
		|__|y-|x+|__|
		   |z+|
	*/

	Vec2f invDims; // Changed according to side dimensions

	// +X
	invDims = invDims_undimensioned;
	invDims.x *= texScalar.z;
	invDims.y *= texScalar.y;

	positions.push_back(transform * Vec3f(1.0f, -1.0f, 1.0f));
	positions.push_back(transform * Vec3f(1.0f, -1.0f, -1.0f));
	positions.push_back(transform * Vec3f(1.0f, 1.0f, -1.0f));
	positions.push_back(transform * Vec3f(1.0f, 1.0f, 1.0f));

	texCoords.push_back(Vec2f(2.0f * invDims.x + texOffset.x, invDims.y + texOffset.y));
	texCoords.push_back(Vec2f(3.0f * invDims.x + texOffset.x, invDims.y + texOffset.y));
	texCoords.push_back(Vec2f(3.0f * invDims.x + texOffset.x, 2.0f * invDims.y + texOffset.y));
	texCoords.push_back(Vec2f(2.0f * invDims.x + texOffset.x, 2.0f * invDims.y + texOffset.y));
	
	for(int i = 0; i < 4; i++)
		normals.push_back(Vec3f(1.0f, 0.0f, 0.0f));

	// -X
	invDims = invDims_undimensioned;
	invDims.x *= texScalar.z;
	invDims.y *= texScalar.y;

	positions.push_back(transform * Vec3f(-1.0f, -1.0f, -1.0f));
	positions.push_back(transform * Vec3f(-1.0f, -1.0f, 1.0f));
	positions.push_back(transform * Vec3f(-1.0f, 1.0f, 1.0f));
	positions.push_back(transform * Vec3f(-1.0f, 1.0f, -1.0f));

	texCoords.push_back(Vec2f(invDims.x + texOffset.x, 2.0f * invDims.y + texOffset.y));
	texCoords.push_back(Vec2f(invDims.x + texOffset.x, invDims.y + texOffset.y));
	texCoords.push_back(Vec2f(texOffset.x, invDims.y + texOffset.y));
	texCoords.push_back(Vec2f(texOffset.x, 2.0f * invDims.y + texOffset.y));
	
	for(int i = 0; i < 4; i++)
		normals.push_back(Vec3f(-1.0f, 0.0f, 0.0f));

	// +Z
	invDims = invDims_undimensioned;
	invDims.x *= texScalar.x;
	invDims.y *= texScalar.y;

	positions.push_back(transform * Vec3f(-1.0f, -1.0f, 1.0f));
	positions.push_back(transform * Vec3f(1.0f, -1.0f, 1.0f));
	positions.push_back(transform * Vec3f(1.0f, 1.0f, 1.0f));
	positions.push_back(transform * Vec3f(-1.0f, 1.0f, 1.0f));

	texCoords.push_back(Vec2f(invDims.x + texOffset.x, invDims.y + texOffset.y));
	texCoords.push_back(Vec2f(2.0f * invDims.x + texOffset.x, invDims.y + texOffset.y));
	texCoords.push_back(Vec2f(2.0f * invDims.x + texOffset.x, texOffset.y));
	texCoords.push_back(Vec2f(invDims.x + texOffset.x, texOffset.y));

	for(int i = 0; i < 4; i++)
		normals.push_back(Vec3f(0.0f, 0.0f, 1.0f));

	// -Z
	invDims = invDims_undimensioned;
	invDims.x *= texScalar.x;
	invDims.y *= texScalar.y;

	positions.push_back(transform * Vec3f(1.0f, -1.0f, -1.0f));
	positions.push_back(transform * Vec3f(-1.0f, -1.0f, -1.0f));
	positions.push_back(transform * Vec3f(-1.0f, 1.0f, -1.0f));
	positions.push_back(transform * Vec3f(1.0f, 1.0f, -1.0f));

	texCoords.push_back(Vec2f(2.0f * invDims.x + texOffset.x, 2.0f * invDims.y + texOffset.y));
	texCoords.push_back(Vec2f(invDims.x + texOffset.x, 2.0f * invDims.y + texOffset.y));
	texCoords.push_back(Vec2f(invDims.x + texOffset.x, 3.0f * invDims.y + texOffset.y));
	texCoords.push_back(Vec2f(2.0f * invDims.x + texOffset.x, 3.0f * invDims.y + texOffset.y));

	for(int i = 0; i < 4; i++)
		normals.push_back(Vec3f(0.0f, 0.0f, -1.0f));

	// +Y
	invDims = invDims_undimensioned;
	invDims.x *= texScalar.x;
	invDims.y *= texScalar.y;

	positions.push_back(transform * Vec3f(-1.0f, 1.0f, 1.0f));
	positions.push_back(transform * Vec3f(1.0f, 1.0f, 1.0f));
	positions.push_back(transform * Vec3f(1.0f, 1.0f, -1.0f));
	positions.push_back(transform * Vec3f(-1.0f, 1.0f, -1.0f));

	texCoords.push_back(Vec2f(4.0f * invDims.x + texOffset.x, invDims.y + texOffset.y));
	texCoords.push_back(Vec2f(3.0f * invDims.x + texOffset.x, invDims.y + texOffset.y));
	texCoords.push_back(Vec2f(3.0f * invDims.x + texOffset.x, 2.0f * invDims.y + texOffset.y));
	texCoords.push_back(Vec2f(4.0f * invDims.x + texOffset.x, 2.0f * invDims.y + texOffset.y));

	for(int i = 0; i < 4; i++)
		normals.push_back(Vec3f(0.0f, 1.0f, 0.0f));

	// -Y
	invDims = invDims_undimensioned;
	invDims.x *= texScalar.x;
	invDims.y *= texScalar.y;

	positions.push_back(transform * Vec3f(1.0f, -1.0f, 1.0f));
	positions.push_back(transform * Vec3f(-1.0f, -1.0f, 1.0f));
	positions.push_back(transform * Vec3f(-1.0f, -1.0f, -1.0f));
	positions.push_back(transform * Vec3f(1.0f, -1.0f, -1.0f));

	texCoords.push_back(Vec2f(2.0f * invDims.x + texOffset.x, invDims.y + texOffset.y));
	texCoords.push_back(Vec2f(invDims.x + texOffset.x, invDims.y + texOffset.y));
	texCoords.push_back(Vec2f(invDims.x + texOffset.x, 2.0f * invDims.y + texOffset.y));
	texCoords.push_back(Vec2f(2.0f * invDims.x + texOffset.x, 2.0f * invDims.y + texOffset.y));

	for(int i = 0; i < 4; i++)
		normals.push_back(Vec3f(0.0f, -1.0f, 0.0f));
}

void WeaponFactory::Create(const std::string &baseCategory,
	bool wiring,
	int initNumBranches, float maxNumBranches, float minNumBranches,
	float preferBackFactor, float doNotPreferSidesFactor,
	const Vec3f &scaling, float branchSizeDecreaseMultiplier, const AABB &maxBranchBounds)
{
	// Create SPT
	m_geomSPT.Create(maxBranchBounds);

	// VBO Arrays
	std::vector<Vec3f> positions;
	std::vector<Vec2f> texCoords;
	std::vector<Vec3f> normals;

	m_currentRandomTextureRegion = baseCategory;
	m_scalar = scaling;
	m_branchShrinkFactor = branchSizeDecreaseMultiplier;

	// Base shape and gun barrel
	m_baseBoxScale = Vec3f(m_generatedGeomScalar * m_scalar.x * ((rand() % 1000) / 5000.0f + 0.8f),
		m_scalar.y * m_generatedGeomScalar * ((rand() % 1000) / 5000.0f + 0.8f),
		m_scalar.z * m_generatedGeomScalar * ((rand() % 1000) / 5000.0f + 0.8f));

	AABB baseShapeAABB;
	baseShapeAABB.SetCenter(Vec3f(0.0f, 0.0f, 0.0f));
	baseShapeAABB.SetHalfDims(Vec3f(1.0f, 1.0f, 1.0f));

	Matrix4x4f baseTransform(Matrix4x4f::ScaleMatrix(m_baseBoxScale));

	m_backMostPos = Vec3f(-m_baseBoxScale.x, 0.0f, 0.0f);

	// Scopes
	m_foundScopePos = false;
	m_scopePos = Vec3f(0.0f, m_baseBoxScale.y, 0.0f);
	m_selectedScope = &m_pWeapon->m_pAssetPack->GetRandomModel("scopes")->m_model;

	baseShapeAABB = baseShapeAABB.GetTransformedAABB(baseTransform);

	GeomOccupant* newGeomOccupant = new GeomOccupant();
	newGeomOccupant->SetAABB(baseShapeAABB);
	m_geom.push_back(std::unique_ptr<GeomOccupant>(newGeomOccupant));
	m_geomSPT.Add(newGeomOccupant);

	Weapon::ModelAndTransform barrel;
	barrel.m_transform = Matrix4x4f::TranslateMatrix(Vec3f(m_baseBoxScale.x, 0.0f, 0.0f));

	std::stringstream ss;
	ss << baseCategory << "_" << "barrels";
	barrel.pModel = &m_pWeapon->m_pAssetPack->GetRandomModel(ss.str())->m_model;

	m_pWeapon->m_models.push_back(barrel);

	m_pWeapon->m_barrelTipPos = Vec3f(barrel.pModel->GetAABB().GetDims().x + m_baseBoxScale.x, 0.0f, 0.0f);

	m_maxBranchBounds = maxBranchBounds;

	// Start branching
	for(int i = 0; i < initNumBranches; i++)
		BranchGeom(baseShapeAABB, 1.0f, maxNumBranches, minNumBranches, preferBackFactor, doNotPreferSidesFactor);

	// Put stock at backmost position
	Weapon::ModelAndTransform stock;
	stock.m_transform = Matrix4x4f::TranslateMatrix(m_backMostPos);
	stock.pModel = &m_pWeapon->m_pAssetPack->GetRandomModel("stocks")->m_model;

	m_pWeapon->m_models.push_back(stock);

	// If a scope position was found for the selected scope, add it
	if(m_foundScopePos)
	{
		Weapon::ModelAndTransform scopeMaT;
		scopeMaT.m_transform = Matrix4x4f::TranslateMatrix(m_scopePos);
		scopeMaT.pModel = m_selectedScope;

		m_pWeapon->m_models.push_back(scopeMaT);
	}

	// Generate base geometry
	m_pWeapon->m_genGeomBounds = m_geom[0]->GetAABB();

	for(unsigned int i = 0, size = m_geom.size(); i < size; i++)
	{
		// Make sure doesn't intersect trigger
		const AABB &geomAABB = m_geom[i]->GetAABB();

		GenGeom_Box(Matrix4x4f::TranslateMatrix(geomAABB.GetCenter()) * Matrix4x4f::ScaleMatrix(geomAABB.GetHalfDims()),
			geomAABB.GetHalfDims(), m_pWeapon->m_pAssetPack->GetRandomTextureOffset(m_currentRandomTextureRegion), positions, texCoords, normals);

		// Expand bounding box
		if(geomAABB.m_lowerBound.x < m_pWeapon->m_genGeomBounds.m_lowerBound.x)
			m_pWeapon->m_genGeomBounds.m_lowerBound.x = geomAABB.m_lowerBound.x;
	
		if(geomAABB.m_lowerBound.y < m_pWeapon->m_genGeomBounds.m_lowerBound.y)
			m_pWeapon->m_genGeomBounds.m_lowerBound.y = geomAABB.m_lowerBound.y;

		if(geomAABB.m_upperBound.x > m_pWeapon->m_genGeomBounds.m_upperBound.x)
			m_pWeapon->m_genGeomBounds.m_upperBound.x = geomAABB.m_upperBound.x;

		if(geomAABB.m_upperBound.y > m_pWeapon->m_genGeomBounds.m_upperBound.y)
			m_pWeapon->m_genGeomBounds.m_upperBound.y = geomAABB.m_upperBound.y;
	}

	// Ray cast geometry SPT to find trigger position in a specified region in front of stock
	float triggerCenter = (m_minTriggerOffsetFromStock + m_maxTriggerOffsetFromStock) / 2.0f;
	Vec3f castOrigin(m_backMostPos + Vec3f(triggerCenter, -100.0f, 0.0f));
	Vec3f castDir(Vec3f(0.0f, 1.0f, 0.0f));

	Vec3f triggerPos(RayCastGeom(castOrigin, castDir));

	// Couldn't find, work outwards
	if(triggerPos == castOrigin)
	{
		float range = (m_maxTriggerOffsetFromStock - m_minTriggerOffsetFromStock) / 2.0f;

		for(float dPosition = 0.0f; dPosition < range; dPosition += m_positionTestIncrement)
		{
			triggerPos = RayCastGeom(castOrigin + Vec3f(dPosition, 0.0f, 0.0f), castDir);

			if(triggerPos != castOrigin)
				break;

			triggerPos = RayCastGeom(castOrigin + Vec3f(-dPosition, 0.0f, 0.0f), castDir);

			if(triggerPos != castOrigin)
				break;
		}
	}

	if(triggerPos == castOrigin)
	{
		std::cout << "Could not find trigger position. Using default position" << std::endl;

		triggerPos = m_backMostPos + Vec3f(triggerCenter, m_pWeapon->m_genGeomBounds.m_lowerBound.y + 0.05f, 0.0f);
	}

	// Place trigger
	Weapon::ModelAndTransform trigger;
	trigger.m_transform = Matrix4x4f::TranslateMatrix(triggerPos);
	trigger.pModel = &m_pWeapon->m_pAssetPack->GetRandomModel("triggers")->m_model;

	m_pWeapon->m_models.push_back(trigger);

	// Find hand location
	// Start at center, if cannot find spot, go outwards in both directions
	float centerHandOffset = (m_minHandTriggerOffset + m_maxHandTriggerOffset) / 2.0f;
	castOrigin = triggerPos + Vec3f(centerHandOffset, -100.0f, 0.0f);
	
	// Cast dir unchanged
	m_pWeapon->m_handPos = RayCastGeom(castOrigin, castDir);

	// Couldn't find, work outwards
	if(m_pWeapon->m_handPos == castOrigin)
	{
		float range = (m_maxHandTriggerOffset - m_minHandTriggerOffset) / 2.0f;

		for(float dPosition = 0.0f; dPosition < range; dPosition += m_positionTestIncrement)
		{
			m_pWeapon->m_handPos = RayCastGeom(castOrigin + Vec3f(dPosition, 0.0f, 0.0f), castDir);

			if(m_pWeapon->m_handPos != castOrigin)
				break;

			m_pWeapon->m_handPos = RayCastGeom(castOrigin + Vec3f(-dPosition, 0.0f, 0.0f), castDir);

			if(m_pWeapon->m_handPos != castOrigin)
				break;
		}
	}

	// Still couldn't find position, give up and add random position
	if(m_pWeapon->m_handPos == castOrigin)
	{
		std::cout << "Could not find hand position. Using default position" << std::endl;

		m_pWeapon->m_handPos = triggerPos + Vec3f(centerHandOffset, 0.0f, 0.0f);
	}

	// Expand entire weapon bounding box to encompass all attachments
	m_pWeapon->m_entireWeaponBounds = m_pWeapon->m_genGeomBounds;

	for(unsigned int i = 0, size = m_pWeapon->m_models.size(); i < size; i++)
	{
		const AABB &modelAABB = m_pWeapon->m_models[i].pModel->GetAABB().GetTransformedAABB(m_pWeapon->m_models[i].m_transform);

		// Expand bounding box
		if(modelAABB.m_lowerBound.x < m_pWeapon->m_entireWeaponBounds.m_lowerBound.x)
			m_pWeapon->m_entireWeaponBounds.m_lowerBound.x = modelAABB.m_lowerBound.x;
	
		if(modelAABB.m_lowerBound.y < m_pWeapon->m_entireWeaponBounds.m_lowerBound.y)
			m_pWeapon->m_entireWeaponBounds.m_lowerBound.y = modelAABB.m_lowerBound.y;

		if(modelAABB.m_upperBound.x > m_pWeapon->m_entireWeaponBounds.m_upperBound.x)
			m_pWeapon->m_entireWeaponBounds.m_upperBound.x = modelAABB.m_upperBound.x;

		if(modelAABB.m_upperBound.y > m_pWeapon->m_entireWeaponBounds.m_upperBound.y)
			m_pWeapon->m_entireWeaponBounds.m_upperBound.y = modelAABB.m_upperBound.y;
	}

	// Generate VBOs
	m_pWeapon->m_numVertices = positions.size();

	m_pWeapon->m_positions.Create();
	m_pWeapon->m_positions.Bind(GL_ARRAY_BUFFER);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * positions.size(), &positions[0], GL_STATIC_DRAW);
	m_pWeapon->m_positions.Unbind();

	m_pWeapon->m_texCoords.Create();
	m_pWeapon->m_texCoords.Bind(GL_ARRAY_BUFFER);
	glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(float) * texCoords.size(), &texCoords[0], GL_STATIC_DRAW);
	m_pWeapon->m_texCoords.Unbind();

	m_pWeapon->m_normals.Create();
	m_pWeapon->m_normals.Bind(GL_ARRAY_BUFFER);
	glBufferData(GL_ARRAY_BUFFER,  3 * sizeof(float) * normals.size(), &normals[0], GL_STATIC_DRAW);
	m_pWeapon->m_normals.Unbind();
}

void WeaponFactory::SpamAttachments(const std::string &baseCategory)
{
	std::stringstream ss;
	ss << baseCategory << "_" << "decoration";
}

Vec3f WeaponFactory::RayCastGeom(const Vec3f &origin, const Vec3f &direction)
{
	OctreeOccupant* result;

	float t;

	m_geomSPT.Query_Ray(result, t, origin, direction);

	if(result == NULL) // No position found
		return origin;

	return origin + direction * t;
}

void WeaponFactory::BranchGeom(const AABB &prevLocalBox,
		float downScalar,
		float maxNumBranches, float minBranches,
		float preferBackFactor, float doNotPreferSidesFactor)
{
	// Random position in box
	Vec3f prevHalfDims(prevLocalBox.GetHalfDims());
	Vec3f randPosInBox;
	Vec3f normal;

	// Randomly max or min one dimension and have others random
	int side = rand() % 6;

	// Un-prefer sides
	bool unpreferedSides = false;

	if(side != 3 && side != 0)
	{
		if((rand() % 1000) / 1000.0f > doNotPreferSidesFactor)
		{
			unpreferedSides = true;

			if(rand() % 2 == 0)
				side = 0;
			else
				side = 3;
		}
	}

	if(side == 0) // +X, front of gun - may choose other according to decrease factor
	{
		if((rand() % 1000) / 1000.0f > preferBackFactor)
		{
			if(unpreferedSides)
				side = 3;
			else
				side = rand() % 5 + 1;
		}
	}

	switch(side)
	{
	case 0:
		randPosInBox = Vec3f(prevHalfDims.x, 
			((1000 - rand() % 2000) / 1000.0f) * prevHalfDims.y, 
			((1000 - rand() % 2000) / 1000.0f) * prevHalfDims.z);
		
		normal = Vec3f(1.0f, 0.0f, 0.0f);

		break;

	case 1:
		randPosInBox = Vec3f(((1000 - rand() % 2000) / 1000.0f) * prevHalfDims.x, 
			prevHalfDims.y, 
			((1000 - rand() % 2000) / 1000.0f) * prevHalfDims.z);

		normal = Vec3f(0.0f, 1.0f, 0.0f);

		break;

	case 2:
		randPosInBox = Vec3f(((1000 - rand() % 2000) / 1000.0f) * prevHalfDims.x, 
			((1000 - rand() % 2000) / 1000.0f) * prevHalfDims.y, 
			prevHalfDims.z);

		normal = Vec3f(0.0f, 0.0f, 1.0f);

		break;

	case 3:
		randPosInBox = Vec3f(-prevHalfDims.x, 
			((1000 - rand() % 2000) / 1000.0f) * prevHalfDims.y, 
			((1000 - rand() % 2000) / 1000.0f) * prevHalfDims.z);

		normal = Vec3f(-1.0f, 0.0f, 0.0f);

		break;

	case 4:
		randPosInBox = Vec3f(((1000 - rand() % 2000) / 1000.0f) * prevHalfDims.x, 
			-prevHalfDims.y, 
			((1000 - rand() % 2000) / 1000.0f) * prevHalfDims.z);

		normal = Vec3f(0.0f, -1.0f, 0.0f);

		break;

	case 5:
		randPosInBox = Vec3f(((1000 - rand() % 2000) / 1000.0f) * prevHalfDims.x, 
			((1000 - rand() % 2000) / 1000.0f) * prevHalfDims.y, 
			-prevHalfDims.z);

		normal = Vec3f(0.0f, 0.0f, -1.0f);

		break;
	}

	// Use surface normal to get new box branch center point
	AABB currentBox;

	Vec3f scale(m_scalar.x * m_generatedGeomScalar * downScalar * ((rand() % 1000) / 2000.0f + 0.5f),
		m_scalar.y * m_generatedGeomScalar * downScalar * ((rand() % 1000) / 2000.0f + 0.5f),
		m_scalar.z * m_generatedGeomScalar * downScalar * ((rand() % 1000) / 2000.0f + 0.5f));

	currentBox.SetCenter(Vec3f(0.0f, 0.0f, 0.0f));
	currentBox.SetHalfDims(Vec3f(1.0f, 1.0f, 1.0f));

	Matrix4x4f currentTransform(Matrix4x4f::TranslateMatrix(prevLocalBox.GetCenter() + randPosInBox + normal * scale) * Matrix4x4f::ScaleMatrix(scale));

	currentBox = currentBox.GetTransformedAABB(currentTransform);

	// If box is out of bounds, break
	if(!m_maxBranchBounds.Contains(currentBox))
		return;

	// If block obstructs a model, break
	for(int i = 0, size = m_pWeapon->m_models.size(); i < size; i++)
	{
		if(m_pWeapon->m_models[i].pModel->GetAABB().GetTransformedAABB(m_pWeapon->m_models[i].m_transform).Intersects(currentBox))
			return;
	}

	// Find backmost position for stock
	if(currentBox.m_lowerBound.x < m_backMostPos.x)
	{
		// Only if this position can keep the stock in line with the root box (in line with barrel somewhat)
		if(fabsf(currentBox.GetCenter().y) < m_baseBoxScale.y && fabsf(currentBox.GetCenter().z) < m_baseBoxScale.z)
			m_backMostPos = currentBox.GetCenter() + Vec3f(-currentBox.GetHalfDims().x, 0.0f, 0.0f);
		else
			// Do not generate this block, don't want something behind stock
			return;
	}

	// Find top most (scope) position
	Vec3f possibleScopePos(currentBox.GetCenter() + Vec3f(0.0f, currentBox.GetHalfDims().y, 0.0f));

	if(possibleScopePos.y > m_scopePos.y)
	{
		AABB scopeAABB(m_selectedScope->GetAABB().GetTransformedAABB(Matrix4x4f::TranslateMatrix(possibleScopePos)));

		std::vector<OctreeOccupant*> result;

		m_geomSPT.Query_Region(scopeAABB, result);

		if(result.empty())
		{
			m_scopePos = possibleScopePos;
			m_foundScopePos = true;
		}
	}

	// Add Geometry to SPT
	GeomOccupant* newGeomOccupant = new GeomOccupant();
	newGeomOccupant->SetAABB(currentBox);

	m_geom.push_back(std::unique_ptr<GeomOccupant>(newGeomOccupant));
	m_geomSPT.Add(newGeomOccupant);

	// Branch randomly
	if(maxNumBranches <= 0)
		return;

	float newDownScalar = downScalar - downScalar * m_branchShrinkFactor;

	if(newDownScalar < m_minBranchScale)
		return;

	int minBranchesi = static_cast<int>(minBranches);
	int numBranches = rand() % (static_cast<int>(maxNumBranches) - minBranchesi) + minBranchesi;

	float doNotPreferSides = doNotPreferSidesFactor - m_doNotPreferSidesDecreaseFactor * ((rand() % 1000) / 1000.0f * (1.0f - doNotPreferSidesFactor));

	if(doNotPreferSides < 0.0f)
		doNotPreferSides = 0.0f;

	for(int i = 0; i < numBranches; i++)
		BranchGeom(currentBox, newDownScalar, maxNumBranches - m_maxBranchDecreaseFactor, 
			minBranches - m_minBranchDecreaseFactor,
			preferBackFactor + m_preferBackIncreaseFactor * ((rand() % 1000) / 1000.0f * (1.0f - preferBackFactor)),
			doNotPreferSidesFactor - m_doNotPreferSidesDecreaseFactor * ((rand() % 1000) / 1000.0f * (1.0f - doNotPreferSidesFactor)));
}

