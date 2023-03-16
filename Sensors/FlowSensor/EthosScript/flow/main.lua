local fields = {}
local sensor = nil
local index = 1

local function read(app)
	print("reading from " .. app)
	--sensor:requestParameter(app) TODO Figure out how this works sensor side
	sensor:pushFrame({physId=0x1B, primId=0x30, appId=app})
end

local function write(app, value)
	print("writing " .. value .. " to " .. app)
	--sensor:writeParameter(app, value) TODO Figure out how this works sensor side
	sensor:pushFrame({physId=0x1B, primId=0x31, appId=app, value=value})
end

local function getValue(parameter)
	if parameter[4] == nil then
	  	return 0
	else
	  	return parameter[4]
	end
end

local function setValue(parameter, value)
	parameter[4] = value
	write(parameter[7], value)
 end

local function createNumberField(line, parameter)
	local field = form.addNumberField(line, nil, parameter[5], parameter[6], function() return getValue(parameter) end, function(value) setValue(parameter, value) end)
	field:enableInstantChange(false)
	if #parameter == 8 then
		field:suffix(parameter[8])
	end
	field:enable(false)
	return field
end

local parameters = {
	-- { name, type, default, value, min, max, remoteid, suffix }
	{"Pulses", createNumberField, 0, nil, 0, 2000, 0x5200, " (/ 100) per ml" },
	{"Tank size", createNumberField, 0, nil, 0, 10000, 0x5201, "ml" }
}

local function updateField(index, value)
	parameters[index][4] = value
	if parameters[index][4] ~= nil then
		fields[index]:enable(true)
	end
end

local function wakeup(widget)
	local frame = sensor:popFrame()

	if frame ~= nil then
		local value = frame:value()
		local prim = frame:primId()
		local app = frame:appId()

		if prim == 0x32 and value ~= nil then
			updateField(index, value)
			index = index + 1

			if index <= #parameters then
				read(parameters[index][7])
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
	print("Starting...")
	fields = {}
	sensor = nil
	index = 1

	sensor = sport.getSensor({appIdStart=0x5000, appIdEnd=0x5201})

	for index = 1, #parameters do
		local parameter = parameters[index]
		local line = form.addLine(parameter[1])
		local field = parameter[2](line, parameter)
		fields[#fields + 1] = field
	end

	--Read first parameter
	read(parameters[index][7])
	return
end

local icon = lcd.loadMask("flow.png")

local function init()
  system.registerSystemTool({name="FLOW config", icon=icon, create=create, wakeup=wakeup, event=event})
end

return {init=init}

