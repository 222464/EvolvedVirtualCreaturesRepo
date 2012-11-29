mapDimX = 512
mapDimY = 128
mapDimZ = 512

cardinal_E = 0
cardinal_N = 1
cardinal_W = 2
cardinal_S = 3

function Corridor(x, y, z, cardinal)

end

function CarveMaze(dimX, dimY, dimZ)
	local maxX = dimX - 1
	local maxY = dimY - 1
	local maxZ = dimZ - 1

	local boxDimX = 16
	local boxDimX = 16


	local numBranches = 10

	for b = 1, numBranches, 1 do

		local startPoint = {math.random(, maxX), math.random(1, maxY), math.random(1, maxZ)}

	end
end

mapSeed = os.time()
math.randomseed(mapSeed)

World_Create(mapDimX,mapDimY, mapDimZ, "data/textures/voxelTexture_diffuse_HD.png", "data/textures/voxelTexture_specular_HD.png", "data/textures/voxelTexture_normalMap_HD.bmp", "data/textures/descriptor.txt", 2, 4)
