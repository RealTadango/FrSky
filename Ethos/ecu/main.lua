local sensor
local myForms = dofile("../lib/myForms.lua")
local ecudisplay = {}
local showTerminal = true
local back

local parameters = {
	-- { name, type, default, value, min, max, suffix }
	{ "ECU type", myForms.createChoiceField, 0, nil, {{"Projet", 1}, {"Fadec", 2}} }
}

local function send(value)
	sensor:pushFrame({physId=0x1B, primId=0x31, appId=0x5000, value=value})
end

local function parameterChanged(parameter)
	value = parameter[4]
	id = parameter[8]

	if id == 0x5050 then
		value = 0x50 + value
	end

	send(id, value)
end

local function create()
	showTerminal = true
	sensor = sport.getSensor({appIdStart=0x5000, appIdEnd=0x51FF})
	send(0x10)
	return
end


local function fixedChar(newChar)
	newChar = newChar % 0xFF

	if newChar == 223 then
		newChar = '°'
	elseif newChar == 228 then
		newChar = 'u'
	else
		newChar = string.char(newChar)
	end

	return newChar	
end

local function wakeup(widget)
	frame = sensor:popFrame()

	if frame ~= nil then
		value = frame:value()
		prim = frame:primId()
		app = frame:appId()

		if prim == 0x32 and app == 0x5050 and value ~= nil then -- received value response
		 	parameters[1][4] = value
			back = nil
			ecudisplay = {}
			lcd.invalidate()
		elseif prim == 0x10 then

			--print("app:" .. app)
			--print("value:" .. value)

			pos = app - 0x5000
			b4 = math.floor(value / (256 ^ 3))
			value = value - b4 * (256 ^ 3)
			b3 = math.floor(value / (256 ^ 2))
			value = value - b3 * (256 ^ 2)
			b2 = math.floor(value / 256)
			value = value - b2 * 256
			b1 = math.floor(value % 256)

			ecudisplay[(pos * 4) + 1] = fixedChar(b1)
			ecudisplay[(pos * 4) + 2] = fixedChar(b2)
			ecudisplay[(pos * 4) + 3] = fixedChar(b3)
			ecudisplay[(pos * 4) + 4] = fixedChar(b4)

			if parameters[1][4] == nil then --Fallback to Jetrronic for old type compatibility
				parameters[1][4] = 1
				back = nil
			end

			lcd.invalidate()
		end
	end
end

local function event(widget, category, value, x, y)
  	--print("Event received:", category, value, x, y, KEY_RIGHT_FIRST)

	if category == EVT_KEY and value == KEY_EXIT_LONG then
		if showTerminal then
			send(0x11)
			return false
		else
			showTerminal = true
			myForms.clear()
		end
	elseif category == EVT_KEY and value == KEY_MODEL_BREAK then
		send(0x10) -- reset
	elseif category == EVT_KEY and value == KEY_ENTER_FIRST then
		send(0x22) -- enter
	elseif category == EVT_KEY and value == KEY_EXIT_BREAK then
		send(0x21) -- long enter / return
	elseif category == EVT_KEY and value == KEY_SYSTEM_BREAK then
		send(0x24) -- down
	elseif category == EVT_KEY and value == KEY_DISP_BREAK then
		send(0x23) -- up
	elseif category == EVT_KEY and value == KEY_MODEL_LONG then
		showTerminal = false
		myForms.load(parameters, parameterChanged)
		lcd.invalidate()
	end

	return true
end

local function loadBack()
	type = parameters[1][4]

	if type == nil then
		back = lcd.loadBitmap("back_ecu.png")
	else
		print("LoadBack: Type = " .. type)
		back = lcd.loadBitmap("back" .. type ..".png")
	end
end

local function paint()
	if not showTerminal then return end

	type = parameters[1][4]
	w, h = lcd.getWindowSize()

	if back == nill then loadBack() end

	lcd.drawBitmap(0,0,back,w,h)

	if type == nil then
		lcd.color(lcd.RGB(0,0,0))
		lcd.font(FONT_XXL)
		lcd.drawText(w/2,h * 0.42,"Loading....", CENTERED)
	else
		x = w / 3.32
		y = h / 3.12
		y2 = h / 2.13

		lcd.color(lcd.RGB(0,0,0))
		lcd.font(FONT_XXL)

		for i=0,15,1 do
			local c1 = ecudisplay[i+1]
			local c2 = ecudisplay[i+17]

			if c1 == nil then c1 = '#' end
			if c2 == nil then c2 = '#' end

			lcd.drawText(x + (i * w / 27), y, c1, CENTERED)
			lcd.drawText(x + (i * w / 27), y2, c2, CENTERED)
		end
	end
end

local icon = lcd.loadMask("turbine.png")

local function init()
  	system.registerSystemTool({name="ECU config", icon=icon, create=create, wakeup=wakeup, event=event, paint=paint})
end

return {init=init}

