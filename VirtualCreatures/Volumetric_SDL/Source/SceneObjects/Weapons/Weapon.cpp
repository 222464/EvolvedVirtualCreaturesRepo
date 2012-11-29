#include <SceneObjects/Weapons/Weapon.h>

#include <Scene/Scene.h>

#include <SceneObjects/Weapons/SceneObject_Shell.h>

#include <SceneObjects/SceneObject_Decal.h>

#include <SceneObjects/Particles/SceneObject_ParticleEmitter_Sprite_Pyramid.h>

#include <SceneObjects/SceneObject_Sound.h>

#include <SceneObjects/Enemies/SceneObject_Enemy.h>

#include <Utilities/UtilFuncs.h>

#include <sstream>

#include <assert.h>

Weapon::Weapon()
	: m_created(false), m_pShellModel(NULL), m_pAssetPack(NULL),
	m_damage(10.0f)
{
	Wire::EndPoint p1;
	p1.m_pos = Vec3f(0.0f, 0.0f, -1.8f);
	p1.m_dir = Vec3f(0.34f, -0.54f, 0.5f).Normalize() * 2.8f;

	Wire::EndPoint p2;
	p2.m_pos = Vec3f(0.2f, 0.7f, 0.3f);
	p2.m_dir = -p1.m_dir;

	m_testWire.Generate(16, 8, p1, p2, 0.04f);
}

void Weapon::Render(const Matrix4x4f &baseTransform)
{
	m_pScene->SetWorldMatrix(baseTransform);
	
	/*m_pScene->SetDiffuseColor(Color3f(0.1f, 0.1f, 0.1f));
	m_pScene->SetSpecularColor(1.0f);

	m_testWire.Render();

	m_pScene->SetDiffuseColor(Color3f(1.0f, 1.0f, 1.0f));
	m_pScene->SetSpecularColor(0.0f);*/

	m_pScene->SetCurrentGBufferRenderShader(Scene::e_bump);
	m_pScene->SetSpecularColor(1.0f);

	// Draw base geometry
	glEnable(GL_VERTEX_ARRAY);
	glEnable(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_NORMAL_ARRAY);

	m_positions.Bind(GL_ARRAY_BUFFER);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	m_texCoords.Bind(GL_ARRAY_BUFFER);
	glTexCoordPointer(2, GL_FLOAT, 0, NULL);

	m_normals.Bind(GL_ARRAY_BUFFER);
	glNormalPointer(GL_FLOAT, 0, NULL);

	m_pAssetPack->BindRandomWeaponTextures();

	glDrawArrays(GL_QUADS, 0, m_numVertices);

	m_normals.Unbind();
	m_texCoords.Unbind();
	m_positions.Unbind();

	glDisable(GL_VERTEX_ARRAY);
	glDisable(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_NORMAL_ARRAY);

	// Draw models
	for(unsigned int i = 0, size = m_models.size(); i < size; i++)
		m_models[i].pModel->Render(baseTransform * m_models[i].m_transform);
}

const Vec3f &Weapon::GetHandPos() const
{
	return m_handPos;
}

const AABB &Weapon::GetAABB()
{
	return m_entireWeaponBounds;
}

float Weapon::GetRecoilingTime()
{
	return 3.0f;
}

float Weapon::GetReturningTime()
{
	return 4.0f;
}

