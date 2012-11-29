#include <Renderer/SDL_OpenGL.h>
#include <Scene/Scene.h>
#include <World/World.h>

#include <SceneObjects/SceneObject_Player.h>

#include <Renderer/Shader/Shader.h>
#include <Renderer/ShadowMap/ShadowCubeMap.h>

#include <Renderer/RenderUtils.h>

#include <SceneEffects/SceneEffect_Lighting.h>
#include <SceneEffects/Light_Spot_Shadowed.h>
#include <SceneEffects/Light_Directional_Shadowed.h>
#include <SceneEffects/SceneEffect_GodRay.h>

#include <World/LuaWorldInterface.h>

#include <time.h>

#include <SceneObjects/SceneObject_SpiralStairs.h>
#include <Renderer/Model_MD5/Model_MD5.h>

#include <SceneObjects/Weapons/SceneObject_Weapon.h>

#include <SceneObjects/Physics/SceneObject_PhysicsBox.h>

#include <SceneObjects/Particles/SceneObject_ParticleEmitter_Sprite_Pyramid.h>

#include <SceneObjects/SceneObject_Prop_Transparent.h>

#include <SceneObjects/Enemies/SceneObject_Enemy_Swarmer.h>

#include <Sound/SoundSystem.h>

#include <SceneEffects/Water/TestGrouping.h>
#include <SceneEffects/Water/SceneEffect_WaterRender.h>

#include <SceneObjects/Physics/SceneObject_Prop_Physics_Static.h>
#include <SceneObjects/Physics/SceneObject_Prop_Physics_Dynamic.h>

#include <SceneObjects/Enemies/SceneObject_Enemy_Spawner.h>

#include <SceneObjects/Weapons/WeaponFactory.h>

#include <SceneEffects/SceneEffect_HUD.h>

#include <SceneObjects/VirtualCreatures/SceneObject_VirtualCreatureSimulator.h>

#include <SceneObjects/SceneObject_SkyBox.h>

#include <Renderer/Model_MD5/Model_MD5.h>

#include <assert.h>

#include <fstream>

