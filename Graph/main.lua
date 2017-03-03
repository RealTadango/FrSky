local options = {
	{ "Source", SOURCE, 1 },
	{ "Background", COLOR, GREY },
	{ "Interval", VALUE, 8, 1, 35 }, 
	{ "Min", VALUE, -100, -1024, 1024 },
	{ "Max", VALUE, 100, -1024, 1024 }
}

local _maxPoints = 100

function create(zone, options)
	local context = { zone=zone, options=options, points={}, lastTime=0, index=0 }
	return context
end

function update(context, options)
	context.options = options
	context.index = 0
end

function refresh(context)
	updatePoints(context)
	drawGrap(context)
end

function background(context)
	updatePoints(context)
end

function updatePoints(context)
	time = getTime()

	delay = context.options.Interval * context.options.Interval 
	
	if time > context.lastTime + delay then
		value = getValue(context.options.Source)

		if(value == nil) then
			return 0
		end

		context.lastTime = time

		if(context.index < _maxPoints) then
			context.points[context.index] = value
			context.index = context.index + 1
		else
			for i=1,_maxPoints - 1,1 do
				context.points[i - 1] = context.points[i]
			end
			context.points[_maxPoints - 1] = value
		end
	end
end

function drawGrap(context)
	x = context.zone.x
	y = context.zone.y
	width = context.zone.w
	height = context.zone.h

	lcd.setColor(CUSTOM_COLOR, context.options.Background)
	lcd.drawFilledRectangle(x, y, width, height, CUSTOM_COLOR)

	lcd.drawLine(x, y, x, y + height, SOLID, TEXT_BGCOLOR)
	lcd.drawLine(x, y + height, x + width, y + height, SOLID, TEXT_BGCOLOR)

	if context.index == 0 then
		return
	end

	dataMin = 9999
	dataMax = -9999

	for i=0,context.index - 1,1 do
		if( context.points[i] > dataMax) then
			dataMax = context.points[i]
		end
	end

	for i=0,context.index - 1,1 do
		if( context.points[i] < dataMin) then
			dataMin = context.points[i]
		end
	end

	maxValue = context.options.Max
	minValue = context.options.Min

	if context.zone.w > 100 then
		if(dataMin < minValue) then minValue = dataMin end
		if(dataMax > maxValue) then maxValue = dataMax end
	end

	zeroPos = height
	range = maxValue - minValue
	
	zeroPos = height - (height / (range / (range + minValue)))

	if minValue < 0 and maxValue > 0 then
		lcd.drawLine(x, y + height - zeroPos, x + width, y + height - zeroPos, DOTTED, TEXT_BGCOLOR)
	end

	if range == 0 then
		range = 1
	end
	
	previous_xPos = x
	previous_yPos = y + height
	
	for i=0,context.index - 1,1 do

		value = context.points[i]

		if value > maxValue then value = maxValue end
		if value < minValue then value = minValue end

		xPos = x + (i * (context.zone.w / _maxPoints))
		yPos = y + (maxValue - value) * (height / range)

		lcd.drawLine(previous_xPos, previous_yPos, xPos, yPos, SOLID, TEXT_BGCOLOR)
		
		previous_xPos = xPos
		previous_yPos = yPos
	end
	
	if context.zone.w > 100 then
		if range >= 100 then
			lcd.drawNumber(x + width, y + 1, value, TEXT_COLOR + RIGHT)
			lcd.drawNumber(x + 3, y + 1, maxValue, TEXT_COLOR)
			lcd.drawNumber(x + 3, y + height - 17, minValue, TEXT_COLOR)

			lcd.drawNumber(x + width - 1, y, value, TEXT_BGCOLOR + RIGHT)
			lcd.drawNumber(x + 2, y, maxValue, TEXT_BGCOLOR)
			lcd.drawNumber(x + 2, y + height - 18, minValue, TEXT_BGCOLOR)
		else 
			if range >= 10 then
				lcd.drawNumber(x + width, y + 1, value * 10, TEXT_COLOR + RIGHT + PREC1)
				lcd.drawNumber(x + 3, y + 1, maxValue * 10, TEXT_COLOR + PREC1)
				lcd.drawNumber(x + 3, y + height - 17, minValue * 10, TEXT_COLOR + PREC1)

				lcd.drawNumber(x + width - 1, y, value * 10, TEXT_BGCOLOR + RIGHT + PREC1)
				lcd.drawNumber(x + 2, y, maxValue * 10, TEXT_BGCOLOR + PREC1)
				lcd.drawNumber(x + 2, y + height - 18, minValue * 10, TEXT_BGCOLOR + PREC1)
			else
				lcd.drawNumber(x + width, y + 1, value * 100, TEXT_COLOR + RIGHT + PREC2)
				lcd.drawNumber(x + 3, y + 1, maxValue * 100, TEXT_COLOR + PREC2)
				lcd.drawNumber(x + 3, y + height - 17, minValue * 100, TEXT_COLOR + PREC2)

				lcd.drawNumber(x + width - 1, y, value * 100, TEXT_BGCOLOR + RIGHT + PREC2)
				lcd.drawNumber(x + 2, y, maxValue * 100, TEXT_BGCOLOR + PREC2)
				lcd.drawNumber(x + 2, y + height - 18, minValue * 100, TEXT_BGCOLOR + PREC2)
			end
		end

		lcd.drawSource(x + (width / 2) - 19, y + 1, context.options.Source, TEXT_COLOR)
		lcd.drawSource(x + (width / 2) - 20, y, context.options.Source, TEXT_BGCOLOR)
	end
end

return { name="Graph", options=options, create=create, update=update, refresh=refresh, background=background }
