mapSeed = os.time()
math.randomseed(mapSeed)

function min(...)
	local lowest
	for _, i in ipairs({...}) do
		if(not lowest or i < lowest) then
			lowest = i
		end
	end
	return lowest
end

function FillBox(x1, y1, z1, x2, y2, z2, material)
	World_FillBox(x1 + 1, y1 + 1, z1 + 1, x2, y2, z2, material)
end

function dtr(angle)
	return angle*math.pi/180
end

function MakeStairs(WallStartEndCoords, storyHeight, lessIsIn, doorCoords)
	local StairCoords = {{}, {}}
	if(storyHeight <= 5) then
		return
	end
	local stairOrder = math.random(0, 4) -- 0 means try spiral first, otherwise try angled first
	local whichIsWall -- which coordinate varies along wall
	if(#WallStartEndCoords[1] == 1) then
		whichIsWall = 2
		table.insert(doorCoords, {WallStartEndCoords[2][1], WallStartEndCoords[2][2] - 1})
	else
		whichIsWall = 1
		table.insert(doorCoords, {WallStartEndCoords[1][1], WallStartEndCoords[1][2] - 1})
	end
	table.sort(doorCoords, function(a, b) if(a[whichIsWall] < b[whichIsWall]) then return true end end)
	whichIsntWall = whichIsWall % 2 + 1
	local makeAngled = function()
		local checkForward = function(doorCoord, lastDoor)
			for i = doorCoord[whichIsWall] - 3, lastDoor + 3, -1 do
				if(WallStartEndCoords[whichIsWall][2] - i > storyHeight) then
					StairCoords[1][whichIsWall] = math.random(lastDoor + 3, i)
					StairCoords[2][whichIsWall] = StairCoords[1][whichIsWall] + storyHeight - 1
					return true
				end
			end
		end
		local checkBackward = function(doorCoord, lastDoor)
			for i = lastDoor + 4, doorCoord[whichIsWall] - 2 do
				if(i - WallStartEndCoords[whichIsWall][1] > storyHeight) then
					StairCoords[1][whichIsWall] = math.random(i, doorCoord[whichIsWall] - 2)
					StairCoords[2][whichIsWall] = StairCoords[1][whichIsWall] - storyHeight + 1
					return true
				end
			end
		end
		local firstCheck = math.random(0, 1)
		while(firstCheck < 6) do
			local lastDoor = WallStartEndCoords[whichIsWall][1]
			for _, doorCoord in ipairs(doorCoords) do
				local goodSpot
				if(firstCheck % 2 == 0) then
					goodSpot = checkForward(doorCoord, lastDoor)
				else
					goodSpot = checkBackward(doorCoord, lastDoor)
				end
				if(goodSpot) then
					if(lessIsIn) then
						StairCoords[1][whichIsntWall] = WallStartEndCoords[whichIsntWall][1] - 1
					else
						StairCoords[1][whichIsntWall] = WallStartEndCoords[whichIsntWall][1] + 1
					end
					StairCoords[2][whichIsntWall] = StairCoords[1][whichIsntWall]
					return storyHeight, 1, {{StairCoords[1][1], StairCoords[1][2]}, {StairCoords[2][1], StairCoords[2][2]}}
				end
				lastDoor = doorCoord[whichIsWall]
			end
			firstCheck = firstCheck + 3
		end
	end
	local makeSpiral = function()
		local lastDoor = WallStartEndCoords[whichIsWall][1] - 1
		for _, doorCoord in ipairs(doorCoords) do
			if(doorCoord[whichIsWall] - lastDoor > 6) then
				StairCoords[1][whichIsWall] = math.random(lastDoor + 3, doorCoord[whichIsWall] - 4)
				local stairType
				if(lessIsIn) then
					StairCoords[1][whichIsntWall] = WallStartEndCoords[whichIsntWall][1] - 3
					stairType = 2
				else
					StairCoords[1][whichIsntWall] = WallStartEndCoords[whichIsntWall][1] + 1
					stairType = -2
				end
				return storyHeight, stairType, {{StairCoords[1][1], StairCoords[1][2]}, {}}
			end
			lastDoor = doorCoord[whichIsWall]
		end
	end
	if(stairOrder == 0) then
		local storyHeight, stairType, StairCoords = makeSpiral()
		if(sstoryHeight) then
			return storyHeight, stairType, StairCoords
		end
		return makeAngled()
	else
		local storyHeight, stairType, StairCoords = makeAngled()
		if(storyHeight) then
			return storyHeight, stairType, StairCoords
		end
		return makeSpiral()
	end
end

function MakeFloor(cornerX, cornerY, cornerZ, dimX, dimY, dimZ, DoorCoordList, FloorTypes)
	local oppX, oppY, oppZ = cornerX + dimX, cornerY + dimY, cornerZ + dimZ
	local wallOrder = {1, 2, 3, 4}
	for i, num in ipairs(wallOrder) do
		local toSwap = math.random(0, #wallOrder)
		wallOrder[i], wallOrder[toSwap] = wallOrder[toSwap], wallOrder[i]
	end
	local secondStoryY, stairType, StairCoords, lessIsIn
	for i = 1, 4 do
		if(not secondStoryY) then
			if(wallOrder[i] == 1) then
				secondStoryY, stairType, StairCoords = MakeStairs({{cornerX}, {cornerZ, oppZ}}, math.ceil(dimY/2), false, DoorCoordList[1])
				if(secondStoryY) then
					lessIsIn = false
				end
			elseif(wallOrder[i] == 2) then
				secondStoryY, stairType, StairCoords = MakeStairs({{oppX}, {cornerZ, oppZ}}, math.ceil(dimY/2), true, DoorCoordList[2])
				if(secondStoryY) then
					lessIsIn = true
				end
			elseif(wallOrder[i] == 3) then
				secondStoryY, stairType, StairCoords = MakeStairs({{cornerX, oppX}, {cornerZ}}, math.ceil(dimY/2), false, DoorCoordList[3])
				if(secondStoryY) then
					lessIsIn = false
				end
			else
				secondStoryY, stairType, StairCoords = MakeStairs({{cornerX, oppX}, {oppZ}}, math.ceil(dimY/2), true, DoorCoordList[4])
				if(secondStoryY) then
					lessIsIn = true
				end
			end
		end
	end
	-- Generate second floor without blocking staircase, full floor is 1 and balcony is 2
	local floorType, balconyX, balconyZ = 1
	if(FloorTypes and #FloorTypes > 0 and stairType == 1) then
		floorType = FloorTypes[math.random(1, #FloorTypes)]
		if(floorType == 2) then
			local balconySize = math.random(3, 4)
			balconyX = math.ceil(balconySize * dimX / 15)
			balconyZ = math.ceil(balconySize * dimZ / 15)
		elseif(floorType == 3) then
			local balconySize = math.random(3, 4)
			balconyX = math.ceil(balconySize * dimX / 6.5)
			balconyZ = math.ceil(balconySize * dimZ / 6.5)
		end
	end
	if(stairType == 1) then
		if(StairCoords[1][1] == StairCoords[2][1]) then
			local drawFloor = function(x, z)
				if(x > StairCoords[1][1] + 2 or x < StairCoords[1][1] - 2) then
					World_SetVoxel(x, cornerY + secondStoryY, z, 1)
				end
				if(StairCoords[1][2] < StairCoords[2][2]) then
					if(z < StairCoords[1][2] or z > StairCoords[2][2]) then
						World_SetVoxel(x, cornerY + secondStoryY, z, 1)
					end
				else
					if(z > StairCoords[1][2] or z < StairCoords[2][2]) then
						World_SetVoxel(x, cornerY + secondStoryY, z, 1)
					end
				end
			end
			for x = cornerX, oppX do
				for z = cornerZ, oppZ do
					if(floorType == 1) then
						drawFloor(x, z)
					elseif(floorType == 2) then
						if(x <= cornerX + balconyX or x >= oppX - balconyX or z <= cornerZ + balconyZ or z >= oppZ - balconyZ) then
							drawFloor(x, z)
						end
					elseif(floorType == 3) then
						if(StairCoords[1][1] == cornerX + 1) then
							if(x <= cornerX + balconyX) then
								drawFloor(x, z)
							end
						elseif(x >= oppX - balconyX) then
							drawFloor(x, z)
						end
					end
				end
			end
		else
			local drawFloor = function(x, z)
				if(z > StairCoords[1][2] + 2 or z < StairCoords[1][2] - 2) then
					World_SetVoxel(x, cornerY + secondStoryY, z, 1)
				end
				if(StairCoords[1][1] < StairCoords[2][1]) then
					if(x < StairCoords[1][1] or x > StairCoords[2][1]) then
						World_SetVoxel(x, cornerY + secondStoryY, z, 1)
					end
				else
					if(x > StairCoords[1][1] or x < StairCoords[2][1]) then
						World_SetVoxel(x, cornerY + secondStoryY, z, 1)
					end
				end
			end
			for x = cornerX, oppX do
				for z = cornerZ, oppZ do
					if(floorType == 1) then
						drawFloor(x, z)
					elseif(floorType == 2) then
						if(x <= cornerX + balconyX or x >= oppX - balconyX or z <= cornerZ + balconyZ or z >= cornerZ - balconyZ) then
							drawFloor(x, z)
						end
					elseif(floorType == 3) then
						if(StairCoords[1][2] == cornerZ + 1 and z <= cornerZ + balconyZ) then
							drawFloor(x, z)
						elseif(z >= oppZ - balconyZ) then
							drawFloor(x, z)
						end
					end
				end
			end
		end
	elseif(stairType and math.abs(stairType) == 2) then
		for x = cornerX, oppX do
			for z = cornerZ, oppZ do
				if(x < StairCoords[1][1] or x > StairCoords[1][1] + 2 or z < StairCoords[1][2] or z > StairCoords[1][2] + 2) then
					World_SetVoxel(x, cornerY + secondStoryY, z, 1)
				end
			end
		end
	else
		secondStoryY = dimY
	end
	-- Draw stairs
	if(stairType == 1) then
		local rotation, step, y, xShift, zShift = 0, 1, 1.5 + cornerY
		if(StairCoords[1][1] < StairCoords[2][1]) then
			rotation = 270
			xShift = .5
			if(lessIsIn) then
				zShift = -.5
			else
				zShift = 1.5
			end
		elseif(StairCoords[1][1] > StairCoords[2][1]) then
			rotation = 90
			step = -1
			xShift = .5
			if(lessIsIn) then
				zShift = -.5
			else
				zShift = 1.5
			end
		elseif(StairCoords[1][2] > StairCoords[2][2]) then
			rotation = 180
			step = -1
			zShift = .5
			if(lessIsIn) then
				xShift = -.5
			else
				xShift = 1.5
			end
		else
			zShift = .5
			if(lessIsIn) then
				xShift = -.5
			else
				xShift = 1.5
			end
		end
		for x = StairCoords[1][1], StairCoords[2][1], step do
			for z = StairCoords[1][2], StairCoords[2][2], step do
				World_AddProp("data/models/stairs.obj", x + xShift, y, z + zShift, 0, dtr(rotation), 0, 1, .5, .5, true)
				y = y + 1
			end
		end
	elseif(stairType and math.abs(stairType) == 2) then
		for y = cornerY + 1, secondStoryY - 1 do
			for x = StairCoords[1][1], StairCoords[1][1] + 2 do
				for z = StairCoords[1][2], StairCoords[1][2] + 2 do
					World_SetVoxel(x, y, z, 1)
				end
			end
		end
	end
	return secondStoryY, stairType, StairCoords, floorType
end

function MakeLab(cornerX, cornerY, cornerZ, dimX, dimY, dimZ, DoorCoordList)
	-- Attempt second story and staircase generation
	local secondStoryY, stairType, StairCoords, floorType = MakeFloor(cornerX, cornerY, cornerZ, dimX, dimY, dimZ, DoorCoordList, {1, 2, 3})

	-- Place static models
	local scalar, generatorAngle  = .0015*min(dimX, dimY, dimZ), dtr(math.random(0, 3)*90)
	local xOffset, zOffset = -math.sin(generatorAngle)*scalar*28, math.cos(generatorAngle)*scalar*28
	World_AddProp("data/models/pistol_static.obj", cornerX + dimX/2, cornerY + 2.69, cornerZ + dimZ/2, 0, generatorAngle + .2, math.pi / 2, scalar * 16, scalar * 16, scalar * 16, true)
	World_AddProp("data/models/table.obj", cornerX + dimX/2 + xOffset, cornerY + 2.1 + scalar*25, cornerZ + dimZ/2 + zOffset, 0, generatorAngle, 0, scalar * 50, scalar * 50, scalar *50, true)
	if(stairType and floorType == 1) then
		print"111"
		World_AddProp("data/models/tree.obj", cornerX + dimX/2, cornerY + secondStoryY + 1, cornerZ + dimZ/2, 0, 0, 0, .25, .25, .25, true)
	end
end
function MakeOffice(cornerX, cornerY, cornerZ, dimX, dimY, dimZ, DoorCoordList)
	-- Attempt second story and staircase generation
	local secondStoryY, stairType, StairCoords = MakeFloor(cornerX, cornerY, cornerZ, dimX, dimY, dimZ, DoorCoordList, {1, 2})
end

function MakeStorageRoom(cornerX, cornerY, cornerZ, dimX, dimY, dimZ, DoorCoordList)
	-- Attempt second story and staircase generation
	local secondStoryY, stairtype, StairCoords
	if(math.random(1,2) == 1) then
		secondStoryY, stairType, StairCoords = MakeFloor(cornerX, cornerY, cornerZ, dimX, dimY, dimZ, DoorCoordList, {1, 3})
	end
	if(not secondStoryY) then
		secondStoryY = dimY
	end
end

-- Randomly generates lab, office, or storage room with dimensions dimX x dimY x dimZ
function MakeRoom(cornerX, cornerY, cornerZ, dimX, dimY, dimZ, DoorCoordList, roomType)
	local oppX = cornerX + dimX
	local oppY = cornerY + dimY
	local oppZ = cornerZ + dimZ
	local doors = {{}, {}, {}, {}}
	for _, doorCoord in ipairs(DoorCoordList) do
		if(doorCoord[1] == cornerX) then
			table.insert(doors[1], doorCoord)
		elseif(doorCoord[1] == oppX) then
			table.insert(doors[2], doorCoord)
		elseif(doorCoord[2] == cornerZ) then
			table.insert(doors[3], doorCoord)
		else
			table.insert(doors[4], doorCoord)
		end
	end

	-- Create room walls (2 is wall, 1 is floor, 3 is roof)
	FillBox(cornerX, cornerY, cornerZ, oppX, oppY, oppZ, 0)
	for x = cornerX + 1, oppX - 1 do
		for z = cornerZ + 1, oppZ - 1 do
			World_SetVoxel(x, cornerY, z, 1)
			World_SetVoxel(x, oppY, z, 3)
		end
	end
	-- Create doors
	for _, doorCoord in ipairs(doors[1]) do
		for y = cornerY + 1, cornerY + 3 do
			for z = doorCoord[2], doorCoord[2] + 1 do
				World_SetVoxel(cornerX, y, z, 0)
			end
		end

		World_AddDoor(cornerX + .5, cornerY + 1, doorCoord[2], math.pi / 2, true)
	end
	for _, doorCoord in ipairs(doors[2]) do
		for y = cornerY + 1, cornerY + 3 do
			for z = doorCoord[2], doorCoord[2] + 1 do
				World_SetVoxel(oppX, y, z, 0)
			end
		end

		World_AddDoor(oppX + .5, cornerY + 1, doorCoord[2], math.pi / 2, true)
	end
	for _, doorCoord in ipairs(doors[3]) do
		for y = cornerY + 1, cornerY + 3 do
			for x = doorCoord[1], doorCoord[1] + 1 do
				World_SetVoxel(x, y, cornerZ, 0)
			end
		end

		World_AddDoor(doorCoord[1], cornerY + 1, cornerZ + .5, 0, true)
	end
	for _, doorCoord in ipairs(doors[4]) do
		for y = cornerY + 1, cornerY + 3 do
			for x = doorCoord[1], doorCoord[1] + 1 do
				World_SetVoxel(x, y, oppZ, 0)
			end
		end

		World_AddDoor(doorCoord[1], cornerY + 1, oppZ + .5, 0, true)
	end

	-- Choose room to generate (1 is lab, 2 is office, 3 is storage
	if(not roomType) then
		roomType = math.random(1,3)
	end
	if(roomType == 1) then
		MakeLab(cornerX, cornerY, cornerZ, dimX, dimY, dimZ, doors)
	elseif(roomType == 2) then
		MakeOffice(cornerX, cornerY, cornerZ, dimX, dimY, dimZ, doors)
	else
		MakeStorageRoom(cornerX, cornerY, cornerZ, dimX, dimY, dimZ, doors)
	end

	-- Add light to the room
	lPosX = cornerX + dimX / 2
	lPosY = cornerY + dimY
	lPosZ = cornerZ + dimZ / 2

	World_AddProp("data/models/lamp_fluorescent.obj", lPosX, lPosY, lPosZ, 0, math.pi / 2, 0, 1, 1, 2, false)
	World_AddSound("data/sounds/fluorescentHum.wav", lPosX, lPosY, lPosZ, 0.07)
	--World_AddLight_Spot(lPosX, lPosY, lPosZ, 0.0, -1.0, 0.0, math.pi / 4.0, .8, .8, 1, 3)
	--World_AddLight_Point(lPosX, lPosY - 0.5, lPosZ, .8, .8, 1, 3)
end

-- Generate long corridor with random offshoots of rooms with high weapon and monster counts
function GenerateCorridorLevel()
	local Sides, cx, cy, cz, ox, oy, oz = {{}, {}}, 50, 100, 100, 550, 106, 105
	-- Place rooms on left and right sides of corridor
	for _, Rooms in ipairs(Sides) do
		local curPos = cx
		local LastHalls = {0, 0, 0}
		while(curPos < ox) do
			-- Randomly make no room, directly connected room, hallway room, or hallway room cluster
			if(math.random(1, 2) == 1) then
				local nextRoom = math.random(cx, curPos + 1000)
				if(nextRoom < cx + 500) then
					local roomLength = math.random(10, 30)
					table.insert(Rooms, {1, curPos, curPos + roomLength, curPos + math.floor(roomLength/2)})
				elseif(nextRoom < cx + 900) then
					local roomLength = 2
					local Angles = {}
					if(curPos - LastHalls[1] > 40) then
						table.insert(Angles, -1)
					end
					if(curPos - LastHalls[2] > 40) then
						table.insert(Angles, 0)
					end
					if(curPos - LastHalls[3] > 40) then
						table.insert(Angles, 1)
					end
					if(#Angles > 0) then
						local angle = Angles[math.random(1, #Angles)]
						table.insert(Rooms, {2, curPos, curPos + roomLength, curPos, angle})
						LastHalls[angle + 2] = curPos
					end
				else
					local roomLength = math.random(2, 4)
					table.insert(Rooms, {3, curPos, curPos + roomLength, curPos, math.random(1, 2) * 2 - 3})
				end
				curPos = Rooms[#Rooms][3]
			else
				curPos = curPos + math.random(5, 30)
			end
		end
	end
	-- Draw corridor
	FillBox(cx, cy, cz, ox, oy, oz, 0)

	for x = cx, ox do
		for z = cz + 1, oz - 1 do
			World_SetVoxel(x, cy, z, 1)
			World_SetVoxel(x, oy, z, 3)
		end

		-- Add a lamp every 20 steps
		if x % 20 == 0 then
			World_AddProp("data/models/lamp_fluorescent.obj", x, oy, cz + 3, 0, math.pi / 2, 0, 1, 1, 2, false)
			World_AddSound("data/sounds/fluorescentHum.wav", x, oy, cz + 3, 0.07)
			--World_AddLight_Spot(x, oy - .5, cz + 3, 0.0, -1.0, 0.0, math.pi / 4.0, .8, .8, 1, 2)
			--World_AddLight_Point(x, oy - .5, cz + 3, .8, .8, 1, 1)
		end
	end
	for i, Rooms in ipairs(Sides) do
		for _, Room in ipairs(Rooms) do
			-- Draw rooms
			if(Room[1] == 1) then
				local zSize = math.random(8, 20)
				if(i == 1) then
					MakeRoom(Room[2], cy, cz - zSize, Room[3] - Room[2], math.random(8, 20), zSize, {{Room[4], cz}}, 1)
				else
					MakeRoom(Room[2], cy, oz, Room[3] - Room[2], math.random(8, 20), zSize, {{Room[4], oz}}, 1)
				end
			elseif(Room[1] == 2) then
				local length, baseY, zStart, step = math.random(21, 25), cy
				if(i == 1) then
					zStart = cz
					step = -1
				else
					zStart = oz
					step = 1
				end
				for z = zStart, zStart + (length - 1) * step, step do
					for y = baseY, baseY + 5 do
						for x = Room[2], Room[3] - 1 do
							World_SetVoxel(x, y, z, 0)
						end
					end
					baseY = baseY + Room[5]
				end
				local dimX, dimY, dimZ = math.random(10, 30), math.random(8, 20), math.random(8, 20)
				MakeRoom(Room[2] - math.floor(dimX/2), baseY - Room[5], zStart + length * step - (1 - step)/2*dimZ, dimX, dimY, dimZ, {{Room[4], zStart + length*step}}, 1)
			end
		end
	end
end
World_Create(ox, 200, oz, "data/textures/voxelTexture_lowRes.png", "data/textures/voxelTexture_lowRes_specular.png", "data/textures/voxelTexture_lowRes_normalMap.bmp", "data/textures/descriptor.txt", 2, 4)
World_FillBox(0, 0, 0, 600, 600, 600, 2)
GenerateCorridorLevel()