int main(int argc, char* args[])
{
	// Create the window
	Window win;
	win.Create(1200, 800, false);
	win.SetViewport();
	win.SetProjection();
	
	glEnable(GL_CULL_FACE);

	glClearDepth(1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);

	Scene scene;

	scene.Create(&win,
		"NONE data/shaders/gBufferRender.vert data/shaders/gBufferRender.frag",
		"NONE data/shaders/gBufferRenderBump.vert data/shaders/gBufferRenderBump.frag",
		"data/shaders/textures/white.png",
		"data/shaders/models/normalizedCube.obj");

	// ------------------------------- MD5 Batch Renderer Setup -------------------------------

	Model_MD5_BatchRenderer* pMD5BatchRenderer = static_cast<Model_MD5_BatchRenderer*>(scene.GetBatchRenderer(MODEL_MD5_BATCHRENDERER_NAME, Model_MD5_BatchRenderer::Model_MD5_BatchRendererFactory));

	assert(pMD5BatchRenderer != NULL);

	if(!pMD5BatchRenderer->Create("NONE data/shaders/gBufferRender_skinningMD5.vert data/shaders/gBufferRender_skinningMD5.frag",
		"NONE data/shaders/gBufferRenderBump_skinningMD5.vert data/shaders/gBufferRenderBump_skinningMD5.frag"))
		abort();

	// --------------------------------------------------------------------------------

	bool ocCull = false;
	scene.SetRenderMode(ocCull);

	// First scene objects
	LUA_API_BindScene(&scene);
	LuaVirtualMachine luaVM;
	World_LUA_API_Register(&luaVM);

	luaVM.RunFile("data/generators/generator_1flatland.lua");

	std::cout << "Lua done. Generating chunks..." << std::endl;

	s_pWorld->GenerateAllChunks();

	std::cout << "Chunk generation complete." << std::endl;

	SceneObject_Player* pPlayer = new SceneObject_Player();

	scene.Add(pPlayer, true, "player");

	// Initial inputs
	scene.m_inputHandler.GetInputs();

	std::ofstream of("dfsdfdf.txt");

	for(int k = 0; k < 8; k++)
	{
		Vec3f n(Randf(-1.0f, 1.0f), Randf(-1.0f, 1.0f), Randf(0.0f, 1.0f));

		n.NormalizeThis();

		n *= Randf(0.05f, 1.0f);

		float scale = static_cast<float>(k) / 8.0f;

		scale = Lerp(0.1f, 1.0f, scale * scale);

		n *= scale;

		of << "vec3(" << n.x << ", " << n.y << ", " << n.z << "),\n";
	}

	of.close();

	SceneEffect_Lighting* pLighting = static_cast<SceneEffect_Lighting*>(scene.GetNamed_Effect("lighting"));

	assert(pLighting != NULL);

	scene.Add(new SceneObject_PhysicsBox(Vec3f(16.5f, 32.0f, 16.5f)), false);

	SoundSystem* pSoundSystem = static_cast<SoundSystem*>(scene.GetNamed_Effect("sndsys"));

	assert(pSoundSystem != NULL);

	/*Sound_Streamed ogg;

	if(!ogg.OpenStream("data/music/industrialAtmosphere.ogg"))
		abort();

	SoundSource source;
	source.SetSound(&ogg);
	pSoundSystem->Add(&ogg);
	ogg.m_looping = true;

	source.SetGain(0.27f);
	source.Play();*/

	World* pWorld = static_cast<World*>(scene.GetNamed_SceneObject("world"));

	assert(pWorld != NULL);

	/*SceneEffect_WaterRender* pWaterRender = static_cast<SceneEffect_WaterRender*>(scene.GetNamed_Effect(WATER_RENDER_NAME));

	assert(pWaterRender != NULL);

	TestGrouping* g = new TestGrouping();

	pWaterRender->AddGrouping(g);*/

	/*for(int p = 0; p < 8; p++)
	{
		SceneObject_Enemy_Swarmer* pSwarmer = new SceneObject_Enemy_Swarmer(Vec3f(200.0f + (float)(rand() % 1000) / 200.0f, 120.0f + (float)(rand() % 1000) / 200.0f, 220.0f + (float)(rand() % 1000) / 200.0f));

		scene.Add(pSwarmer, true);
	}*/

	//SceneObject_Enemy_Swarmer* pSwarmer = new SceneObject_Enemy_Swarmer(Vec3f(200.0f + (float)(rand() % 1000) / 200.0f, 120.0f + (float)(rand() % 1000) / 200.0f, 220.0f + (float)(rand() % 1000) / 200.0f));

	//scene.Add(pSwarmer, true);

	//scene.Add(new SceneObject_Enemy_Spawner(Vec3f(200.0f, 120.0f, 200.0f)), true);

	//SceneEffect_HUD* pHUD = new SceneEffect_HUD(s_pWorld);

	//s_pScene->Add(pHUD, 9);

	/*SceneObject_SkyBox* pSkyBox = new SceneObject_SkyBox();

	scene.Add(pSkyBox, false);

	if(!pSkyBox->Create("data/skyboxes/clouds/clouds", ".png"))
		abort();*/

	Light_Directional* pDL = new Light_Directional();//(4096, 2, 250.0f, 0.6f, &scene);

	pDL->SetDirection(Vec3f(0.0f, -1.0f, 0.0f));
	pDL->SetIntensity(2.25f);

	pLighting->AddLight_Directional(pDL);

	SceneObject_VirtualCreatureSimulator* pSimulator = new SceneObject_VirtualCreatureSimulator();

	scene.Add(pSimulator, false);

	if(!pSimulator->Create("data/VC_Config.txt"))
		abort();

	bool hyperMode = false;

	while(!scene.m_inputHandler.Quit() && !scene.m_inputHandler.GetCurrentKeyState(SDLK_q))
	{
		if(hyperMode)
		{
			// Logic only at fixed time step
			scene.m_frameTimer.m_timeMultiplier = 2.0f; // 30 fps simulation

			scene.m_inputHandler.GetInputs();

			scene.Logic();
		}
		else // Render and run at normal frame rate
			scene.Frame();
		
		//pDL->UpdateProjection(&scene);
	//	pDL->RenderToCascades(&scene);

		//pDL->DebugDrawCascade(&scene, 0);

		if(scene.m_inputHandler.KeyReleased(SDLK_b))
			hyperMode = !hyperMode;

		if(scene.m_inputHandler.KeyReleased(SDLK_h))
		{
			//scene.Add(new SceneObject_PhysicsBox(pPlayer->GetPosition()), false);
			SceneObject_Prop_Physics_Dynamic* pProp = new SceneObject_Prop_Physics_Dynamic();

			scene.Add(pProp, false);

			pProp->Create("data/models/table.obj", "data/models/table.phy", pPlayer->GetPosition(), Quaternion(0.0f, 1.0f, 0.0f, 0.0f), 10.0f, 0.02f, 0.02f);
		}

		if(scene.m_inputHandler.KeyReleased(SDLK_y))
			scene.Add(new SceneObject_Enemy_Swarmer(pPlayer->GetPosition()), true);

		if(scene.m_inputHandler.KeyReleased(SDLK_u))
		{
			Light_Spot_Shadowed* pShadowedLight = new Light_Spot_Shadowed(512, 512);

			pShadowedLight->SetCenter(pPlayer->GetPosition() - pPlayer->GetViewVec() * 2.0f);
			pShadowedLight->SetDirection(-pPlayer->GetViewVec());
			pShadowedLight->SetIntensity(16.0f);
			pShadowedLight->SetSpreadAngle(pif / 9.0f);

			pLighting->AddLight(pShadowedLight);

			pShadowedLight->RenderToMap(&scene);
		}

		if(scene.m_inputHandler.KeyReleased(SDLK_j))
		{
			Point3i pos(pWorld->HighlightVoxel(pPlayer->GetPosition(), -pPlayer->GetViewVec()));
			pWorld->SetVoxelAndUpdate(-pPlayer->GetViewVec(), pos.x, pos.y, pos.z, 0);
		}

		if(scene.m_inputHandler.KeyReleased(SDLK_i))
			pPlayer->UsePhysics(!pPlayer->UsingPhysics());

		if(scene.m_inputHandler.GetCurrentKeyState(SDLK_k))
		{
			// Increase ambient light level
			if(pLighting->m_ambient.r < 1.0f)
			{
				float dt = scene.m_frameTimer.GetTimeMultiplier();
				pLighting->m_ambient.r += dt * 0.01f;

				if(pLighting->m_ambient.r > 1.0f)
				{
					pLighting->m_ambient.r = 1.0f;
					pLighting->m_ambient.g = 1.0f;
					pLighting->m_ambient.b = 1.0f;
				}
				else
				{
					pLighting->m_ambient.g += dt * 0.01f;
					pLighting->m_ambient.b += dt * 0.01f;
				}		
			}
		}
		else if(scene.m_inputHandler.GetCurrentKeyState(SDLK_l))
		{
			// Decrease ambient light level
			if(pLighting->m_ambient.r > 0.0f)
			{
				float dt = scene.m_frameTimer.GetTimeMultiplier();
				pLighting->m_ambient.r -= dt * 0.01f;

				if(pLighting->m_ambient.r < 0.0f)
				{
					pLighting->m_ambient.r = 0.0f;
					pLighting->m_ambient.g = 0.0f;
					pLighting->m_ambient.b = 0.0f;
				}
				else
				{
					pLighting->m_ambient.g -= dt * 0.01f;
					pLighting->m_ambient.b -= dt * 0.01f;
				}		
			}
		}

		if(scene.m_inputHandler.KeyPressed(SDLK_o))
		{
			ocCull = !ocCull;

			scene.SetRenderMode(ocCull);
		}

		SDL_GL_SwapBuffers();
	}

	scene.Clear();
	scene.ClearAssets();

	return EXIT_SUCCESS;
}