local ecudisplay = {}
local cmd = 0x10
local back

local function init()
  	for i=0,31,1 do
		ecudisplay[i] = 0
	end

	back = Bitmap.open("img/back.png")
	cmd = 0x10
end

local function run(event)
	if event == nil then
		error("Cannot be run as a model script!")
		return 2
	elseif event == EVT_EXIT_BREAK then
		cmd = 0x11
	elseif event == EVT_MODEL_FIRST then
		cmd = 0x10
	elseif event == EVT_ENTER_BREAK or event == EVT_SYS_FIRST then
		cmd = 0x21
	elseif event == EVT_ENTER_LONG or event == EVT_TELEM_FIRST then
		cmd = 0x22
	elseif event == EVT_PAGE_BREAK or event == EVT_PAGEDN_FIRST  then
		cmd = 0x23
	elseif event == EVT_PAGE_LONG or event == EVT_PAGEUP_FIRST then
		cmd = 0x24
	end

	local physicalId, primId, dataId, value = sportTelemetryPop()  
	
	while physicalId ~= nil do
		if primId ~= 0x32 then
			pos = dataId - 0x5000
			b4 = math.floor(value / (256 ^ 3))
			value = value - b4 * (256 ^ 3)
			b3 = math.floor(value / (256 ^ 2))
			value = value - b3 * (256 ^ 2)
			b2 = math.floor(value / 256)
			value = value - b2 * 256
			b1 = math.floor(value % 256)

			ecudisplay[pos * 4] = b1
			ecudisplay[(pos * 4) + 1] = b2
			ecudisplay[(pos * 4) + 2] = b3
			ecudisplay[(pos * 4) + 3] = b4
		end

		physicalId, primId, dataId, value = sportTelemetryPop()  
	end
	
	line1 = ""
	line2 = ""

	for i=0,15,1 do
		line1 = line1 .. string.char(ecudisplay[i])
	end
	
	for i=16,31,1 do
		line2 = line2 .. string.char(ecudisplay[i])
	end

	line1 = string.gsub(line1, "ß", "@")
	line2 = string.gsub(line2, "ß", "@")

	lcd.clear()
	lcd.drawBitmap(back, 0, 0)

	lcd.drawText(130, 85, line1, BLACK + DBLSIZE)
	lcd.drawText(130, 125, line2, BLACK + DBLSIZE)


	if cmd ~= 0 then
		lcd.drawText(130, 25, "Cmd: " .. cmd, BLACK + DBLSIZE)
		if sportTelemetryPush(0x1B, 0x31, 0x5000, cmd) then
			if cmd == 0x11 then
				cmd = 0x10
				return 2
			else
				cmd = 0
			end
		end
	end

	return 0
end

return { init=init, run=run }
