--function cache
local math_floor = math.floor
local math_fmod = math.fmod
local string_gmatch = string.gmatch
local string_gsub = string.gsub
local string_len = string.len

local heap = 2048
local file
local filename

local columns = {}
local sessions = {}
local sessionCount = 0
local selectedSession = 0

local STEP_SELECTFILE = 0
local STEP_READFILE = 1
local STEP_SELECTSESSION = 2
local STEP_READSESSION = 3
local STEP_PARSEDATA = 4
local STEP_GRAPH = 5

local step = STEP_SELECTFILE

local points = {}

local conversionStep = 0
local conversionIndex = 0

--File reading data
local valPos = 0
local skipLines = 0
local lines = 0
local index = 0
local buffer = ""
local prevTotalSeconds = 0

--Option data
local maxLines

local current_option = 1

local fileSelection = {
	{ y = 40, label = "Logfile", values = {}, value = -1, min = 1 },
	{ y = 60, label = "Speed", values = { "Accurate", "Fast", "Use index" }, value = 1, min = 1, max = 2 }
}

local dataSelection = {
	{ y = 40, label = "Session", values = {}, value = 1, min = 1 },
	{ y = 100, label = "Var 1", values = {}, value = 1, min = 0 },
	{ y = 120, label = "Var 2", values = {}, value = 0, min = 0 },
	{ y = 140, label = "Var 3", values = {}, value = 0, min = 0 },
	{ y = 160, label = "Var 4", values = {}, value = 0, min = 0 },
	{ y = 180, label = "Speed", values = { "Accurate", "Faster", "Fast", "Fastest" }, value = 1, min = 1 }
}

--Graph data
local values = {}

local graphConfig = {
	xstart = 60, xend = 420,
	{ color = RED, minx = 5, maxx = 5, miny = 220, maxy = 40, valx = 5, valy = 249 },
	{ color = BLUE, minx = 430, maxx = 430, miny = 220, maxy = 40, valx = 120, valy = 249  },
	{ color = YELLOW, minx = 5, maxx = 5, miny = 200, maxy = 60, valx = 235, valy = 249  },
	{ color = GREY, minx = 430, maxx = 430, miny = 200, maxy = 60, valx = 350, valy = 249  }
}

local xStep = (graphConfig.xend - graphConfig.xstart) / 100

local cursor = 0

local GRAPH_CURSOR = 0
local GRAPH_ZOOM = 1
local GRAPH_SCROLL = 2
local GRAPH_MINMAX = 3
local graphMode = GRAPH_CURSOR
local graphStart = 0
local graphSize = 0
local graphTimeBase = 0
local graphMinMaxIndex = 0

local function doubleDigits(value)
	if value < 10 then
		return "0" .. value
	else
		return value
	end
end

local function toDuration(totalSeconds)
	local hours = math_floor(totalSeconds / 3600)
	totalSeconds = totalSeconds - (hours * 3600)
	local minutes = math_floor(totalSeconds / 60)
	local seconds = totalSeconds - (minutes * 60)

	return doubleDigits(hours) .. ":" .. doubleDigits(minutes) .. ":" .. doubleDigits(seconds);
end

local function split(text)
	local cnt = 0
	local result = {}
	for val in string_gmatch(string_gsub(text, ",,", ", ,"), "([^,]+),?") do
		result[cnt] = val
		cnt = cnt + 1
	end

	return result, cnt
end

local function getTotalSeconds(time)
	local total = tonumber(string.sub(time, 1, 2)) * 3600
	total = total + tonumber(string.sub(time, 4, 5)) * 60
	total = total + tonumber(string.sub(time, 7, 8))

	return total
end

local function readHeader(file)
	local read = io.read(file, 2048)

	index = string.find(read, "\n")

	if index ~= nil then
		io.seek(file, index)

		local headerLine = string.sub(read, 0, index - 1)

		columns = split(headerLine)
		return true
	end

	return false
end

local function readIndex()
	sessions = {}
	sessionCount = 0
	
	local indexFile = io.open("/LOGS/" .. filename .. ".index", "r")

	buffer = io.read(indexFile, 2048 * 32)

	for line in string_gmatch(buffer, "([^\n]+)\n") do
		local values = split(line)

		sessions[sessionCount] = { 
			startTime = values[0], 
			endTime = values[1],
			startLine = tonumber(values[2]),
			endLine = tonumber(values[3]),
			startIndex = tonumber(values[4]),
			endIndex = tonumber(values[5])
		}

		sessionCount = sessionCount + 1
		selectedSession = sessionCount - 1
	end

	io.close(file)
