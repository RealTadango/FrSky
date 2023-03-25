local sensor
local index
local myForms = dofile("../lib/myForms.lua")

local parameters = {
	-- { name, type, default, value, min, max, suffix, remoteid }
	{"Pulses", myForms.createNumberField, 570, nil, 0, 2000, " per 100ml", 0x5200 },
	{"Tank size", myForms.createNumberField, 3600, nil, 0, 10000, "ml", 0x5201 }
}

local function read(app)
	sensor:pushFrame({physId=0x1B, primId=0x30, appId=app})
end

local function write(app, value)
	sensor:pushFrame({physId=0x1B, primId=0x31, appId=app, value=value})
end

local function parameterChanged(parameter)
	write(parameter[8], parameter[4])
end

local function wakeup(widget)
	frame = sensor:popFrame()

	if frame ~= nil then
		value = frame:value()
		prim = frame:primId()
		app = frame:appId()

		if prim == 0x32 and value ~= nil then -- received value response
			myForms.updateField(index, value)
			index = index + 1

			if index <= #parameters then
				read(parameters[index][8])
			end
		end
	end
end

local function event(widget, category, value, x, y)
	if category == EVT_KEY and value == KEY_EXIT_BREAK then
		return false
	else
		return true
	end
end

local function create()
	myForms.load(parameters, parameterChanged)

	index = 1
	sensor = sport.getSensor({appIdStart=0x5000, appIdEnd=0x5201})
	read(parameters[index][8])
end

local icon = lcd.loadMask("flow.png")

local function init()
  system.registerSystemTool({name="FLOW config", icon=icon, create=create, wakeup=wakeup, event=event})
end

return {init=init}

