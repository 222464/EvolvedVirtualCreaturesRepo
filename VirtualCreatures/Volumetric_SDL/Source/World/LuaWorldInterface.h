#pragma once

#include <lua.hpp>

#include <Perlin/NoiseGenerator.h>

#include <SceneObjects/Enemies/SceneObject_Enemy.h>

#include <World/World.h>

// When uncommented, puts lua in debug mode
//#define LUA_DEBUG

class LuaVirtualMachine
{
private:
	void Lua_Init();
	void Lua_Destroy();

public:
	bool m_stopOnError;

	lua_State* m_pLuaVM;

	LuaVirtualMachine();
	~LuaVirtualMachine();

	void RunFile(const std::string &fileName);
	void RunString(const std::string &str);

	void Lua_Report_Errors();
};

extern World* s_pWorld;
extern Scene* s_pScene;

extern NoiseGenerator s_generator;

void LUA_API_BindScene(Scene* pScene);

void World_LUA_API_Register(LuaVirtualMachine* pLVM);

// Lua functions
int World_Create(lua_State* pLuaState);

int World_SetVoxel(lua_State* pLuaState);
int World_GetVoxel(lua_State* pLuaState);

int World_SetRandomSeed(lua_State* pLuaState);
int World_SetSeed(lua_State* pLuaState);

int World_Perlin(lua_State* pLuaState);

int World_FillBox(lua_State* pLuaState);

int World_AddProp(lua_State* pLuaState);
int World_AddProp_Physics_Static(lua_State* pLuaState);
int World_AddProp_Physics_Dynamic(lua_State* pLuaState);
int World_AddProp_Transparent(lua_State* pLuaState);

int World_AddLight_Point(lua_State* pLuaState);
int World_AddLight_Spot(lua_State* pLuaState);

int World_AddSpiralStairs(lua_State* pLuaState);

int World_AddDoor(lua_State* pLuaState);

int World_AddSound(lua_State* pLuaState);

int World_GetSeed(lua_State* pLuaState);

int World_AddWater(lua_State* pLuaState);

// Enemies
int World_GetRandomEnemyType(lua_State* pLuaState);

int World_SpawnEnemyByName(lua_State* pLuaState);
int World_SpawnEnemyByTier(lua_State* pLuaState);