end

local function saveIndex()
	local indexFile = io.open("/LOGS/" .. filename .. ".index", "w")

	for i = 0, #sessions, 1 do
		local session = sessions[i]

		local line = session.startTime .. "," .. session.endTime .. "," .. session.startLine .. "," .. session.endLine .. "," .. session.startIndex .. "," .. session.endIndex .. "\n"

		io.write(indexFile, line)
	end

	io.close(file)
end

local function readFile()
	local read = io.read(file, heap)

	if read == "" then
		io.close(file)
		return true
	end

	local indexOffset = string_len(buffer)

	buffer = buffer .. read
	local i = 0

	for line in string_gmatch(buffer, "([^\n]+)\n") do
		lines = lines + 1

		if math_fmod(lines, skipLines) == 0 then
			local time = string.sub(line, 12, 19)
			local totalSeconds = getTotalSeconds(time);

			if (totalSeconds - prevTotalSeconds) > 60 then
				sessions[sessionCount] = { startTime = time, startLine = lines, startIndex = index + i - indexOffset }
				sessionCount = sessionCount + 1
				selectedSession = sessionCount - 1
			end

			sessions[sessionCount - 1].endTime = time
			sessions[sessionCount - 1].endLine = lines
			sessions[sessionCount - 1].endIndex = index + i

			prevTotalSeconds = totalSeconds
		end

		i = i + string_len(line) + 1 --dont forget the newline ;)
	end

	buffer = string.sub(buffer, i + 1) --dont forget the newline ;
	index = index + heap
	io.seek(file, index)

	return false
end

local function collectData(file)
	local read = io.read(file, heap)

	if read == "" then
		io.close(file)
		return true
	end

	buffer = buffer .. read
	local i = 0

	for line in string_gmatch(buffer, "([^\n]+)\n") do
		if math_fmod(lines, skipLines) == 0 then
			vals = split(line)

			for varIndex = 2, 5, 1 do
				if dataSelection[varIndex].value ~= 0 then
					values[varIndex - 1][valPos] = vals[dataSelection[varIndex].value + 1]
				end
			end

			valPos = valPos + 1
		end

		lines = lines + 1

		if lines > maxLines then
			io.close(file)
			return true
		end

		i = i + string_len(line) + 1 --dont forget the newline ;)
	end

	buffer = string.sub(buffer, i + 1) --dont forget the newline ;
	index = index + heap
	io.seek(file, index)

	return false
end

local function initFile()
	file = io.open("/LOGS/" .. filename, "r")

	if file == nil then
		return "Cannot open file?"
	end

	if not readHeader(file) then
		return "Header could not be found"
	end

	return nil
end

local function drawMain()
	lcd.clear()
	lcd.drawFilledRectangle(0, 0, LCD_W, LCD_H, TEXT_BGCOLOR)
	lcd.drawFilledRectangle(0, 0, LCD_W, 30, TITLE_BGCOLOR)

	if filename ~= nil then
		lcd.drawText(1, 5, "Filename: " .. filename, MENU_TITLE_COLOR)
	end
end

local function drawOption(y, label, value, select_index)
	local activeOption = current_option == select_index
	
	lcd.drawText(5, y, label, TEXT_COLOR)

	if activeOption then
		lcd.drawText(100, y, value, TEXT_INVERTED_COLOR + INVERS)
	else
		lcd.drawText(100, y, value, TEXT_COLOR)
	end
end

local function drawOptions(options)
	if current_option > #options then
		current_option = #options
	end

	for i = 1, #options, 1 do
		option = options[i]
		if option.value > #option.values then
			option.value = #option.values
		elseif option.value < option.min then
			option.value = option.min
		end
		drawOption(option.y, option.label, option.values[option.value], i)
	end
end

