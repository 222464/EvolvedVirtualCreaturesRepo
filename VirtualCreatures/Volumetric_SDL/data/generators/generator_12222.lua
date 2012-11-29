mapSeed = os.time()
math.randomseed(mapSeed)

-- For rounding path generation
function round(num)
	return math.floor(num)
end

function min(...)
	local lowest
	for _, i in ipairs({...}) do
		if(not lowest or i < lowest) then
			lowest = i
		end
	end
	return lowest
end

function max(...)
	local biggest
	for _, i in ipairs({...}) do
		if(not biggest or i > biggest) then
			biggest = i
		end
	end
	return biggest
end

function FillBox(x1, y1, z1, x2, y2, z2, material)
	World_FillBox(x1 + 1, y1 + 1, z1 + 1, x2, y2, z2, material)
end

function dtr(angle)
	return angle*math.pi/180
end

function rotate(angle, px, py, sx, sy, ex, ey)
	local nsx, nsy = sx - px, sy - py
	if(ex and ey) then
		nex, ney = ex - px, ey - py
	end
	angle = dtr(angle)
	nsx, nsy = round(nsx*math.cos(angle) - nsy*math.sin(angle) + px), round(nsx*math.sin(angle) + nsy*math.cos(angle) + py)
	if(ex and ey) then
		nex, ney = round(nex*math.cos(angle) - ney*math.sin(angle) + px), round(nex*math.sin(angle) + ney*math.cos(angle) + py)
	end
	if(ex and ey and nex < nsx) then
		nsx, nex = nex, nsx
	end
	if(ex and ey and ney < nsy) then
		nsy, ney = ney, nsy
	end
	return nsx, nsy, nex, ney
end

