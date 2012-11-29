#include <World/LuaWorldInterface.h>

#include <SceneObjects/SceneObject_Prop.h>
#include <SceneObjects/Physics/SceneObject_Prop_Physics_Static.h>
#include <SceneObjects/Physics/SceneObject_Prop_Physics_Dynamic.h>
#include <SceneObjects/SceneObject_SpiralStairs.h>
#include <SceneObjects/SceneObject_Door.h>
#include <SceneObjects/Physics/SceneObject_PhysicsWorld.h>
#include <SceneObjects/SceneObject_Prop_Transparent.h>
#include <SceneObjects/SceneObject_Water.h>
#include <SceneObjects/SceneObject_Sound.h>
#include <SceneObjects/Enemies/SceneObject_EnemySelector.h>

#include <SceneEffects/SceneEffect_SSAO.h>
#include <SceneEffects/SceneEffect_Lighting.h>
#include <SceneEffects/Light_Spot_Shadowed.h>
#include <SceneEffects/SceneEffect_EdgeBlurAA.h>
#include <SceneEffects/SceneEffect_EmissiveRender.h>
#include <SceneEffects/Transparency/SceneEffect_TransparencyRender.h>
#include <SceneEffects/Water/SceneEffect_WaterRender.h>
#include <SceneEffects/SceneEffect_HDRR.h>
#include <SceneEffects/SceneEffect_HUD.h>

// Enemies
#include <SceneObjects/Enemies/SceneObject_Enemy_Swarmer.h>
#include <SceneObjects/Enemies/SceneObject_Enemy_Flea.h>

#include <Sound/SoundSystem.h>

#include <assert.h>
#include <iostream>

World* s_pWorld = NULL;
Scene* s_pScene = NULL;

NoiseGenerator s_generator;

LuaVirtualMachine::LuaVirtualMachine()
	: m_pLuaVM(NULL), m_stopOnError(true)
{
	Lua_Init();
}

LuaVirtualMachine::~LuaVirtualMachine()
{
	Lua_Destroy();
}

void LuaVirtualMachine::Lua_Init()
{
	assert(m_pLuaVM == NULL);

	// Init lua
	m_pLuaVM = luaL_newstate();

	// Open libraries
	luaL_openlibs(m_pLuaVM);

	// Open libraries
#ifdef LUA_DEBUG
	luaopen_debug(m_pLuaVM);

	// Add debug hook
	//lua_sethook(m_pLuaVM, Lua_Inline_Error_Check, LUA_MASKLINE, 0);
#endif
}

void LuaVirtualMachine::Lua_Destroy()
{
	assert(m_pLuaVM != NULL);

	lua_close(m_pLuaVM);
}

void LuaVirtualMachine::Lua_Report_Errors()
{
	const char* msg = lua_tostring(m_pLuaVM, -1);

	if(msg != NULL)
	{
		std::cerr << "Lua Error: " << msg << std::endl;

		if(m_stopOnError)
			abort();
	}

	lua_pop(m_pLuaVM, -1); // Remove error message
}

void LuaVirtualMachine::RunFile(const std::string &fileName)
{
	luaL_dofile(m_pLuaVM, fileName.c_str());

	Lua_Report_Errors();
}

void LuaVirtualMachine::RunString(const std::string &str)
{
	luaL_dostring(m_pLuaVM, str.c_str());

	Lua_Report_Errors();
}

void LUA_API_BindScene(Scene* pScene)
{
	s_pScene = pScene;
}

void World_LUA_API_Register(LuaVirtualMachine* pLVM)
{
	lua_register(pLVM->m_pLuaVM, "World_Create", World_Create);
	lua_register(pLVM->m_pLuaVM, "World_SetRandomSeed", World_SetRandomSeed);
	lua_register(pLVM->m_pLuaVM, "World_SetSeed", World_SetSeed);
	lua_register(pLVM->m_pLuaVM, "World_SetVoxel", World_SetVoxel);
	lua_register(pLVM->m_pLuaVM, "World_GetVoxel", World_GetVoxel);
	lua_register(pLVM->m_pLuaVM, "World_Perlin", World_Perlin);
	lua_register(pLVM->m_pLuaVM, "World_FillBox", World_FillBox);
	lua_register(pLVM->m_pLuaVM, "World_AddProp", World_AddProp);
	lua_register(pLVM->m_pLuaVM, "World_AddProp_Physics_Static", World_AddProp_Physics_Static);
	lua_register(pLVM->m_pLuaVM, "World_AddProp_Physics_Dynamic", World_AddProp_Physics_Dynamic);
	lua_register(pLVM->m_pLuaVM, "World_AddProp_Transparent", World_AddProp_Transparent);
	lua_register(pLVM->m_pLuaVM, "World_AddLight_Point", World_AddLight_Point);
	lua_register(pLVM->m_pLuaVM, "World_AddLight_Spot", World_AddLight_Spot);
	lua_register(pLVM->m_pLuaVM, "World_AddSpiralStairs", World_AddSpiralStairs);
	lua_register(pLVM->m_pLuaVM, "World_AddDoor", World_AddDoor);
	lua_register(pLVM->m_pLuaVM, "World_AddSound", World_AddSound);
	lua_register(pLVM->m_pLuaVM, "World_AddWater", World_AddWater);
}