void Weapon::Shoot(const Vec3f m_barrelTipPos, const Quaternion &baseAngle, const Vec3f &baseVel)
{
	// Ray cast
	Vec3f dir(baseAngle * Vec3f(0.0f, 0.0f, -1.0f));

	// Add some randomness, decrease accuracy
	dir += Vec3f(Randf(-1.0f, 1.0f), Randf(-1.0f, 1.0f), Randf(-1.0f, 1.0f)) / 50.0f;

	btVector3 btDir(bt(dir));
	btVector3 start(bt(m_barrelTipPos));
	btVector3 end(start + btDir * 50.0f);

	btCollisionWorld::ClosestRayResultCallback rayCallBack(start, end);

	m_pPhysicsWorld->m_pDynamicsWorld->rayTest(start, end, rayCallBack);

	if(rayCallBack.hasHit())
	{
		Vec3f hitPos(cons(rayCallBack.m_hitPointWorld));
		Vec3f hitNormal(cons(rayCallBack.m_hitNormalWorld));

		// ---------------------------- Spawn a Bullet Hole ----------------------------

		if(rayCallBack.m_collisionObject->getUserPointer() != NULL)
		{
			SceneObject* pHitObject = static_cast<SceneObject*>(rayCallBack.m_collisionObject->getUserPointer());

			if(pHitObject->m_unmanagedName == "enemy")
			{
				SceneObject_Enemy* pEnemy = static_cast<SceneObject_Enemy*>(pHitObject);
				pEnemy->m_health -= m_damage;

				btVector3 newStart(rayCallBack.m_hitPointWorld + btDir * 0.5f);
				btVector3 newEnd(newStart + btDir * 10.0f);

				// Fall
				newEnd.setY(newEnd.getY() - 1.0f - Randf() * 3.0f);

				// Random XZ offsets
				newEnd.setX(newEnd.getX() + Randf(-1.0f, 1.0f));
				newEnd.setZ(newEnd.getZ() + Randf(-1.0f, 1.0f));

				// Perform another raycast to spawn blood splat
				btCollisionWorld::ClosestRayResultCallback rayCallBack_toWall(newStart, newEnd);

				m_pPhysicsWorld->m_pDynamicsWorld->rayTest(newStart, newEnd, rayCallBack_toWall);

				if(rayCallBack_toWall.hasHit())
				{
					SceneObject* pHitObject2 = static_cast<SceneObject*>(rayCallBack_toWall.m_collisionObject->getUserPointer());

					if(pHitObject2->m_unmanagedName == "world")
					{
						Vec3f hitPos_wall(cons(rayCallBack_toWall.m_hitPointWorld));
						Vec3f hitNormal_wall(cons(rayCallBack_toWall.m_hitNormalWorld));

						// Spawn blood splatter
						SceneObject_Decal* bloodSplat = new SceneObject_Decal();

						m_pScene->Add(bloodSplat, false);

						// Select random blood texture
						const int numTextures = 8;

						std::ostringstream os;

						int splat = rand() % numTextures + 1;

						os << "data/weapons/bloodSplats/bloodSplat" << splat << ".png";

						if(!bloodSplat->Create(os.str(), hitPos_wall, hitNormal_wall, 1500.0f, 1.5f)) // Position slightly offset to avoid z fighting
							abort();

						// Bump map
						os.str("");	
						os << "data/weapons/bloodSplats/bloodSplat" << splat << ".bmp";

						if(!bloodSplat->AddNormalMap(os.str()))
							abort();

						bloodSplat->m_specularColor = 0.5f;
					}
				}
			}
			else if(pHitObject->m_unmanagedName == "dynProp") // Dynamic physics prop
			{
				SceneObject_Prop_Physics_Dynamic* pProp = static_cast<SceneObject_Prop_Physics_Dynamic*>(pHitObject);

				SceneObject_Decal* bulletHole = new SceneObject_Decal();

				m_pScene->Add(bulletHole, false);

				if(!bulletHole->Create("data/weapons/bulletHoles/ballistic1.png", pProp, hitPos, hitNormal, 400.0f, 0.125f)) // Position slightly offset to avoid z fighting
					abort();

				if(!bulletHole->AddNormalMap("data/weapons/bulletHoles/ballistic1.bmp"))
					abort();

				bulletHole->m_specularColor = 0.4f;
			}
			else
			{
				SceneObject_Decal* bulletHole = new SceneObject_Decal();

				m_pScene->Add(bulletHole, false);

				if(!bulletHole->Create("data/weapons/bulletHoles/ballistic1.png", hitPos, hitNormal, 400.0f, 0.25f)) // Position slightly offset to avoid z fighting
					abort();

				if(!bulletHole->AddNormalMap("data/weapons/bulletHoles/ballistic1.bmp"))
					abort();

				bulletHole->m_specularColor = 0.4f;
			}

			// ---------------------------- Add Particle System ----------------------------

			SceneObject_ParticleEmitter_Sprite_Pyramid* pParticleSystem = new SceneObject_ParticleEmitter_Sprite_Pyramid();

			Asset* pAsset;

			if(pHitObject->m_unmanagedName == "enemy")
			{
				if(!m_pScene->GetAssetManager_AutoCreate("tex", Asset_Texture::Asset_Factory)->GetAsset("data/particles/blood1.png", pAsset))
					abort();

				pParticleSystem->m_position = hitPos;
				pParticleSystem->m_direction = hitNormal;

				pParticleSystem->m_minDespawnTime = 9.0f;
				pParticleSystem->m_maxDespawnTime = 15.0f;

				pParticleSystem->m_minSpeed = 0.1f;
				pParticleSystem->m_maxSpeed = 0.2f;

				pParticleSystem->m_size = 0.7f;
			}
			else
			{
				if(!m_pScene->GetAssetManager_AutoCreate("tex", Asset_Texture::Asset_Factory)->GetAsset("data/particles/debris1.png", pAsset))
					abort();

				pParticleSystem->m_position = hitPos;
				pParticleSystem->m_direction = hitNormal;

				pParticleSystem->m_minDespawnTime = 9.0f;
				pParticleSystem->m_maxDespawnTime = 15.0f;

				pParticleSystem->m_minSpeed = 0.1f;
				pParticleSystem->m_maxSpeed = 0.2f;

				pParticleSystem->m_size = 0.7f;
			}

			Asset_Texture* pParticleTexture = static_cast<Asset_Texture*>(pAsset);

			assert(pParticleTexture != NULL);

			Sprite s;

			s.Create(pParticleTexture, 0.15f, 1, 1, 1);

			pParticleSystem->AddSprite(s);

			pParticleSystem->AddAffector(Affector_Gravity);

			pParticleSystem->m_emit = false;
			pParticleSystem->m_autoDestruct = true;

			m_pScene->Add(pParticleSystem, true);

			if(pHitObject->m_unmanagedName == "enemy")
				pParticleSystem->SpawnBurst(rand() % 20 + 30);
			else
				pParticleSystem->SpawnBurst(rand() % 4 + 4);

			// ---------------------------- Add Impact Sound ----------------------------

			SceneObject_Sound* pSound = new SceneObject_Sound();

			m_pScene->Add(pSound, false);

			int randSoundIndex = rand() % 3;

			switch(randSoundIndex)
			{
			case 0:
				if(!pSound->Create("data/sounds/bulletHits/ballisticWallHit1.wav"))
					abort();
				break;
			case 1:
				if(!pSound->Create("data/sounds/bulletHits/ballisticWallHit2.wav"))
					abort();
				break;
			case 2:
				if(!pSound->Create("data/sounds/bulletHits/ballisticWallHit3.wav"))
					abort();
				break;
			}

			pSound->m_soundSource.SetPosition(hitPos);
			pSound->m_soundSource.SetGain(0.7f);
			pSound->m_soundSource.Play();

			pSound->m_autoDestroy = true;
		}
	}
}

void Weapon::EjectShell(const Matrix4x4f &baseTransform, const Quaternion &baseAngle, const Vec3f &baseVel)
{
	Vec3f vel(baseAngle * Vec3f(Randf(0.0f, 0.02f) + 0.02f, Randf(0.0f, 0.005f) + 0.01f, Randf(-0.005f, 0.005f)));
	m_pScene->Add(new SceneObject_Shell(m_pShellModel, baseTransform * GetEjectPos(), baseVel + vel, baseAngle.Conjugate()), false);
}

Vec3f Weapon::GetEjectPos() const
{
	return m_handPos + Vec3f(0.1f, 0.2f, 0.0f);
}

const Vec3f &Weapon::GetBarrelTipPos() const
{
	return m_barrelTipPos;
}

Sound_Effect* Weapon::GetShootSound() const
{
	return m_pShootSound;
}

Asset_Texture* Weapon::GetMuzzleFlashSpriteSheet() const
{
	return m_pMuzzleFlashSpriteSheet;
}