function MakeStairs(WallStartEndCoords, storyHeight, lessIsIn, DoorCoords)
	local StairCoords = {{}, {}}
	if(storyHeight < 7) then
		return
	end
	local stairOrder = math.random(0, 4) -- 0 means try spiral first, otherwise try angled first
	local whichIsWall, whichIsntWall -- which coordinate does (not) vary along wall
	if(#WallStartEndCoords[1] == 1) then
		whichIsWall = 2
		whichIsntWall = 1
		table.insert(DoorCoords, {WallStartEndCoords[1][1], WallStartEndCoords[2][2] - 1})
	else
		whichIsWall = 1
		whichIsntWall = 2
		table.insert(DoorCoords, {WallStartEndCoords[1][2] - 1, WallStartEndCoords[2][1]})
	end
	table.sort(DoorCoords, function(A, B) if(A[whichIsWall] < B[whichIsWall]) then return true end end)
	whichIsntWall = whichIsWall % 2 + 1
	local makeAngled = function()
		local checkForward = function(DoorCoord, lastDoor)
			for i = DoorCoord[whichIsWall] - 4, lastDoor + 3, -1 do
				if(WallStartEndCoords[whichIsWall][2] - i > storyHeight) then
					StairCoords[1][whichIsWall] = math.random(lastDoor + 3, i)
					StairCoords[2][whichIsWall] = StairCoords[1][whichIsWall] + storyHeight - 1
					return true
				end
			end
		end
		local checkBackward = function(DoorCoord, lastDoor)
			for i = lastDoor + 5, DoorCoord[whichIsWall] - 2 do
				if(i - WallStartEndCoords[whichIsWall][1] > storyHeight) then
					StairCoords[1][whichIsWall] = math.random(i, DoorCoord[whichIsWall] - 2)
					StairCoords[2][whichIsWall] = StairCoords[1][whichIsWall] - storyHeight + 1
					return true
				end
			end
		end
		local firstCheck = math.random(0, 1)
		while(firstCheck < 6) do
			local lastDoor = WallStartEndCoords[whichIsWall][1]
			for _, DoorCoord in ipairs(DoorCoords) do
				local goodSpot
				if(firstCheck % 2 == 0) then
					goodSpot = checkForward(DoorCoord, lastDoor)
				else
					goodSpot = checkBackward(DoorCoord, lastDoor)
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
				lastDoor = DoorCoord[whichIsWall]
			end
			firstCheck = firstCheck + 3
		end
	end
	local makeSpiral = function()
		local lastDoor = WallStartEndCoords[whichIsWall][1] - 1
		for _, DoorCoord in ipairs(DoorCoords) do
			if(DoorCoord[whichIsWall] - lastDoor > 6) then
				StairCoords[1][whichIsWall] = math.random(lastDoor + 3, DoorCoord[whichIsWall] - 4)
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
			lastDoor = DoorCoord[whichIsWall]
		end
	end
	if(stairOrder == 0) then
		local storyHeight, stairType, StairCoords = makeSpiral()
		if(not storyHeight) then
			storyHeight, stairType, StairCoords = makeAngled()
		end
		table.remove(DoorCoords, #DoorCoords)
		return storyHeight, stairType, StairCoords
	else
		local storyHeight, stairType, StairCoords = makeAngled()
		if(not storyHeight) then
			storyHeight, stairType, StairCoords = makeSpiral()
		end
		table.remove(DoorCoords, #DoorCoords)
		return storyHeight, stairType, StairCoords
	end
end

function MakeFloor(cornerX, cornerY, cornerZ, dimX, dimY, dimZ, DoorCoordList, FloorTypes)
	-- Only consider x and z
	for _, DoorList in ipairs(DoorCoordList) do
		for _, Door in ipairs(DoorList) do
			table.remove(Door, 2)
		end
	end
	local oppX, oppY, oppZ = cornerX + dimX, cornerY + dimY, cornerZ + dimZ
	local WallOrder = {1, 2, 3, 4}
	for i, num in ipairs(WallOrder) do
		local toSwap = math.random(0, #WallOrder)
		WallOrder[i], WallOrder[toSwap] = WallOrder[toSwap], WallOrder[i]
	end
	local secondStoryY, stairType, StairCoords, lessIsIn
	for i = 1, 4 do
		if(not secondStoryY) then
			if(WallOrder[i] == 1) then
				secondStoryY, stairType, StairCoords = MakeStairs({{cornerX}, {cornerZ, oppZ}}, math.ceil(dimY/2), false, DoorCoordList[1])
				if(secondStoryY) then
					lessIsIn = false
				end
			elseif(WallOrder[i] == 2) then
				secondStoryY, stairType, StairCoords = MakeStairs({{oppX}, {cornerZ, oppZ}}, math.ceil(dimY/2), true, DoorCoordList[2])
				if(secondStoryY) then
					lessIsIn = true
				end
			elseif(WallOrder[i] == 3) then
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
	World_AddProp("data/models/tree.obj", cornerX + dimX/2, cornerY + 1, cornerZ + dimZ/2, 0, generatorAngle, 0, scalar, scalar, scalar, true)
	World_AddProp("data/models/table.obj", cornerX + dimX/2 + xOffset, cornerY + 1 + scalar*25, cornerZ + dimZ/2 + zOffset, 0, generatorAngle, 0, scalar, scalar, scalar, true)
	World_AddLight_Point(cornerX + dimX/2, cornerY + dimY - 1, cornerZ + dimZ/2, 1, 1, 1, 2)
	if(stairType) then
		World_AddLight_Point(cornerX + dimX/2, cornerY + secondStoryY - 1, cornerZ + dimZ/2, 1, 1, 1, 2)
		if(floorType == 1) then
			World_AddProp("data/models/pistol_static.obj", cornerX + dimX/2, cornerY + secondStoryY + 1, cornerZ + dimZ/2, 0, 0, 0, .04, .04, .04, true)
		end
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
function MakeRoom(cornerX, cornerY, cornerZ, oppX, oppY, oppZ, DoorCoordList, roomType)
	local dimX, dimY, dimZ = oppX - cornerX, oppY - cornerY, oppZ - cornerZ
	local Doors = {{}, {}, {}, {}}
	local output = function(a, DoorCoord)
		print(a)
		print("X:", cornerX, oppX)
		print("Y:", cornerY, oppY)
		print("Z:", cornerZ, oppZ)
		print(DoorCoord[1], DoorCoord[2], DoorCoord[3])
		print()
	end
	for _, DoorCoord in ipairs(DoorCoordList) do
		if(DoorCoord[1] == cornerX) then
			table.insert(Doors[1], DoorCoord)
			if(DoorCoord[3] > oppZ or DoorCoord[3] < cornerZ) then
				output(1, DoorCoord)
			end
		elseif(DoorCoord[1] == oppX) then
			table.insert(Doors[2], DoorCoord)
			if(DoorCoord[3] > oppZ or DoorCoord[3] < cornerZ) then
				output(2, DoorCoord)
			end
		elseif(DoorCoord[3] == cornerZ) then
			table.insert(Doors[3], DoorCoord)
			if(DoorCoord[1] > oppX or DoorCoord[1] < cornerX) then
				output(3, DoorCoord)
			end
		elseif(DoorCoord[3] == oppZ) then
			table.insert(Doors[4], DoorCoord)
			if(DoorCoord[1] > oppX or DoorCoord[1] < cornerX) then
				output(4, DoorCoord)
			end
		else
			output(5, DoorCoord)
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
	for _, DoorCoord in ipairs(Doors[1]) do
		for y = cornerY + 1, cornerY + 4 do
			for z = DoorCoord[3], DoorCoord[3] + 1 do
				World_SetVoxel(cornerX, y, z, 0)
			end
		end
	end
	for _, DoorCoord in ipairs(Doors[2]) do
		for y = cornerY + 1, cornerY + 4 do
			for z = DoorCoord[3], DoorCoord[3] + 1 do
				World_SetVoxel(oppX, y, z, 0)
			end
		end
	end
	for _, DoorCoord in ipairs(Doors[3]) do
		for y = cornerY + 1, cornerY + 4 do
			for x = DoorCoord[1], DoorCoord[1] + 1 do
				World_SetVoxel(x, y, cornerZ, 0)
			end
		end
	end
	for _, DoorCoord in ipairs(Doors[4]) do
		for y = cornerY + 1, cornerY + 4 do
			for x = DoorCoord[1], DoorCoord[1] + 1 do
				World_SetVoxel(x, y, oppZ, 0)
			end
		end
	end

	-- Choose room to generate (1 is lab, 2 is office, 3 is storage)
	if(not roomType) then
		roomType = math.random(1,3)
	end
	if(roomType == 1) then
		MakeLab(cornerX, cornerY, cornerZ, dimX, dimY, dimZ, Doors)
	elseif(roomType == 2) then
		MakeOffice(cornerX, cornerY, cornerZ, dimX, dimY, dimZ, Doors)
	else
		MakeStorageRoom(cornerX, cornerY, cornerZ, dimX, dimY, dimZ, Doors)
	end
end

-- Generate long corridor with random offshoots of rooms with high weapon and monster counts
function GenerateCorridor(cx, cy, cz, ox, oy, oz, angle, hallStart, hallStop, roomStart, roomStop)
	-- Sides contains one table corresponding to all rooms on a side of the corridor for each side
	local Sides = {{}, {}}
	local addRoom = function(Table, hXS, hXE, RSC, REC, AdjRs, Ds, Ps, hH)
		if(not hXE and hXS) then
			hXE = hXS + 2
		end
		table.insert(Table, {hallXStart = hXS, hallXEnd = hXE, RoomSCoord = RSC, RoomECoord = REC, SecondaryRooms = AdjRs, Doors = Ds, Paths = Ps, hasHallway = hH})
	end
	local makeCorridorRightHalf = function(Rooms)
		local curPos, LastHalls = roomStart, {[-1] = 0, [0] = 0, [1] = 0}
		while(curPos < roomStop) do
			local numAdjHallRooms = math.random(-1, 3)
			for i = 1, numAdjHallRooms do
				if(roomStop - curPos < 10) then
					curPos = roomStop
					break
				end
				local hasHallway
				if(math.random(0, 2) == 0) then
					hasHallway = true
				end
				local xSizeMax = min(30, roomStop - curPos)
				local xSize, ySize, zSize, hallEnd = math.random(10, xSizeMax), math.random(8, 20), math.random(8, 20)
				local Paths, SecondaryRooms, Doors, xCorner, yCorner, zCorner = {}, {}, {}
				local hallStraightLength = 5  -- Must remain constant
				local LastRoom = Rooms[#Rooms]

				-- Check to see if it is possible to make a hall
				local Angles = {}
				for ang, coord in pairs(LastHalls) do
					if(curPos > coord + 3) then
						table.insert(Angles, ang)
					end
				end
				-- Get xCorner, yCorner, zCorner, hallEnd, Doors, SecondaryRooms, and Paths, call addRoom, and update LastHalls and curPos
				if(hasHallway and #Angles > 0 and curPos < hallStop and curPos >= hallStart) then
					local angle, hallAngLength = Angles[math.random(1, #Angles)], math.random(21, 25)
					curPos = curPos + 1
					if(i > 1) then
						if(LastRoom.hasHallway) then
							local pathZ = math.random(oz + 1, oz + hallStraightLength - 3)
							table.insert(Paths, {{curPos, cy, pathZ}, {curPos - 2, cy, pathZ}})
						else
							table.insert(LastRoom.Doors, {LastRoom.RoomECoord[1], cy, math.random(oz + 1, oz + hallStraightLength - 3)})
						end
					end
					xCorner = math.random(max(curPos - xSize + 5, LastHalls[angle] + 2), curPos - 2)
					yCorner = cy + hallAngLength * angle
					zCorner = oz + hallAngLength + hallStraightLength

					table.insert(Doors, {curPos, yCorner, zCorner})
					table.insert(Paths, {{curPos, cy, oz}, {curPos, cy, oz + hallStraightLength}})
					table.insert(Paths, {{curPos, cy, oz + hallStraightLength}, {curPos, yCorner, zCorner}})


					-- Add additional rooms if possible, starting with backward-x room
					local BackXRoom, ForwardXRoom, ForwardZRoom = {}, {}, {}
					if(xCorner > LastHalls[angle] + 8 and math.random(1, 2) == 1) then
						local zCornerTwo = math.random(zCorner, zCorner + zSize - 6)
						local zOppTwo = math.random(zCornerTwo + 6, zCorner + zSize)
						local doorZ = math.random(zCornerTwo + 1, zOppTwo - 3)
						addRoom(BackXRoom, nil, nil, {math.random(max(LastHalls[angle] + 2, xCorner - 12), xCorner - 6), yCorner, zCornerTwo}, {xCorner, yCorner + math.random(6, 12), zOppTwo}, {}, {{xCorner, yCorner, doorZ}}, {}, false)
						table.insert(Doors, {xCorner, yCorner, doorZ})
						-- Make path to previous room if possible
					end
					-- Forward-x room
					if(math.random(1, 2) == 1) then
						local zCornerTwo = math.random(zCorner, zCorner + zSize - 6)
						local zOppTwo, xOppTwo = math.random(zCornerTwo + 6, zCorner + zSize), xCorner + xSize + math.random(6, 12)
						local doorZ = math.random(zCornerTwo + 1, zOppTwo - 3)
						addRoom(ForwardXRoom, nil, nil, {xCorner + xSize, yCorner, zCornerTwo}, {xOppTwo, yCorner + math.random(6, 12), zOppTwo}, {}, {{xCorner + xSize, yCorner, doorZ}}, {}, false)
						table.insert(Doors, {xCorner + xSize, yCorner, doorZ})
						LastHalls[angle] = xOppTwo
						-- Make path from new room
						local pathX = math.random(xCorner + xSize + 1, xOppTwo - 3)
						if(math.random(1, 3) ~= 1 and pathX < roomStop - 3) then
							local hallStraightLength = zCornerTwo - oz - math.abs(yCorner - cy)
							table.insert(Paths, {{pathX, cy, oz}, {pathX, cy, oz + hallStraightLength}})
							table.insert(Paths, {{pathX, cy, oz + hallStraightLength}, {pathX, yCorner, zCornerTwo}})
							table.insert(ForwardXRoom[1].Doors, {pathX, yCorner, zCornerTwo})
							hallEnd = pathX + 2
						end
					else
						LastHalls[angle] = xCorner + xSize
					end
					-- Forward-z room
					if(math.random(1, 2) == 1) then
						local xMin, xMax = xCorner, xCorner + xSize
						if(#BackXRoom > 0) then
							xMin = BackXRoom[1].RoomSCoord[1]
						end
						if(#ForwardXRoom > 0) then
							xMax = ForwardXRoom[1].RoomECoord[1]
						end
						local xCornerTwo = math.random(xMin, min(xMax - 6, xCorner + xSize - 4))
						local xOppTwo = math.random(max(xCornerTwo + 6, xCorner + 4), xMax)
						local doorX = math.random(max(xCornerTwo + 1, xCorner + 1), min(xOppTwo - 3, xCorner + xSize - 3))
						addRoom(ForwardZRoom, nil, nil, {xCornerTwo, yCorner, zCorner + zSize}, {xOppTwo, yCorner + math.random(6, 12), zCorner + zSize + math.random(6, 12)}, {}, {{doorX, yCorner, zCorner + zSize}}, {}, false)
						table.insert(Doors, {doorX, yCorner, zCorner + zSize})
					end
					if(#BackXRoom > 0) then
						table.insert(SecondaryRooms, BackXRoom[1])
					end
					if(#ForwardXRoom > 0) then
						table.insert(SecondaryRooms, ForwardXRoom[1])
					end
					if(#ForwardZRoom > 0) then
						table.insert(SecondaryRooms, ForwardZRoom[1])
					end
				else
					hasHallway = false
					xCorner = curPos
					yCorner = cy
					zCorner = oz
					hallEnd = xCorner + xSize
					table.insert(Doors, {math.random(xCorner + 1, min(xCorner + xSize - 3, ox - 3)), yCorner, oz})
					if(i > 1) then
						local doorZ
						if(LastRoom.hasHallway) then
							doorZ = math.random(oz + 1, oz + hallStraightLength - 3)
						else
							doorZ = math.random(oz + 1, min(LastRoom.RoomECoord[3], zCorner + zSize) - 3)
							table.insert(LastRoom.Doors, {LastRoom.RoomECoord[1], yCorner, doorZ})
						end
						table.insert(Doors, {xCorner, yCorner, doorZ})
					end
					-- Add second room if enough space
					if(curPos > LastHalls[0] + 3 and math.random(1, 2) == 1) then
						local xSizeTwo = math.random(10, 30)
						local xBound
						if(LastRoom) then
							xBound = LastRoom.hallXEnd
						else
							xBound = cx
						end
						local xCornerTwo = math.random(max(curPos - xSizeTwo + 5, LastHalls[0] + 2, xBound), curPos)
						if(xCornerTwo + xSizeTwo > xCorner + xSize) then
							xSizeTwo = xCorner + xSize - xCornerTwo
						end
						local doorX = math.random(max(xCorner, xCornerTwo) + 1, min(xCorner + xSize, xCornerTwo + xSizeTwo) - 4)
						addRoom(SecondaryRooms, nil, nil, {xCornerTwo, yCorner, zCorner + zSize}, {xCornerTwo + xSizeTwo, cy + math.random(8, 20), zCorner + zSize + math.random(8, 20)}, {}, {{doorX, yCorner, zCorner + zSize}}, {}, false)
						table.insert(Doors, {doorX, yCorner, zCorner + zSize})
						LastHalls[0] = xCornerTwo + xSizeTwo
					end
				end
				addRoom(Rooms, curPos, hallEnd, {xCorner, yCorner, zCorner}, {xCorner + xSize, yCorner + ySize, zCorner + zSize}, SecondaryRooms, Doors, Paths, hasHallway)
				curPos = Rooms[#Rooms].hallXEnd
			end
			curPos = curPos + math.random(5, 15)
		end
	end

	makeCorridorRightHalf(Sides[1])

	local LeftHalf = {}
	makeCorridorRightHalf(LeftHalf)
	for _, Room in ipairs(LeftHalf) do
		local SecondaryRooms, Doors, Paths = {}, {}, {}
		for _, NextRoom in ipairs(Room.SecondaryRooms) do
			local Doors, Paths = {}, {}
			for _, Door in ipairs(NextRoom.Doors) do
				if(Door[1] == NextRoom.RoomSCoord[1] or Door[1] == NextRoom.RoomECoord[1]) then
					table.insert(Doors, {Door[1], Door[2], cz+oz-Door[3]-1})
				else
					table.insert(Doors, {Door[1], Door[2], cz+oz-Door[3]})
				end
			end
			for _, Path in ipairs(NextRoom.Paths) do
				table.insert(Paths, {{Path[2][1], Path[2][2], cz+oz-Path[2][3]}, {Path[1][1], Path[1][2], cz+oz-Path[1][3]}})
			end
			addRoom(SecondaryRooms, NextRoom.hallXStart, NextRoom.hallXEnd, {NextRoom.RoomSCoord[1], NextRoom.RoomSCoord[2], cz+oz-NextRoom.RoomECoord[3]}, {NextRoom.RoomECoord[1], NextRoom.RoomECoord[2], cz+oz-NextRoom.RoomSCoord[3]}, {}, Doors, Paths)
		end
		for _, Door in ipairs(Room.Doors) do
			if(Door[1] == Room.RoomSCoord[1] or Door[1] == Room.RoomECoord[1]) then
				table.insert(Doors, {Door[1], Door[2], cz+oz-Door[3]-1})
			else
				table.insert(Doors, {Door[1], Door[2], cz+oz-Door[3]})
			end
		end
		for _, Path in ipairs(Room.Paths) do
			table.insert(Paths, {{Path[2][1], Path[2][2], cz+oz-Path[2][3]}, {Path[1][1], Path[1][2], cz+oz-Path[1][3]}})
		end
		addRoom(Sides[2], Room.hallXStart, Room.hallXEnd, {Room.RoomSCoord[1], Room.RoomSCoord[2], cz+oz-Room.RoomECoord[3]}, {Room.RoomECoord[1], Room.RoomECoord[2], cz+oz-Room.RoomSCoord[3]}, SecondaryRooms, Doors, Paths)
	end

	for _, Rooms in ipairs(Sides) do
		for _, Room in ipairs(Rooms) do
			for _, Door in ipairs(Room.Doors) do
				if(Door[1] == Room.RoomSCoord[1] or Door[1] == Room.RoomECoord[1]) then
					Door[1], Door[3] = rotate(angle, cx, cz, Door[1], Door[3], Door[1], Door[3] + 1)
				else
					Door[1], Door[3] = rotate(angle, cx, cz, Door[1], Door[3], Door[1] + 1, Door[3])
				end
			end
			Room.RoomSCoord[1], Room.RoomSCoord[3], Room.RoomECoord[1], Room.RoomECoord[3] = rotate(angle, cx, cz, Room.RoomSCoord[1], Room.RoomSCoord[3], Room.RoomECoord[1], Room.RoomECoord[3])
			for _, Path in ipairs(Room.Paths) do
				local distanceInXZ = math.sqrt((Path[2][1] - Path[1][1])^2 + (Path[2][3] - Path[1][3])^2)
				local PerpendicularInXZ = {math.abs((Path[2][3] - Path[1][3])/distanceInXZ), math.abs((Path[1][1] - Path[2][1])/distanceInXZ)}
				Path[1][1], Path[1][3] = rotate(angle, cx, cz, Path[1][1], Path[1][3])
				Path[2][1], Path[2][3] = rotate(angle, cx, cz, Path[2][1], Path[2][3])
			end
			for _, Room in ipairs(Room.SecondaryRooms) do
				for _, Door in ipairs(Room.Doors) do
					if(Door[1] == Room.RoomSCoord[1] or Door[1] == Room.RoomECoord[1]) then
						Door[1], Door[3] = rotate(angle, cx, cz, Door[1], Door[3], Door[1], Door[3] + 1)
					else
						Door[1], Door[3] = rotate(angle, cx, cz, Door[1], Door[3], Door[1] + 1, Door[3])
					end
				end
				Room.RoomSCoord[1], Room.RoomSCoord[3], Room.RoomECoord[1], Room.RoomECoord[3] = rotate(angle, cx, cz, Room.RoomSCoord[1], Room.RoomSCoord[3], Room.RoomECoord[1], Room.RoomECoord[3])
			end
		end
	end

	-- Draw corridor
	local ncx, ncz, nox, noz = rotate(angle, cx, cz, cx, cz, ox, oz)
	FillBox(ncx, cy, ncz, nox, oy, noz, 0)
	for x = cx, ox do
		if(x % 8 == 0) then
			local nx, nz = rotate(angle, cx, cz, x, cz + math.floor((oz - cz)/2))
			World_AddLight_Point(nx, oy - 1, nz, 1, 1, 1, 4)
		end
		for z = cz, oz do
			local nx, nz = rotate(angle, cx, cz, x, z)
			World_SetVoxel(nx, cy, nz, 1)
			World_SetVoxel(nx, oy, nz, 3)
		end
	end
	-- Draw rooms
	for _, Rooms in ipairs(Sides) do
		for _, Room in ipairs(Rooms) do
			local S, E = Room.RoomSCoord, Room.RoomECoord
			MakeRoom(S[1], S[2], S[3], E[1], E[2], E[3], Room.Doors, 1)
			for _, Path in ipairs(Room.Paths) do
				local distance = math.sqrt((Path[2][1] - Path[1][1])^2 + (Path[2][2] - Path[1][2])^2 + (Path[2][3] - Path[1][3])^2)
				local Direction = {(Path[2][1] - Path[1][1])/distance, (Path[2][2] - Path[1][2])/distance, (Path[2][3] - Path[1][3])/distance}
				local distanceInXZ = math.sqrt((Path[2][1] - Path[1][1])^2 + (Path[2][3] - Path[1][3])^2)
				local PerpendicularInXZ = {(Path[2][3] - Path[1][3])/distanceInXZ, (Path[1][1] - Path[2][1])/distanceInXZ}
				for t = 0, math.ceil(distance) do
					for p = 0, 1 do
						for dy = 1, 4 do
							World_SetVoxel(round(Direction[1]*t + Path[1][1] + PerpendicularInXZ[1]*p), round(Direction[2]*t + Path[1][2]) + dy, round(Direction[3]*t + Path[1][3] + PerpendicularInXZ[2]*p), 0)
							if(t % 8 == 0 and p == 0 and dy == 2) then
								World_AddLight_Point(math.ceil(Direction[1]*t + Path[1][1] + PerpendicularInXZ[1]*p), math.ceil(Direction[2]*t + Path[1][2]) + dy, math.ceil(Direction[3]*t + Path[1][3] + PerpendicularInXZ[2]*p), 1, 1, 1, 2)
							end
						end
					end
				end
			end
			for _, NextRoom in ipairs(Room.SecondaryRooms) do
				local S, E = NextRoom.RoomSCoord, NextRoom.RoomECoord
				MakeRoom(S[1], S[2], S[3], E[1], E[2], E[3], NextRoom.Doors, 1)
			end
		end
	end
end

function MakeCorridorLevel(hallLength, numMainHalls)
	ox, oz = 600, 600
	World_Create(ox, 192, oz, "data/textures/voxelTexture_lowRes.png", "data/textures/voxelTexture_lowRes_specular.png", "data/textures/descriptor.txt", 2, 4)
	World_FillBox(0, 0, 0, 600, 200, 600, 2)
	GenerateCorridor(100, 100, 300, 200, 106, 306, 0, 100, 175, 100, 195)
	local prev, Loc = 1, {200, 300}
	for i = 1, numMainHalls - 1 do
		local NextDir = {1}
		if(Loc[2] > hallLength + 25 and prev ~= 2) then
			table.insert(NextDir, 3)
		end
		if(Loc[2] < oz - hallLength - 25 and prev ~= 3) then
			table.insert(NextDir, 2)
		end
		local curPos = math.random(1, #NextDir)
		local current, sub = NextDir[curPos]
		table.remove(NextDir, curPos)
		if(#NextDir > 0) then
			sub = NextDir[math.random(1, #NextDir)]
		end
		local Doors, xCorner, zCorner, xOpp, zOpp = {}
		print(prev, current, sub)
		if(prev == 1) then
			xCorner, xOpp, zCorner, zOpp = Loc[1], Loc[1] + math.random(10, 25), Loc[2] - math.random(5, 10), Loc[2] + math.random(11, 16)
			table.insert(Doors, {Loc[1], 100, Loc[2] + 2})
		elseif(prev == 2) then
			xCorner, xOpp, zCorner, zOpp = Loc[1] - math.random(5, 10), Loc[1] + math.random(11, 16), Loc[2], Loc[2] + math.random(10, 25)
			table.insert(Doors, {Loc[1] + 2, 100, Loc[2]})
		else
			xCorner, xOpp, zCorner, zOpp = Loc[1] - math.random(5, 10), Loc[1] + math.random(11, 16), Loc[2] - math.random(10, 25), Loc[2]
			table.insert(Doors, {Loc[1] + 2, 100, Loc[2]})
		end
		if(current == 1) then
			local zDoor = zCorner + math.ceil((zOpp - zCorner)/2)
			table.insert(Doors, {xOpp, 100, zDoor})
			local hallStop
			if(i == 3) then
				hallStop = xOpp + hallLength
			else
				hallStop = xOpp + hallLength - 25
			end
			local roomStart
			if(current == prev) then
				roomStart = xOpp + 5
			else
				roomStart = xOpp + 25
			end
			GenerateCorridor(xOpp, 100, zDoor - 2, xOpp + hallLength, 106, zDoor + 4, 0, xOpp + 25, hallStop, roomStart, xOpp + hallLength - 5)
			Loc[1], Loc[2] = xOpp + hallLength, zDoor
		elseif(current == 2) then
			local xDoor = xCorner + math.ceil((xOpp - xCorner)/2)
			table.insert(Doors, {xDoor, 100, zOpp})
			local xShift = 3
			local hallStop
			if(i == 3) then
				hallStop = xDoor + hallLength + xShift
			else
				hallStop = xDoor + hallLength - 25 + xShift
			end
			local roomStart
			if(current == prev) then
				roomStart = xDoor + 5 + xShift
			else
				roomStart = xDoor + 25 + xShift
			end
			GenerateCorridor(xDoor + xShift, 100, zOpp, xDoor + hallLength + xShift, 106, zOpp + 6, 90, xDoor + 25 + xShift, hallStop, roomStart, xDoor + hallLength - 5 + xShift)
			Loc[1], Loc[2] = xDoor, zOpp + hallLength
		else
			local xDoor = xCorner + math.ceil((xOpp - xCorner)/2)
			table.insert(Doors, {xDoor, 100, zCorner})
			local xShift = -2
			local hallStop
			if(i == 3) then
				hallStop = xDoor + hallLength + xShift
			else
				hallStop = xDoor + hallLength + xShift
			end
			local roomStart
			if(current == prev) then
				roomStart = xDoor + 5 + xShift
			else
				roomStart = xDoor + 25 + xShift
			end
			GenerateCorridor(xDoor + xShift, 100, zCorner, xDoor + hallLength + xShift, 106, zCorner + 6, -90, xDoor + 25 + xShift, hallStop, roomStart, xDoor + hallLength - 5 + xShift)
			Loc[1], Loc[2] = xDoor, zCorner - hallLength
		end
		if(sub == 1) then
			local zDoor = zCorner + math.ceil((zOpp - zCorner)/2)
			table.insert(Doors, {xOpp, 100, zDoor})
			local roomStart
			if(sub == prev) then
				roomStart = xOpp + 5
			else
				roomStart = xOpp + 25
			end
			GenerateCorridor(xOpp, 100, zDoor - 2, xOpp + hallLength, 106, zDoor + 4, 0, xOpp + 25, xOpp + 100, roomStart, xOpp + hallLength)
		elseif(sub == 2) then
			local xDoor = xCorner + math.ceil((xOpp - xCorner)/2)
			table.insert(Doors, {xDoor, 100, zOpp})
			local xShift = 3
			local roomStart
			if(sub == prev) then
				roomStart = xDoor + 5
			else
				roomStart = xDoor + 25
			end
			GenerateCorridor(xDoor + xShift, 100, zOpp, xDoor + hallLength + xShift, 106, zOpp + 6, 90, xDoor + 25 + xShift, xDoor + 100 + xShift, roomStart, xDoor + hallLength - 5 + xShift)
		elseif(sub == 3) then
			local xDoor = xCorner + math.ceil((xOpp - xCorner)/2)
			table.insert(Doors, {xDoor, 100, zCorner})
			local xShift = -2
			local roomStart
			if(sub == prev) then
				roomStart = xDoor + 5 + xShift
			else
				roomStart = xDoor + 25
			end
			GenerateCorridor(xDoor + xShift, 100, zCorner, xDoor + hallLength + xShift, 106, zCorner + 6, -90, xDoor + 25 + xShift, xDoor + 100 + xShift, roomStart, xDoor + hallLength - 5 + xShift)
		end
		print(xCorner, zCorner, xOpp, zOpp)
		MakeRoom(xCorner, 100, zCorner, xOpp, 100 + math.random(10, 20), zOpp, Doors, 1)
		prev = current
	end
end

MakeCorridorLevel(90, 3)