local function handleOptionInput(options, event)
	if event == EVT_PAGE_BREAK or event == EVT_PAGEDN_FIRST then
		current_option = current_option + 1
	elseif event == EVT_PAGEUP_FIRST then
		current_option = current_option - 1
	end

	if current_option < 1 then
		current_option = 1
	elseif current_option > #options then
		current_option = #options
	end

	option = options[current_option]

	if event == EVT_PLUS_FIRST or event == EVT_ROT_RIGHT or event == EVT_PLUS_REPT then
		option.value = option.value + 1
	elseif event == EVT_MINUS_FIRST or event == EVT_ROT_LEFT or event == EVT_MINUS_REPT then
		option.value = option.value - 1
	end

	local maxOption = #option.values

	if option.max ~= nil then
		maxOption = option.max
	end

	if option.value < option.min then
		option.value = option.min
	elseif option.value > maxOption then
		option.value = maxOption
	end
end

local function drawGraph_base()
	lcd.drawLine(graphConfig.xstart, 40, graphConfig.xstart, 240, SOLID, CUSTOM_COLOR)
	lcd.drawLine(graphConfig.xstart, 240, graphConfig.xend, 240, SOLID, CUSTOM_COLOR)
	lcd.drawLine(graphConfig.xend, 40, graphConfig.xend, 240, SOLID, CUSTOM_COLOR)
	lcd.drawLine(graphConfig.xstart, 40, graphConfig.xend, 40, DOTTED, CUSTOM_COLOR)

	local x = graphConfig.xstart + (xStep * cursor)

	lcd.drawLine(x, 40, x, 240, DOTTED, CUSTOM_COLOR)

	if graphMode == GRAPH_CURSOR then
		lcd.drawText(430, 130, "Curs.", CUSTOM_COLOR)
	elseif graphMode == GRAPH_ZOOM then
		lcd.drawText(430, 130, "Zoom", CUSTOM_COLOR)
	elseif graphMode == GRAPH_MINMAX then
		lcd.drawText(430, 130, "Mi-Ma", CUSTOM_COLOR)
	else
		lcd.drawText(430, 130, "Scroll", CUSTOM_COLOR)
	end
end

local function drawGraph_points(points, min, max)
	if min == max then
		return
	end

	local yScale = (max - min) / 200

	prevY = 240 - ((points[0] - min) / yScale)

	if prevY > 240 then
		prevY = 240
	elseif prevY < 40 then
		prevY = 40
	end

	for i = 0, #points - 1, 1 do
		x1 = graphConfig.xstart + (xStep * i)
		y = 240 - ((points[i + 1] - min) / yScale)

		if y > 240 then
			y = 240
		elseif y < 40 then
			y = 40
		end

		lcd.drawLine(x1, prevY, x1 + xStep, y, SOLID, CUSTOM_COLOR)

		prevY = y
	end
end

local function drawGraph()
	skip = graphSize / 101

	lcd.setColor(CUSTOM_COLOR, BLACK)
	drawGraph_base()

	local cursorLine = math_floor((graphStart + (cursor * skip)) / graphTimeBase)
	local cursorTime = toDuration(cursorLine)

	if cursorLine < 3600 then
		cursorTime = string.sub(cursorTime, 4)
	end

	lcd.drawText(5, 130, cursorTime, CUSTOM_COLOR)

	--Draw session view locations
	local viewScale = valPos / 480
	local viewStart = math.floor(graphStart / viewScale)
	local viewEnd = math.floor((graphStart + graphSize) / viewScale)

	lcd.setColor(CUSTOM_COLOR, RED)

	lcd.drawLine(viewStart, 269, viewEnd, 269, SOLID, CUSTOM_COLOR)
	lcd.drawLine(viewStart, 270, viewEnd, 270, SOLID, CUSTOM_COLOR)
	lcd.drawLine(viewStart, 271, viewEnd, 271, SOLID, CUSTOM_COLOR)

	for varIndex = 2, 5, 1 do
		if dataSelection[varIndex].value ~= 0 then
			local points = points[varIndex - 1]
			local cfg = graphConfig[varIndex - 1]

			if #points.points == 0 then
				for i = 0, 100, 1 do
					
					--print("i:" .. i .. ", skip: " .. skip .. ", result:" .. math_floor(graphStart + (i * skip)))

					points.points[i] = values[varIndex - 1][math_floor(graphStart + (i * skip))]

					if points.points[i] == nil then
						points.points[i] = 0
					end
				end
			end

			lcd.setColor(CUSTOM_COLOR, cfg.color)
			drawGraph_points(points.points, points.min, points.max)

			local minPos = math_floor((points.minpos + 1 - graphStart) / skip)
			local maxPos = math_floor((points.maxpos + 1 - graphStart) / skip)

			if minPos < 0 then
				minPos = 0
			elseif minPos > 100 then
				minPos = 100
			end

			if maxPos < 0 then
				maxPos = 0
			elseif maxPos > 100 then
				maxPos = 100
			end

			local x = graphConfig.xstart + (minPos * xStep)
			lcd.drawLine(x, 240, x, 250, SOLID, CUSTOM_COLOR)

			local x = graphConfig.xstart + (maxPos * xStep)
			lcd.drawLine(x, 30, x, 40, SOLID, CUSTOM_COLOR)

			if graphMode == GRAPH_MINMAX and graphMinMaxIndex == (varIndex - 2) * 2 then
				lcd.drawText(cfg.maxx, cfg.maxy, points.max, TEXT_INVERTED_COLOR + INVERS)
			else
				lcd.drawText(cfg.maxx, cfg.maxy, points.max, CUSTOM_COLOR)
			end

			if graphMode == GRAPH_MINMAX and graphMinMaxIndex == ((varIndex - 2) * 2) + 1 then
				lcd.drawText(cfg.minx, cfg.miny, points.min, TEXT_INVERTED_COLOR + INVERS)
			else
				lcd.drawText(cfg.minx, cfg.miny, points.min, CUSTOM_COLOR)
			end

			if points.points[cursor] ~= nil then
				lcd.drawText(cfg.valx, cfg.valy, points.name .. " " .. points.points[cursor], CUSTOM_COLOR)
			end
		end
	end