int World_Create(lua_State* pLuaState)
{
	if(s_pWorld != NULL)
	{
		std::cerr << "Cannot create more than once!" << std::endl;

		// Failed load, return false for lua
		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	if(s_pScene == NULL)
	{
		std::cerr << "No scene bound!" << std::endl;

		// Failed load, return false for lua
		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	int argc = lua_gettop(pLuaState);

	if(argc != 9)
	{
		std::cerr << "Function \"World_Create\" requires 9 arguments, not " << argc << std::endl;

		// Failed load, return false for lua
		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	// Arguments
	int xDim = static_cast<int>(lua_tonumber(pLuaState, 1));
	int yDim = static_cast<int>(lua_tonumber(pLuaState, 2));
	int zDim = static_cast<int>(lua_tonumber(pLuaState, 3));
	std::string textureFileName_diffuse(lua_tostring(pLuaState, 4));
	std::string textureFileName_specular(lua_tostring(pLuaState, 5));
	std::string textureFileName_bump(lua_tostring(pLuaState, 6));
	std::string descriptorName(lua_tostring(pLuaState, 7));
	int numTexturesInX = static_cast<int>(lua_tonumber(pLuaState, 8));
	int numTexturesInY = static_cast<int>(lua_tonumber(pLuaState, 9));

	lua_remove(pLuaState, 9);

	// Add physics world
	s_pScene->Add(new SceneObject_PhysicsWorld(), false, "physWrld");

	// Get next higher chunk size
	int xChunks = xDim / Chunk::s_chunkSizeX;
	
	if(xChunks % xDim != 0)
		xDim++;

	int yChunks = yDim / Chunk::s_chunkSizeY;
	
	if(yChunks % yDim != 0)
		yDim++;

	int zChunks = zDim / Chunk::s_chunkSizeZ;
	
	if(zChunks % zDim != 0)
		zDim++;

	Chunk::s_numTileTexturesInX = numTexturesInX;
	Chunk::s_numTileTexturesInY = numTexturesInY;

	s_pWorld = new World();

	if(!s_pWorld->Create(xChunks, yChunks, zChunks, textureFileName_diffuse, textureFileName_specular, textureFileName_bump, descriptorName))
	{
		// Failed load, return false for lua
		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	s_pScene->TreeSetup(s_pWorld->GetAABB());
	s_pScene->Add(s_pWorld, false, "world");

	std::cout << "Created world with chunk dimensions " << xChunks << ", " << yChunks << ", " << zChunks << std::endl;
	std::cout << "using the diffuse texture " << textureFileName_diffuse  << " and the specular texture " << textureFileName_specular << " which contain a " << numTexturesInX << " x " << numTexturesInY << " matrix of textures, following descriptor " << descriptorName << std::endl;

	// --------------------------------- Effects ---------------------------------

	// Add light system so can add lights after this call
	SceneEffect_Lighting* pLighting = new SceneEffect_Lighting();

	s_pScene->Add(pLighting, 0, "lighting");

	if(!pLighting->Create("NONE NONE data/shaders/effect_light_point.frag", "NONE NONE data/shaders/effect_light_spot.frag",
		"NONE NONE data/shaders/VSM/VSM_spotLight.frag", "NONE data/shaders/VSM/depthStore.vert data/shaders/VSM/depthStore.frag",
		"NONE NONE data/shaders/VSM/VSM_blur_horizontal.frag", "NONE NONE data/shaders/VSM/VSM_blur_vertical.frag",
		"NONE NONE data/shaders/effect_light_directional.frag", "NONE NONE data/shaders/VSM/VSM_directionalLight.frag", "NONE data/shaders/VSM/depthStore_directional.vert data/shaders/VSM/depthStore_directional.frag",
		"NONE data/shaders/nullShader.vert data/shaders/nullShader.frag", "data/shaders/models/icosphere.obj", "data/shaders/models/cone.obj"))
	{
		// Failed load, return false for lua
		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	/*SceneEffect_SSAO* pSSAO = new SceneEffect_SSAO();

	s_pScene->Add(pSSAO, 1, "ssao");

	if(!pSSAO->Create("NONE NONE data/shaders/SSAO/ssao.frag", "NONE NONE data/shaders/blur_horizontal.frag", "NONE NONE data/shaders/blur_vertical.frag", "data/shaders/SSAO/random.bmp"))
	{
		delete pSSAO;
		
		// Failed load, return false for lua
		lua_pushboolean(pLuaState, 0);

		return 1;
	}*/

	SceneEffect_EmissiveRender* pEmissiveRender = new SceneEffect_EmissiveRender();

	s_pScene->Add(pEmissiveRender, 3, "emRen");

	SoundSystem* pSoundSystem = new SoundSystem();

	s_pScene->Add(pSoundSystem, 4, "sndsys");

	if(!pSoundSystem->Create())
	{
		std::cerr << "ERROR: Could not create sound system!" << std::endl;

		// Failed load, return false for lua
		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	/*SceneEffect_TransparencyRender* pTransparencyRender = new SceneEffect_TransparencyRender();

	s_pScene->Add(pTransparencyRender, 5, TRANSPARENT_RENDER_NAME);

	if(!pTransparencyRender->Create(pLighting, "NONE data/shaders/lighting_forward.vert data/shaders/lighting_forward.frag"))
	{
		std::cerr << "ERROR: Could not create transparency rendering system!" << std::endl;

		// Failed load, return false for lua
		lua_pushboolean(pLuaState, 0);

		return 1;
	}*/

	/*SceneEffect_WaterRender* pWaterRender = new SceneEffect_WaterRender();

	s_pScene->Add(pWaterRender, 6, WATER_RENDER_NAME);

	if(!pWaterRender->Create(pLighting, "NONE data/shaders/waterShader.vert data/shaders/waterShader.frag", "data/shaders/water/bump0.bmp", "data/shaders/water/bump1.bmp"))
	{
		std::cerr << "ERROR: Could not create water rendering system!" << std::endl;

		// Failed load, return false for lua
		lua_pushboolean(pLuaState, 0);

		return 1;
	}*/

	SceneEffect_EdgeBlurAA* pEdgeBlur = new SceneEffect_EdgeBlurAA();

	s_pScene->Add(pEdgeBlur, 7);

	if(!pEdgeBlur->Create("NONE data/shaders/fxaa.vert data/shaders/fxaa.frag"))
	{
		// Failed load, return false for lua
		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	SceneEffect_HDRR* pHDRR = new SceneEffect_HDRR();

	s_pScene->Add(pHDRR, 8);
	
	if(!pHDRR->Create("NONE NONE data/shaders/downSample.frag", "NONE NONE data/shaders/toneMap.frag", "NONE NONE data/shaders/bloomPortionRender.frag", "NONE NONE data/shaders/blur_horizontal.frag", "NONE NONE data/shaders/blur_vertical.frag", 8))
	{
		// Failed load, return false for lua
		lua_pushboolean(pLuaState, 0);

		return 1;
	}
	
	// --------------------------------- Enemy Spawning System ---------------------------------

	SceneObject_EnemySelector* pEnemySelector = new SceneObject_EnemySelector();

	s_pScene->Add(pEnemySelector, false, "enemSel");

	// Load successful, return true for lua
	lua_pushboolean(pLuaState, 1);

	return 1;
}

int World_SetRandomSeed(lua_State* pLuaState)
{
	if(s_pWorld == NULL)
	{
		std::cerr << "No world bound!" << std::endl;
		return 0;
	}

	int argc = lua_gettop(pLuaState);

	if(argc != 0)
	{
		std::cerr << "Function \"World_SetRandomSeed\" requires 0 arguments, not " << argc << std::endl;
		return 0;
	}

	// Random seed
	s_generator.m_seed = rand() % 10000;

	return 0; // Number of return values
}

int World_SetSeed(lua_State* pLuaState)
{
	if(s_pWorld == NULL)
	{
		std::cerr << "No world bound!" << std::endl;
		return 0;
	}

	int argc = lua_gettop(pLuaState);

	if(argc != 1)
	{
		std::cerr << "Function \"World_SetRandomSeed\" requires 1 argument, not " << argc << std::endl;
		return 0;
	}

	s_generator.m_seed = static_cast<int>(lua_tonumber(pLuaState, 1));
	lua_remove(pLuaState, 1);

	return 0;
}

int World_SetVoxel(lua_State* pLuaState)
{
	if(s_pWorld == NULL)
	{
		std::cerr << "No world bound!" << std::endl;
		return 0;
	}

	int argc = lua_gettop(pLuaState);

	if(argc != 4)
	{
		std::cerr << "Function \"World_SetVoxel\" requires 4 arguments, not " << argc << std::endl;
		return 0;
	}

	// Arguments
	int x = static_cast<int>(lua_tonumber(pLuaState, 1));
	int y = static_cast<int>(lua_tonumber(pLuaState, 2));
	int z = static_cast<int>(lua_tonumber(pLuaState, 3));
	unsigned char value = static_cast<unsigned char>(lua_tonumber(pLuaState, 4));
	lua_remove(pLuaState, 4);

	s_pWorld->SetVoxel(x, y, z, value);

	return 0; // Number of return values
}

int World_GetVoxel(lua_State* pLuaState)
{
	if(s_pWorld == NULL)
	{
		std::cerr << "No world bound!" << std::endl;
		return 0;
	}

	int argc = lua_gettop(pLuaState);

	if(argc != 3)
	{
		std::cerr << "Function \"World_GetVoxel\" requires 3 arguments, not " << argc << std::endl;
		return 0;
	}

	// Arguments
	int x = static_cast<int>(lua_tonumber(pLuaState, 1));
	int y = static_cast<int>(lua_tonumber(pLuaState, 2));
	int z = static_cast<int>(lua_tonumber(pLuaState, 3));
	lua_remove(pLuaState, 3);

	lua_pushnumber(pLuaState, s_pWorld->GetVoxel(x, y, z));

	return 1; // Number of return values
}

int World_Perlin(lua_State* pLuaState)
{
	int argc = lua_gettop(pLuaState);

	if(argc != 6)
	{
		std::cerr << "Function \"World_Perlin\" requires 6 arguments, not " << argc << std::endl;
		return 0;
	}

	float x = static_cast<float>(lua_tonumber(pLuaState, 1));
	float y = static_cast<float>(lua_tonumber(pLuaState, 2));
	float z = static_cast<float>(lua_tonumber(pLuaState, 3));

	int octaves = static_cast<int>(lua_tonumber(pLuaState, 4));
	float frequencyBase = static_cast<float>(lua_tonumber(pLuaState, 5));
	float persistence = static_cast<float>(lua_tonumber(pLuaState, 6));

	lua_remove(pLuaState, 6);

	lua_pushnumber(pLuaState, s_generator.PerlinNoise3D(x, y, z, octaves, frequencyBase, persistence));

	return 1; // Number of return values
}

int World_FillBox(lua_State* pLuaState)
{
	int argc = lua_gettop(pLuaState);

	if(argc != 7)
	{
		std::cerr << "Function \"World_FillBox\" requires 7 arguments, not " << argc << std::endl;
		return 0;
	}

	int l_x = static_cast<int>(lua_tonumber(pLuaState, 1));
	int l_y = static_cast<int>(lua_tonumber(pLuaState, 2));
	int l_z = static_cast<int>(lua_tonumber(pLuaState, 3));

	int u_x = static_cast<int>(lua_tonumber(pLuaState, 4));
	int u_y = static_cast<int>(lua_tonumber(pLuaState, 5));
	int u_z = static_cast<int>(lua_tonumber(pLuaState, 6));

	unsigned char id = static_cast<unsigned char>(lua_tonumber(pLuaState, 7));

	lua_remove(pLuaState, 7);

	s_pWorld->FillBox(Point3i(l_x, l_y, l_z), Point3i(u_x, u_y, u_z), id);

	return 0; // Number of return values
}

int World_AddProp(lua_State* pLuaState)
{
	if(s_pScene == NULL)
	{
		std::cerr << "No scene bound!" << std::endl;

		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	int argc = lua_gettop(pLuaState);

	if(argc != 10)
	{
		std::cerr << "Function \"World_AddProp\" requires 10 arguments, not " << argc << std::endl;

		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	std::string fileName(lua_tostring(pLuaState, 1));

	float x = static_cast<float>(lua_tonumber(pLuaState, 2));
	float y = static_cast<float>(lua_tonumber(pLuaState, 3));
	float z = static_cast<float>(lua_tonumber(pLuaState, 4));

	float rx = static_cast<float>(lua_tonumber(pLuaState, 5));
	float ry = static_cast<float>(lua_tonumber(pLuaState, 6));
	float rz = static_cast<float>(lua_tonumber(pLuaState, 7));

	float sx = static_cast<float>(lua_tonumber(pLuaState, 8));
	float sy = static_cast<float>(lua_tonumber(pLuaState, 9));
	float sz = static_cast<float>(lua_tonumber(pLuaState, 10));

	lua_remove(pLuaState, 10);

	// Create new prop
	SceneObject_Prop* pProp = new SceneObject_Prop();

	s_pScene->Add(pProp, true);

	if(!pProp->Create(fileName))
	{
		delete pProp;

		std::cerr << "Could not create model " << fileName << "!" << std::endl;

		// Failed load, return false for lua
		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	pProp->SetPosition(Vec3f(x, y, z));

	if(rx != 0.0f || ry != 0.0f || rz != 0.0f)
		pProp->SetRotation(Vec3f(rx, ry, rz));

	if(sx != 1.0f || sy != 1.0f || sz != 1.0f)
		pProp->SetScale(Vec3f(sx, sy, sz));

	// Succeeded, return true for lua
	lua_pushboolean(pLuaState, 1);

	return 1; // Number of return values
}

int World_AddProp_Physics_Static(lua_State* pLuaState)
{
	if(s_pScene == NULL)
	{
		std::cerr << "No scene bound!" << std::endl;

		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	int argc = lua_gettop(pLuaState);

	if(argc != 9)
	{
		std::cerr << "Function \"World_AddProp_Physics_Static\" requires 9 arguments, not " << argc << std::endl;

		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	std::string fileName(lua_tostring(pLuaState, 1));

	float x = static_cast<float>(lua_tonumber(pLuaState, 2));
	float y = static_cast<float>(lua_tonumber(pLuaState, 3));
	float z = static_cast<float>(lua_tonumber(pLuaState, 4));

	float rx = static_cast<float>(lua_tonumber(pLuaState, 5));
	float ry = static_cast<float>(lua_tonumber(pLuaState, 6));
	float rz = static_cast<float>(lua_tonumber(pLuaState, 7));

	float restitution = static_cast<float>(lua_tonumber(pLuaState, 8));
	float friction = static_cast<float>(lua_tonumber(pLuaState, 9));

	lua_remove(pLuaState, 9);

	// Create new prop
	SceneObject_Prop_Physics_Static* pProp = new SceneObject_Prop_Physics_Static();

	s_pScene->Add(pProp, true);

	Quaternion rot(Quaternion::GetRotated(rx, Vec3f(1.0f, 0.0f, 0.0f)) *
		Quaternion::GetRotated(ry, Vec3f(0.0f, 1.0f, 0.0f)) *
		Quaternion::GetRotated(rz, Vec3f(0.0f, 0.0f, 1.0f)));

	if(!pProp->Create(fileName, Vec3f(x, y, z), rot, restitution, friction))
	{
		delete pProp;

		std::cerr << "Could not create model " << fileName << "!" << std::endl;

		// Failed load, return false for lua
		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	// Succeeded, return true for lua
	lua_pushboolean(pLuaState, 1);

	return 1; // Number of return values
}

int World_AddProp_Physics_Dynamic(lua_State* pLuaState)
{
	if(s_pScene == NULL)
	{
		std::cerr << "No scene bound!" << std::endl;

		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	int argc = lua_gettop(pLuaState);

	if(argc != 11)
	{
		std::cerr << "Function \"World_AddProp_Physics_Static\" requires 11 arguments, not " << argc << std::endl;

		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	std::string fileName_obj(lua_tostring(pLuaState, 1));
	std::string fileName_phy(lua_tostring(pLuaState, 2));

	float x = static_cast<float>(lua_tonumber(pLuaState, 3));
	float y = static_cast<float>(lua_tonumber(pLuaState, 4));
	float z = static_cast<float>(lua_tonumber(pLuaState, 5));

	float rx = static_cast<float>(lua_tonumber(pLuaState, 6));
	float ry = static_cast<float>(lua_tonumber(pLuaState, 7));
	float rz = static_cast<float>(lua_tonumber(pLuaState, 8));

	float mass = static_cast<float>(lua_tonumber(pLuaState, 9));
	float restitution = static_cast<float>(lua_tonumber(pLuaState, 10));
	float friction = static_cast<float>(lua_tonumber(pLuaState, 11));

	lua_remove(pLuaState, 11);

	// Create new prop
	SceneObject_Prop_Physics_Dynamic* pProp = new SceneObject_Prop_Physics_Dynamic();

	s_pScene->Add(pProp, true);

	Quaternion rot(Quaternion::GetRotated(rx, Vec3f(1.0f, 0.0f, 0.0f)) *
		Quaternion::GetRotated(ry, Vec3f(0.0f, 1.0f, 0.0f)) *
		Quaternion::GetRotated(rz, Vec3f(0.0f, 0.0f, 1.0f)));

	if(!pProp->Create(fileName_obj, fileName_phy, Vec3f(x, y, z), rot, mass, restitution, friction))
	{
		delete pProp;

		std::cerr << "Could not create model " << fileName_obj << "!" << std::endl;

		// Failed load, return false for lua
		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	// Succeeded, return true for lua
	lua_pushboolean(pLuaState, 1);

	return 1; // Number of return values
}

int World_AddProp_Transparent(lua_State* pLuaState)
{
	return 0;

	if(s_pScene == NULL)
	{
		std::cerr << "No scene bound!" << std::endl;

		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	int argc = lua_gettop(pLuaState);

	if(argc != 11)
	{
		std::cerr << "Function \"World_AddProp\" requires 11 arguments, not " << argc << std::endl;

		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	std::string fileName(lua_tostring(pLuaState, 1));

	float x = static_cast<float>(lua_tonumber(pLuaState, 2));
	float y = static_cast<float>(lua_tonumber(pLuaState, 3));
	float z = static_cast<float>(lua_tonumber(pLuaState, 4));

	float rx = static_cast<float>(lua_tonumber(pLuaState, 5));
	float ry = static_cast<float>(lua_tonumber(pLuaState, 6));
	float rz = static_cast<float>(lua_tonumber(pLuaState, 7));

	float sx = static_cast<float>(lua_tonumber(pLuaState, 8));
	float sy = static_cast<float>(lua_tonumber(pLuaState, 9));
	float sz = static_cast<float>(lua_tonumber(pLuaState, 10));

	bool solid = lua_toboolean(pLuaState, 11) == 1 ? true : false;

	lua_remove(pLuaState, 11);

	// Create new prop
	SceneObject_Prop_Transparent* pProp = new SceneObject_Prop_Transparent();

	s_pScene->Add(pProp, true);

	if(!pProp->Create(fileName, solid))
	{
		delete pProp;

		std::cerr << "Could not create model " << fileName << "!" << std::endl;

		// Failed load, return false for lua
		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	pProp->SetPosition(Vec3f(x, y, z));

	if(rx != 0.0f || ry != 0.0f || rz != 0.0f)
		pProp->SetRotation(Vec3f(rx, ry, rz));

	if(sx != 1.0f || sy != 1.0f || sz != 1.0f)
		pProp->SetScale(Vec3f(sx, sy, sz));

	// Succeeded, return true for lua
	lua_pushboolean(pLuaState, 1);

	return 1; // Number of return values
}

int World_AddLight_Point(lua_State* pLuaState)
{
	if(s_pScene == NULL)
	{
		std::cerr << "No scene bound!" << std::endl;

		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	int argc = lua_gettop(pLuaState);

	if(argc != 7)
	{
		std::cerr << "Function \"World_AddLight_Point\" requires 7 arguments, not " << argc << std::endl;

		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	float x = static_cast<float>(lua_tonumber(pLuaState, 1));
	float y = static_cast<float>(lua_tonumber(pLuaState, 2));
	float z = static_cast<float>(lua_tonumber(pLuaState, 3));

	float r = static_cast<float>(lua_tonumber(pLuaState, 4));
	float g = static_cast<float>(lua_tonumber(pLuaState, 5));
	float b = static_cast<float>(lua_tonumber(pLuaState, 6));

	float intensity = static_cast<float>(lua_tonumber(pLuaState, 7));

	lua_remove(pLuaState, 7);

	SceneEffect_Lighting* pLighting = static_cast<SceneEffect_Lighting*>(s_pScene->GetNamed_Effect("lighting"));

	assert(pLighting != NULL);

	Light_Point* pLight = new Light_Point();
	pLight->SetCenter(Vec3f(x, y, z));
	pLight->m_color = Color3f(r, g, b);
	pLight->SetIntensity(intensity);

	pLighting->AddLight(pLight);

	return 0; // Number of return values
}

int World_AddLight_Spot(lua_State* pLuaState)
{
	if(s_pScene == NULL)
	{
		std::cerr << "No scene bound!" << std::endl;

		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	int argc = lua_gettop(pLuaState);

	if(argc != 11)
	{
		std::cerr << "Function \"World_AddLight_Point\" requires 11 arguments, not " << argc << std::endl;

		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	float x = static_cast<float>(lua_tonumber(pLuaState, 1));
	float y = static_cast<float>(lua_tonumber(pLuaState, 2));
	float z = static_cast<float>(lua_tonumber(pLuaState, 3));

	float dx = static_cast<float>(lua_tonumber(pLuaState, 4));
	float dy = static_cast<float>(lua_tonumber(pLuaState, 5));
	float dz = static_cast<float>(lua_tonumber(pLuaState, 6));

	float spreadAngle = static_cast<float>(lua_tonumber(pLuaState, 7));

	float r = static_cast<float>(lua_tonumber(pLuaState, 8));
	float g = static_cast<float>(lua_tonumber(pLuaState, 9));
	float b = static_cast<float>(lua_tonumber(pLuaState, 10));

	float intensity = static_cast<float>(lua_tonumber(pLuaState, 11));

	lua_remove(pLuaState, 11);

	SceneEffect_Lighting* pLighting = static_cast<SceneEffect_Lighting*>(s_pScene->GetNamed_Effect("lighting"));

	assert(pLighting != NULL);

	Light_Spot* pLight = new Light_Spot();
	pLight->SetCenter(Vec3f(x, y, z));
	pLight->m_color = Color3f(r, g, b);
	pLight->SetIntensity(intensity);
	pLight->SetDirection(Vec3f(dx, dy, dz));
	pLight->SetSpreadAngle(spreadAngle);

	pLighting->AddLight(pLight);

	return 0; // Number of return values
}

int World_AddSpiralStairs(lua_State* pLuaState)
{
	if(s_pScene == NULL)
	{
		std::cerr << "No scene bound!" << std::endl;

		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	int argc = lua_gettop(pLuaState);

	if(argc != 6)
	{
		std::cerr << "Function \"World_AddSpiralStairs\" requires 6 arguments, not " << argc << std::endl;

		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	bool turnCW = lua_toboolean(pLuaState, 1) == 1 ? true : false;

	float x = static_cast<float>(lua_tonumber(pLuaState, 2));
	float y = static_cast<float>(lua_tonumber(pLuaState, 3));
	float z = static_cast<float>(lua_tonumber(pLuaState, 4));

	int height = static_cast<int>(lua_tonumber(pLuaState, 5));

	float startAngle = static_cast<float>(lua_tonumber(pLuaState, 6));

	lua_remove(pLuaState, 6);

	SceneObject_SpiralStairs* pStairs = new SceneObject_SpiralStairs();

	s_pScene->Add(pStairs, true);

	if(!pStairs->Create("data/models/step_spiral.obj", "data/models/beam.obj", "data/models/step_spiral.phy", turnCW, height, startAngle, Vec3f(x, y, z)))
		abort();

	lua_pushboolean(pLuaState, 1);

	return 1; // Number of return values
}

int World_AddDoor(lua_State* pLuaState)
{
	if(s_pScene == NULL)
	{
		std::cerr << "No scene bound!" << std::endl;

		lua_pushboolean(pLuaState, 0);

		return 0;
	}

	int argc = lua_gettop(pLuaState);

	if(argc != 5)
	{
		std::cerr << "Function \"World_AddDoor\" requires 5 arguments, not " << argc << std::endl;

		lua_pushboolean(pLuaState, 0);

		return 0;
	}

	float x = static_cast<float>(lua_tonumber(pLuaState, 1));
	float y = static_cast<float>(lua_tonumber(pLuaState, 2));
	float z = static_cast<float>(lua_tonumber(pLuaState, 3));
	float angle = static_cast<float>(lua_tonumber(pLuaState, 4));

	bool openCW = lua_toboolean(pLuaState, 5) == 1 ? true : false;

	lua_remove(pLuaState, 5);

	SceneObject_Door* pDoor = new SceneObject_Door();

	s_pScene->Add(pDoor, true);

	if(!pDoor->Create("data/models/door.obj", Vec3f(x, y, z), angle, openCW))
		abort();

	lua_pushboolean(pLuaState, 1);

	return 1; // Number of return values
}

int World_AddSound(lua_State* pLuaState)
{
	if(s_pScene == NULL)
	{
		std::cerr << "No scene bound!" << std::endl;

		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	int argc = lua_gettop(pLuaState);

	if(argc != 5)
	{
		std::cerr << "Function \"World_AddSound\" requires 5 arguments, not " << argc << std::endl;

		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	std::string fileName(lua_tostring(pLuaState, 1));

	float x = static_cast<float>(lua_tonumber(pLuaState, 2));
	float y = static_cast<float>(lua_tonumber(pLuaState, 3));
	float z = static_cast<float>(lua_tonumber(pLuaState, 4));

	float gain = static_cast<float>(lua_tonumber(pLuaState, 5));

	lua_remove(pLuaState, 5);

	SceneObject_Sound* pSound = new SceneObject_Sound();

	s_pScene->Add(pSound, false);

	if(!pSound->Create(fileName))
	{
		std::cerr << "ERROR: Could not create sound!" << argc << std::endl;

		delete pSound;

		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	pSound->m_soundSource.SetPosition(Vec3f(x, y, z));
	pSound->m_soundSource.SetPitch(1.0f);
	pSound->m_soundSource.SetVelocity(Vec3f(0.0f, 0.0f, 0.0f));
	pSound->m_soundSource.SetGain(gain);
	pSound->m_soundSource.SetLooping(true);
	pSound->m_soundSource.Play();

	return 1; // Number of return values
}

int World_GetSeed(lua_State* pLuaState)
{
	int argc = lua_gettop(pLuaState);

	if(argc != 0)
	{
		std::cerr << "Function \"World_GetSeed\" requires 0 arguments, not " << argc << std::endl;

		lua_pushnumber(pLuaState, 0.0f);

		return 1;
	}
	
	lua_pushnumber(pLuaState, s_generator.m_seed);

	return 1; // Number of return values
}

int World_AddWater(lua_State* pLuaState)
{
	if(s_pScene == NULL)
	{
		std::cerr << "No scene bound!" << std::endl;

		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	if(s_pWorld == NULL)
	{
		std::cerr << "No world bound!" << std::endl;

		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	int argc = lua_gettop(pLuaState);

	if(argc != 5)
	{
		std::cerr << "Function \"World_AddWater\" requires 5 arguments, not " << argc << std::endl;

		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	std::string fileName_bump0(lua_tostring(pLuaState, 1));
	std::string fileName_bump1(lua_tostring(pLuaState, 2));

	int x = static_cast<int>(lua_tonumber(pLuaState, 3));
	int y = static_cast<int>(lua_tonumber(pLuaState, 4));
	int z = static_cast<int>(lua_tonumber(pLuaState, 5));

	lua_remove(pLuaState, 5);

	SceneObject_Water* pWater = new SceneObject_Water();

	s_pScene->Add(pWater, true);

	if(!pWater->Create(fileName_bump0, fileName_bump1, Point3i(x, y, z)))
	{
		std::cerr << "Could not create water!" << std::endl;

		lua_pushboolean(pLuaState, 1);

		return 1;
	}

	lua_pushboolean(pLuaState, 1);

	return 1;
}

/*int World_SetPlayerSpawn(lua_State* pLuaState)
{
	// For now, just spawn a prop place holder named "spawn"

	if(s_pScene == NULL)
	{
		std::cerr << "No scene bound!" << std::endl;

		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	int argc = lua_gettop(pLuaState);

	if(argc != 4)
	{
		std::cerr << "Function \"World_SetPlayerSpawn\" requires 4 arguments, not " << argc << std::endl;

		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	float x = static_cast<float>(lua_tonumber(pLuaState, 1));
	float y = static_cast<float>(lua_tonumber(pLuaState, 2));
	float z = static_cast<float>(lua_tonumber(pLuaState, 3));

	float ry = static_cast<float>(lua_tonumber(pLuaState, 4));

	lua_remove(pLuaState, 4);

	// Create new prop
	SceneObject_Prop* pProp = new SceneObject_Prop();

	s_pScene->Add(pProp, true, "spawn");

	if(!pProp->Create("data/models/spawn.obj", false))
	{
		delete pProp;

		std::cerr << "Could not create model data/models/spawn.obj!" << std::endl;

		// Failed load, return false for lua
		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	pProp->SetPosition(Vec3f(x, y, z));

	if(ry != 0.0f)
		pProp->SetRotation(Vec3f(0.0f, ry, 0.0f));

	// Succeeded, return true for lua
	lua_pushboolean(pLuaState, 1);

	return 1; // Number of return values
}

int World_SetPlayerGoal(lua_State* pLuaState)
{
	// For now, just spawn a prop place holder named "goal"

	if(s_pScene == NULL)
	{
		std::cerr << "No scene bound!" << std::endl;

		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	int argc = lua_gettop(pLuaState);

	if(argc != 4)
	{
		std::cerr << "Function \"World_SetPlayerGoal\" requires 4 arguments, not " << argc << std::endl;

		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	float x = static_cast<float>(lua_tonumber(pLuaState, 1));
	float y = static_cast<float>(lua_tonumber(pLuaState, 2));
	float z = static_cast<float>(lua_tonumber(pLuaState, 3));

	float ry = static_cast<float>(lua_tonumber(pLuaState, 4));

	lua_remove(pLuaState, 4);

	// Create new prop
	SceneObject_Prop* pProp = new SceneObject_Prop();

	s_pScene->Add(pProp, true, "goal");

	if(!pProp->Create("data/models/goal.ofalse))
	{
		delete pProp;

		std::cerr << "Could not create model data/models/goal.obj!" << std::endl;

		// Failed load, return false for lua
		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	pProp->SetPosition(Vec3f(x, y, z));

	if(ry != 0.0f)
		pProp->SetRotation(Vec3f(0.0f, ry, 0.0f));

	// Succeeded, return true for lua
	lua_pushboolean(pLuaState, 1);

	return 1; // Number of return values
}*/

int World_GetRandomEnemyType(lua_State* pLuaState)
{
	return 0;
}

int World_SpawnEnemy(lua_State* pLuaState)
{
	if(s_pScene == NULL)
	{
		std::cerr << "No scene bound!" << std::endl;

		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	int argc = lua_gettop(pLuaState);

	if(argc != 11)
	{
		std::cerr << "Function \"World_AddProp\" requires 11 arguments, not " << argc << std::endl;

		lua_pushboolean(pLuaState, 0);

		return 1;
	}

	std::string fileName(lua_tostring(pLuaState, 1));

	float x = static_cast<float>(lua_tonumber(pLuaState, 2));
	float y = static_cast<float>(lua_tonumber(pLuaState, 3));
	float z = static_cast<float>(lua_tonumber(pLuaState, 4));

	float rx = static_cast<float>(lua_tonumber(pLuaState, 5));
	float ry = static_cast<float>(lua_tonumber(pLuaState, 6));

	return 0;
}

/*int World_GetRandomEnemyType(lua_State* pLuaState)
{

}

int World_SpawnEnemyByName(lua_State* pLuaState)
{

}

int World_SpawnEnemyByTier(lua_State* pLuaState)
{

}*/