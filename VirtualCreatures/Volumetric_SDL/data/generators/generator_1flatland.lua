mapDimX = 256
mapDimY = 16
mapDimZ = 256

mapSeed = os.time()
math.randomseed(mapSeed)

World_Create(mapDimX, mapDimY, mapDimZ, "data/textures/terrain_diffuse.png", "data/textures/terrain_specular.png", "data/textures/terrain_normal.bmp", "data/textures/descriptor.txt", 4, 4)

World_FillBox(0, 0, 0, mapDimX, mapDimY - 8, mapDimZ, 1)
World_FillBox(0, mapDimY - 8, 0, mapDimX, mapDimY, mapDimZ, 0)

World_AddProp("data/weapons/ac10.obj", 210, 122, 198, 0, 0, 0, .6, .6, .6)