end

local function drawProgress(y, total, current)
	local pct = current / total

	lcd.drawRectangle(5, y, 470, 16, TEXT_COLOR)
	lcd.drawFilledRectangle(6, y + 1, 468 * pct, 14, TEXT_INVERTED_BGCOLOR)
end

local function init()
	local name = string_gsub(model.getInfo().name, " ", "_");
	local now = getDateTime();

	local year = now.year
	local month = now.mon
	local day = now.day

	for i = 0, 100, 1 do
		local testFilename = name .. "-" .. year .. "-" .. doubleDigits(month) .. "-" .. doubleDigits(day) .. ".csv"

		local file = io.open("/LOGS/" .. testFilename, "r")

		if file ~= nil then
			fileSelection[1].values[#fileSelection[1].values + 1] = testFilename
			io.close(file)
		end

		day = day - 1
		if day == 0 then
			day = 31
			month = month - 1
		end
		if month == 0 then
			month = 12
			year = year - 1
		end
	end
end

local function run_SELECTFILE(event)
	if event == EVT_EXIT_BREAK then
		return 2
	elseif event == EVT_ENTER_BREAK or event == EVT_ROT_BREAK then
		filename = fileSelection[1].values[fileSelection[1].value]

		if fileSelection[2].value == 3 then
			local fileResult = initFile()

			if fileResult ~= nill then
				error(fileResult)
			else
				io.close(file);
				readIndex()

				for i=0,sessionCount - 1,1 do
					dataSelection[1].values[i + 1] = (i + 1)
				end

				dataSelection[1].value = sessionCount

				for varIndex = 2, 5, 1 do
					dataSelection[varIndex].values[0] = "---"
					for i=2,#columns,1 do
						dataSelection[varIndex].values[i - 1] = columns[i]
					end
				end

				current_option = 1
				step = STEP_SELECTSESSION
				return 0
			end
		else
			--Reset file load data
			buffer = ""
			sessions = {}
			sessionCount = 0
			lines = 0

			heap = 2048 * 12

			if fileSelection[2].value == 1 then
				skipLines = 1
			else
				skipLines = 5
			end

			prevTotalSeconds = 0

			local fileResult = initFile()

			if fileResult ~= nill then
				error(fileResult)
			else
				step = STEP_READFILE
				return 0
			end
		end
	end
		
	if #fileSelection[1].values == 0 then
		lcd.drawText(5, 40, "No recent log files found for the current model", TEXT_COLOR)
	else
		local currentSelection = fileSelection[1].value

		drawOptions(fileSelection)
		handleOptionInput(fileSelection, event)

		if currentSelection ~= fileSelection[1].value then
			local indexFileName = fileSelection[1].values[fileSelection[1].value] .. ".index"
			local file = io.open("/LOGS/" .. indexFileName, "r")

			if file ~= nil then
				io.close(file)
				fileSelection[2].max = 3
				fileSelection[2].value = 3
			else
				fileSelection[2].max = 2
				fileSelection[2].value = 1
			end

		end

		if fileSelection[2].value == 1 then
			lcd.drawText(100, 80, "Check every line for a session change", TEXT_COLOR)
		elseif fileSelection[2].value == 2 then
			lcd.drawText(100, 80, "Check every 5 lines for a session change", TEXT_COLOR)
			lcd.drawText(100, 110, "!You might loose 5 points in the beginning", TEXT_COLOR)
			lcd.drawText(100, 130, "or the end of the session", TEXT_COLOR)
		else
			lcd.drawText(100, 80, "!New sessions since the index was created will", TEXT_COLOR)
			lcd.drawText(100, 100, "not be available", TEXT_COLOR)
		end
	end

	return 0
end

local function run_READFILE(event)
	if event == EVT_EXIT_BREAK then
		filename = nil
		step = STEP_SELECTFILE
		return 0
	end

	lcd.drawText(5, 40, "Analysing file...", TEXT_COLOR)
	lcd.drawText(5, 60, "Found " .. lines .. " lines", TEXT_COLOR)
	lcd.drawText(5, 80, "Found " .. sessionCount .. " sessions", TEXT_COLOR)
	if readFile() then
		for i=0,sessionCount - 1,1 do
			dataSelection[1].values[i + 1] = (i + 1)
		end

		dataSelection[1].value = sessionCount

		for varIndex = 2, 5, 1 do
			dataSelection[varIndex].values[0] = "---"
			for i=2,#columns,1 do
				dataSelection[varIndex].values[i - 1] = columns[i]
			end
		end

		current_option = 1
		step = STEP_SELECTSESSION
		saveIndex()
	end

	return 0
end

local function run_SELECTSESSION(event)
	if event == EVT_EXIT_BREAK then
		filename = nil
		step = STEP_SELECTFILE
		return 0
	elseif event == EVT_ENTER_BREAK or event == EVT_ROT_BREAK then

		if dataSelection[6].value == 1 then
			skipLines = 1
			heap = 2048 * 4
		elseif dataSelection[6].value == 2 then
			skipLines = 2
			heap = 2048 * 8
		elseif dataSelection[6].value == 3 then
			skipLines = 5
			heap = 2048 * 16
		else
			skipLines = 10
			heap = 2048 * 16
		end

		buffer = ""
		file = io.open("/LOGS/" .. filename, "r")
		io.seek(file, session.startIndex)
		index = session.startIndex

		valPos = 0
		lines = 0
		maxLines = session.endLine - session.startLine
		points = {}
		values = {}
		
		for varIndex = 2, 5, 1 do
			if dataSelection[varIndex].value ~= 0 then
				points[varIndex - 1] = {}
				values[varIndex - 1] = {}
			end
		end

		step = STEP_READSESSION
		return 0
	end

	drawOptions(dataSelection)
	handleOptionInput(dataSelection, event)

	session = sessions[dataSelection[1].value - 1]
	duration = toDuration(getTotalSeconds(session.endTime) - getTotalSeconds(session.startTime))

	lcd.drawText(100, 60, "Started " .. session.startTime, TEXT_COLOR)
	lcd.drawText(100, 80, "Duration " .. duration, TEXT_COLOR)

		if dataSelection[6].value == 1 then
			lcd.drawText(100, 200, "Read a point from every line", TEXT_COLOR)
		elseif dataSelection[6].value == 2 then
			lcd.drawText(100, 200, "Read a point from every 2nd line", TEXT_COLOR)
			lcd.drawText(100, 230, "!Min / max values might not be found", TEXT_COLOR)
		elseif dataSelection[6].value == 3 then
			lcd.drawText(100, 200, "Read a point from every 5th line", TEXT_COLOR)
			lcd.drawText(100, 230, "!Min / max values might not be found", TEXT_COLOR)
		else
			lcd.drawText(100, 200, "Read a point from every 10th line", TEXT_COLOR)
			lcd.drawText(100, 230, "!Min / max values might not be found", TEXT_COLOR)
		end

	return 0
end

local function run_READSESSION(event)
	if event == EVT_EXIT_BREAK then
		step = STEP_SELECTSESSION
		return 0
	end

	lcd.drawText(5, 40, "Reading data from file...", TEXT_COLOR)
	lcd.drawText(5, 60, "Reading line " .. lines, TEXT_COLOR)
		
	drawProgress(85, maxLines, lines)

	if collectData(file) then
		conversionStep = 0
		step = STEP_PARSEDATA
	end
		
	return 0
end

local function run_PARSEDATA(event)
	if event == EVT_EXIT_BREAK then
		step = STEP_SELECTSESSION
		return 0
	end

	lcd.drawText(5, 40, "Parsing data", TEXT_COLOR)
	lcd.drawText(5, 60, "Almost ready :)", TEXT_COLOR)


	if conversionStep ~= 0 then
		lcd.drawText(5, 80, "Parsing Var " .. conversionStep, TEXT_COLOR)
		drawProgress(105, valPos, conversionIndex)
	end

	local cnt = 0
	if conversionStep == 0 then
		conversionStep = 1
		conversionIndex = 0

		session = sessions[dataSelection[1].value - 1]
		graphTimeBase = valPos / (getTotalSeconds(session.endTime) - getTotalSeconds(session.startTime))

		for varIndex = 2, 5, 1 do
			if dataSelection[varIndex].value ~= 0 then
				local columnName = columns[dataSelection[varIndex].value + 1]
				local i = string.find(columnName, "%(")

				if i ~= nil then
					columnName = string.sub(columnName, 0, i - 1)
				end

				points[varIndex - 1] = { min = 9999, max = -9999, minpos = 0, maxpos = 0, points = {}, name = columnName }
			end
		end
	else
		if dataSelection[conversionStep + 1].value ~= 0 then
			for i = conversionIndex, valPos - 1, 1 do
				
				val = tonumber(values[conversionStep][i])
				values[conversionStep][i] = val
				conversionIndex = conversionIndex + 1
				cnt = cnt + 1

				if val > points[conversionStep].max then
					points[conversionStep].max = val
					points[conversionStep].maxpos = i
				elseif val < points[conversionStep].min then
					points[conversionStep].min = val
					points[conversionStep].minpos = i
				end

				if cnt > 100 then
					return 0
				end
			end
		end

		if conversionStep == 4 then
			graphStart = 0
			graphSize = valPos
			cursor = 0
			graphMode = GRAPH_CURSOR
			step = STEP_GRAPH
		else
			conversionIndex = 0
			conversionStep = conversionStep + 1
		end
	end

	return 0
end

local function run_GRAPH_Adjust(amount, mode)
	if mode == GRAPH_CURSOR then
		cursor = cursor + math.floor(amount)
		if cursor > 100 then
			cursor = 100
		elseif cursor < 0 then
			cursor = 0
		end
	elseif mode == GRAPH_ZOOM then
		if amount > 4 then
			amount = 4
		elseif amount < -4 then
			amount = -4
		end

		local oldgraphSize = graphSize
		graphSize = math.floor(graphSize / (1 + (amount * 0.2)))

		if graphSize < 101 then
			graphSize = 101
		elseif graphSize > valPos then
			graphSize = valPos
		end

		if graphSize > (valPos - graphStart) then
			if amount > 0  then
				graphSize = valPos - graphStart
			else
				graphStart = valPos - graphSize
			end
		else
			local delta = oldgraphSize - graphSize
			graphStart = graphStart + math_floor((delta * (cursor / 100)))

			if graphStart < 0 then
				graphStart = 0
			elseif graphStart + graphSize > valPos then
				graphStart = valPos - graphSize
			end
		end

		graphSize = math_floor(graphSize)

		for varIndex = 2, 5, 1 do
			if dataSelection[varIndex].value ~= 0 then
				points[varIndex - 1].points = {}
			end
		end
	elseif mode == GRAPH_MINMAX then
		local point = points[(math.floor(graphMinMaxIndex / 2)) + 1]

		local delta = math.floor((point.max - point.min) / 50 * amount)

		if amount > 0 and delta < 1 then
			delta = 1
		elseif amount < 0 and delta > -1 then
			delta = -1
		end

		if graphMinMaxIndex % 2 == 0 then
			point.max = point.max + delta

			if point.max < point.min then
				point.max = point.min + 1
			end
		else
			point.min = point.min + delta

			if point.min > point.max then
				point.min = point.max - 1
			end
		end
	elseif mode == GRAPH_SCROLL then
		graphStart = graphStart + math.floor(((graphSize / 10) * amount))

		if graphStart + graphSize > valPos then
			graphStart = valPos - graphSize
		elseif graphStart < 0 then
			graphStart = 0
		end

		graphStart = math_floor(graphStart)

		for varIndex = 2, 5, 1 do
			if dataSelection[varIndex].value ~= 0 then
				points[varIndex - 1].points = {}
			end
		end
	end
end

local function run_GRAPH(event)
	if event == EVT_EXIT_BREAK then
		step = STEP_SELECTSESSION
		return 0
	elseif graphMode == GRAPH_MINMAX and event == EVT_PAGEDN_FIRST then
		graphMinMaxIndex = graphMinMaxIndex + 1

		if graphMinMaxIndex == 8 then
			graphMinMaxIndex = 0
		end

		if graphMinMaxIndex == 2 and dataSelection[3].value == 0 then
			graphMinMaxIndex = 4
		end

		if graphMinMaxIndex == 4 and dataSelection[4].value == 0 then
			graphMinMaxIndex = 6
		end

		if graphMinMaxIndex == 6 and dataSelection[5].value == 0 then
			graphMinMaxIndex = 0
		end

		if graphMinMaxIndex == 0 and dataSelection[2].value == 0 then
			graphMinMaxIndex = 2
		end
	elseif graphMode == GRAPH_MINMAX and event == EVT_PAGEUP_FIRST then
		graphMinMaxIndex = graphMinMaxIndex - 1

		if graphMinMaxIndex < 0 then
			graphMinMaxIndex = 7
		end

		if graphMinMaxIndex == 7 and dataSelection[5].value == 0 then
			graphMinMaxIndex = 5
		end

		if graphMinMaxIndex == 5 and dataSelection[4].value == 0 then
			graphMinMaxIndex = 3
		end

		if graphMinMaxIndex == 3 and dataSelection[3].value == 0 then
			graphMinMaxIndex = 1
		end

		if graphMinMaxIndex == 1 and dataSelection[2].value == 0 then
			graphMinMaxIndex = 7
		end
	elseif event == EVT_ENTER_BREAK or event == EVT_ROT_BREAK then
		if graphMode == GRAPH_CURSOR then
			graphMode = GRAPH_ZOOM
		elseif graphMode == GRAPH_ZOOM then
			graphMode = GRAPH_SCROLL
		elseif graphMode == GRAPH_SCROLL then
			graphMode = GRAPH_MINMAX
		else
			graphMode = GRAPH_CURSOR
		end
	elseif event == EVT_PLUS_FIRST or event == EVT_ROT_RIGHT or event == EVT_PLUS_REPT then
		run_GRAPH_Adjust(1, graphMode)
	elseif event == EVT_MINUS_FIRST or event == EVT_ROT_LEFT or event == EVT_MINUS_REPT then
		run_GRAPH_Adjust(-1, graphMode)
	end

	local adjust = getValue('ail') / 200

	if math.abs(adjust) > 0.5 then
		if graphMode == GRAPH_MINMAX then
			run_GRAPH_Adjust(adjust, GRAPH_MINMAX)
		else
			run_GRAPH_Adjust(adjust, GRAPH_CURSOR)
		end
	end

	adjust = getValue('ele') / 200

	if math.abs(adjust) > 0.5 then
		run_GRAPH_Adjust(adjust, GRAPH_ZOOM)
	end

	adjust = getValue('rud') / 200

	if math.abs(adjust) > 0.5 then
		run_GRAPH_Adjust(adjust, GRAPH_SCROLL)
	end

	drawGraph()
	return 0
end

local function run(event)
	if event == nil then
		error("Cannot be run as a model script!")
		return 2
	end

	drawMain()

	if step == STEP_SELECTFILE then
		return run_SELECTFILE(event)
	elseif step == STEP_READFILE then
		return run_READFILE(event)
	elseif step == STEP_SELECTSESSION then
		return run_SELECTSESSION(event)
	elseif step == STEP_READSESSION then
		return run_READSESSION(event)
	elseif step == STEP_PARSEDATA then
		return run_PARSEDATA(event)
	elseif step == STEP_GRAPH then
		return run_GRAPH(event)
	end

	--impossible step??
	error("Something went wrong with the script!")
	return 2
end

return { init=init, run=run }
