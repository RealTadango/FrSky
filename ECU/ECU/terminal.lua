local ecudisplay = {}
local cmd = 0x10
local delay = 0
local back

local function init()
  	for i=0,31,1 do
		ecudisplay[i] = 0
	end

	back = Bitmap.open("back.png")
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
		if primId == 0x32 then
			delay = 0
			if cmd == 0x11 then
				cmd = 0x10
				setup = true
				return 2
			end
			cmd = 0
		else
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

	lcd.clear()
	lcd.drawBitmap(back, 0, 0)

	lcd.drawText(130, 85, line1, BLACK + DBLSIZE)
	lcd.drawText(130, 125, line2, BLACK + DBLSIZE)
	
	--lcd.drawText(130, 85, line1, WHITE)
	--lcd.drawText(1, 25, line2, MENU_TITLE_COLOR)
	--lcd.drawText(1, 45, "Cmd: " .. cmd, MENU_TITLE_COLOR)

	if cmd ~= 0 then
		lcd.drawText(130, 25, "Cmd: " .. cmd, BLACK + DBLSIZE)
		if delay == 0 then
			if sportTelemetryPush(0x1B, 0x31, 0x5000, cmd) then
				delay = 25
			end
		end
	end
	
	if delay ~= 0 then
		delay = delay - 1
	end

	return 0
end

return { init=init, run=run }
