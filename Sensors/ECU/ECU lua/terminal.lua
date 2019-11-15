local ecudisplay = {}
local cmd = 0x10
local back
local type = -1
local newType = 0

local function sendCmd(cmd)
	sportTelemetryPush(0x1B, 0x31, 0x5000, cmd)
end

local function loadWaitBack()
	back = Bitmap.open("img/back.png")
end

local function loadBrandBack()
	if type > 0 then
		back = Bitmap.open("img/back" .. type .. ".png")
	end
end

local function init()
  	for i=0,31,1 do
		ecudisplay[i] = '#'
	end

	loadWaitBack()
	loadBrandBack()
	sendCmd(0x10)
end

local function fixedChar(newChar)
	newChar = string.char(newChar)
	if newChar == 'ß' then
		newChar = '@'
	end
	return newChar	
end

local function run(event)
	if event == nil then
		error("Cannot be run as a model script!")
		return 2
	elseif event == EVT_EXIT_BREAK then
		if newType > 0 then
			newType = 0
			loadBrandBack();
		else
			sendCmd(0x11)
			return 2
		end
	elseif event == EVT_MODEL_FIRST then
		sendCmd(0x10)
	elseif event == EVT_ENTER_BREAK then
		sendCmd(0x21)
	elseif event == EVT_ENTER_LONG then
		sendCmd(0x22)
	elseif event == EVT_PAGEDN_FIRST or event == EVT_ROT_LEFT  then
		sendCmd(0x23)
	elseif event == EVT_PAGEUP_FIRST or event == EVT_ROT_RIGHT then
		sendCmd(0x24)
	elseif event == EVT_SYS_FIRST then
		if newType == 1 then
			newType = 2
		else 
			newType = 1
		end
		loadWaitBack();
	elseif event == EVT_TELEM_FIRST and newType > 0 then
		sendCmd(0x50 + newType)
		type = newType
		newType = -1
		loadBrandBack()
	end

	if type == -1 then
		sportTelemetryPush(0x1B, 0x30, 0x5000, 0x50)
	end

	local physicalId, primId, dataId, value = sportTelemetryPop()  
	
	while physicalId ~= nil do
		if primId == 0x10 then
			pos = dataId - 0x5000
			b4 = math.floor(value / (256 ^ 3))
			value = value - b4 * (256 ^ 3)
			b3 = math.floor(value / (256 ^ 2))
			value = value - b3 * (256 ^ 2)
			b2 = math.floor(value / 256)
			value = value - b2 * 256
			b1 = math.floor(value % 256)

			ecudisplay[pos * 4] = fixedChar(b1)
			ecudisplay[(pos * 4) + 1] = fixedChar(b2)
			ecudisplay[(pos * 4) + 2] = fixedChar(b3)
			ecudisplay[(pos * 4) + 3] = fixedChar(b4)
		elseif primId == 0x32 and dataId == 0x5050 then
			type = value
			if type == 0 then
				loadWait()
				newType = 1
			else
				loadBrandBack()
			end
		end

		physicalId, primId, dataId, value = sportTelemetryPop()  
	end
	
	lcd.clear()
	lcd.drawBitmap(back, 0, 0)
	
	if newType > 0 then
		lcd.setColor(CUSTOM_COLOR, WHITE)
		lcd.drawText(67, 85, "Set ECU type (TELE)", CUSTOM_COLOR + DBLSIZE)
		lcd.setColor(CUSTOM_COLOR, BLACK)
		lcd.drawText(70, 88, "Set ECU type (TELE)", CUSTOM_COLOR + DBLSIZE)
		
		typeName = "??"
		
		if newType == 1 then
			typeName = "Jetronic"
		elseif newType == 2 then
			typeName = "Fadec"
		end

		lcd.setColor(CUSTOM_COLOR, WHITE)
		lcd.drawText(67, 128, typeName, CUSTOM_COLOR + DBLSIZE)
		lcd.setColor(CUSTOM_COLOR, BLACK)
		lcd.drawText(70, 125, typeName, CUSTOM_COLOR + DBLSIZE)
	elseif type > 0 then
		y1 = 85
		y2 = 125
		x = 130
		
		if type == 2 then
			x = 88
			y1 = 75
			y2 = 110
		end
		
		for i=0,15,1 do
			lcd.drawText(x + (i * 19), y1, ecudisplay[i], BLACK + DBLSIZE)
			lcd.drawText(x + (i * 19), y2, ecudisplay[i + 16], BLACK + DBLSIZE)
		end
	else
		lcd.setColor(CUSTOM_COLOR, WHITE)
		lcd.drawText(67, 103, "Waiting for terminal...", CUSTOM_COLOR + DBLSIZE)
		lcd.setColor(CUSTOM_COLOR, BLACK)
		lcd.drawText(70, 100, "Waiting for terminal...", CUSTOM_COLOR + DBLSIZE)
	end

	return 0
end

return { init=init, run=run }
