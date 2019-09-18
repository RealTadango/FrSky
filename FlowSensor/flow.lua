local pulses = 533
local tanksize = 1000
local selected = 0

local function init()
	--request config data from sensor
  	sportTelemetryPush(0x1B, 0x31, 0x5290, 0x0)
end

local function run(event)
	if event == nil then
		error("Cannot be run as a model script!")
		return 2
	elseif event == EVT_EXIT_BREAK then
		return 2
	elseif event == EVT_MODEL_FIRST then
		--todo
	elseif event == EVT_ENTER_BREAK or event == EVT_SYS_FIRST then
		--todo
	elseif event == EVT_ENTER_LONG or event == EVT_TELEM_FIRST then
		if selected == 0 then
			sportTelemetryPush(0x1B, 0x32, 0x5290, pulses)
		elseif selected == 1 then
			sportTelemetryPush(0x1B, 0x32, 0x5291, tanksize)
		end
	elseif event == EVT_PAGE_BREAK or event == EVT_PAGEDN_FIRST  then
		selected = 1
	elseif event == EVT_PAGE_LONG or event == EVT_PAGEUP_FIRST then
		selected = 0
	elseif event == EVT_PLUS_FIRST or event == EVT_ROT_RIGHT or event == EVT_PLUS_REPT then
      if selected == 0 then
		pulses = pulses + 1
	  elseif selected == 1 then
		tanksize = tanksize + 100
	  end
    elseif event == EVT_MINUS_FIRST or event == EVT_ROT_LEFT or event == EVT_MINUS_REPT then
      if selected == 0 then
		pulses = pulses - 1
	  elseif selected == 1 then
		tanksize = tanksize - 100
	  end
    end

	local physicalId, primId, dataId, value = sportTelemetryPop()  
	
	while physicalId ~= nil do
		if dataId == 0x5000 then
			pulses = value
		elseif dataId == 0x5001 then
			tanksize = value
		end

		physicalId, primId, dataId, value = sportTelemetryPop()  
	end
	
	lcd.clear()
	lcd.drawFilledRectangle(0, 0, LCD_W, LCD_H, TEXT_BGCOLOR)
	lcd.drawFilledRectangle(0, 0, LCD_W, 30, TITLE_BGCOLOR)
	lcd.drawText(1, 5, "FLOW sensor Config", MENU_TITLE_COLOR)

	lcd.drawText(1, 30, "Pulses ", TEXT_COLOR)
	if selected == 0 then
		lcd.drawNumber(100, 30, pulses, PREC2 + LEFT + TEXT_COLOR + INVERS)
	else
		lcd.drawNumber(100, 30, pulses, PREC2 + LEFT + TEXT_COLOR)
	end
	lcd.drawText(140, 30, "pulses / ml ", TEXT_COLOR)
	
	lcd.drawText(1, 55, "Tanksize ", TEXT_COLOR)

	if selected == 1 then
		lcd.drawNumber(100, 55, tanksize, LEFT + TEXT_COLOR + INVERS)
	else
		lcd.drawNumber(100, 55, tanksize, LEFT + TEXT_COLOR)
	end
	lcd.drawText(140, 55, "ml ", TEXT_COLOR)

	lcd.drawText(1, 90, "Use TELE to update the selected value ", TEXT_COLOR)
	
	return 0
end

return { init=init, run=run }
