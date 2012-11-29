--[[--

Test Map Generator - generator_1.lua

Description: Generates a grassy floating round piece of land with trees

--]]--

totalVoxelSizeX = 256
totalVoxelSizeY = 32
totalVoxelSizeZ = 256

function GenerateTree(baseX, baseY, baseZ)
	local leafDist = 3
	local randomnessDist = 1.0

	local maxExpansionTries = math.floor((4.0 / 3.0) * math.pi * math.pow(leafDist, 3.0))

	local height = math.random(0, 4) + 4

	-- stump generation
	for h = 0, height - 1, 1 do
		World_SetVoxel(baseX, baseY + h, baseZ, 9)
	end

	local leafStartY = baseY + height

	local openList = {}

	-- starting point
	table.insert(openList, {baseX, leafStartY, baseZ})

	tries = 0

	while #openList ~= 0 do

		-- Last element, breadth-first
		point = {x, y, z}
		point.x = openList[1][1]
		point.y = openList[1][2]
		point.z = openList[1][3]

		table.remove(openList, 1)

		World_SetVoxel(point.x, point.y, point.z, 12)

		tries = tries + 1

		if tries > maxExpansionTries then
			break
		end

		-- Add successors if in a radius
		for dx = -1, 1, 1 do
			for dy = -1, 1, 1 do
				for dz = -1, 1, 1 do
					newPoint = {x, y, z}

					newPoint[1] = point.x + dx
					newPoint[2] = point.y + dy
					newPoint[3] = point.z + dz

					-- Bounds check
					if newPoint[1] >= 0 and newPoint[2] >= 0 and newPoint[3] >= 0 and
					newPoint[1] < totalVoxelSizeX and newPoint[2] < totalVoxelSizeY and newPoint[3] < totalVoxelSizeZ then

						-- Must be air
						if World_GetVoxel(newPoint[1], newPoint[2], newPoint[3]) == 0 then

							-- See if expanded too far by getting distance
							dist = math.sqrt(math.pow(newPoint[1] - baseX, 2.0) + math.pow(newPoint[2] - leafStartY, 2.0) + math.pow(newPoint[3] - baseZ, 2.0))

							if dist > leafDist - randomnessDist then
								-- Further out leaves are random
								if math.random(0, 100) > 50 then
									table.insert(openList, newPoint)
								end
							else
								table.insert(openList, newPoint)
							end
						end
					end
				end
			end
		end
	end
end

-------------------------- MAIN EXECUTION --------------------------

math.randomseed(os.time())

centerX = totalVoxelSizeX / 2
centerY = totalVoxelSizeY / 2
centerZ = totalVoxelSizeZ / 2

maxDist = math.max(centerX, centerY, centerZ)

World_Create(totalVoxelSizeX, totalVoxelSizeY, totalVoxelSizeZ, "data/textures/terrain_diffuse.png", "data/textures/terrain_specular.png", "data/textures/terrain_normal.bmp", "data/textures/descriptor.txt", 4, 4)

-- Main perlin terrain pass
for x = 0, totalVoxelSizeX - 1, 1 do
	for y = totalVoxelSizeY - 1, 0, -1 do
		for z = 0, totalVoxelSizeZ - 1, 1 do

			noise = (World_Perlin(x * 0.03, y * 0.03, z * 0.03, 1, 0.8, 1.2) + 2.0) * (totalVoxelSizeY - y) / totalVoxelSizeY * 1.2
			distMuliplier = 1.0--(maxDist - math.sqrt(math.pow(x - centerX, 2.0) + math.pow(y - centerY, 2.0) + math.pow(z - centerZ, 2.0))) / maxDist + 0.4 -- Add a bit, or else too much air

			-- Less dense as gets further from center of map
			noise = noise * distMuliplier

			if noise > 1.0 then
				if World_GetVoxel(x, y + 1, z) == 0 then
					World_SetVoxel(x, y, z, 1);
				elseif noise > 1.2 then
					World_SetVoxel(x, y, z, 2)
				else
					World_SetVoxel(x, y, z, 3)
				end
			else
				World_SetVoxel(x, y, z, 0)
			end
		end
	end
end

-- Tree generation pass

local x = 6
local z = 6

while x < totalVoxelSizeX - 7 do
	while z < totalVoxelSizeZ - 7 do
		for y = totalVoxelSizeY - 7, 1, -1 do
			if World_GetVoxel(x, y - 1, z) == 3 then
				if math.random(0, 1000) > 300 then
					GenerateTree(x, y, z)
				end
			end
		end

		z = z + math.random(0, 6) + 5
	end

	x = x + math.random(0, 6) + 5
end

-- Well in the middle
for x = centerX - 3, centerX + 3, 1 do
	for z = centerZ - 3, centerZ + 3, 1 do
		for y = totalVoxelSizeY, centerY - 5, -1 do
			World_SetVoxel(x, y, z, 0)
		end
	end
end

--World_AddWater("data/shaders/water/bump0.bmp", "data/shaders/water/bump1.bmp", centerX, centerY - 3, centerZ);

World_AddSpiralStairs(false, 16, 16, 16, 20, 0)

