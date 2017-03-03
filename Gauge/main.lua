-- Zone sizes WxH(wo menu / w menu):
-- 2x4 = 160x32
-- 2x2 = 225x122/98
-- 2x1 = 225x252/207
-- 2+1 = 192x152 & 180x70
-- 1x1 = 460/390x252/217/172
--Heights: 32,70,98,122,172,207,217,252

local options = {
	{ "Source", SOURCE, 1 },
	{ "Min", VALUE, 0, -1024, 1024 },
	{ "Max", VALUE, 100, -1024, 1024 }
}

function create(zone, options)
	local context = { zone=zone, options=options }
	return context
end

function drawGauge(context)
	
	if context.back == nil then
		imageFile = "/WIDGETS/Gauge/g" .. context.zone.h .. ".png"
		context.back = Bitmap.open(imageFile)
	end

	lcd.drawBitmap(context.back, context.zone.x, context.zone.y)

	--value = 550
	value = getValue(context.options.Source)

	if(value == nil) then
		return
	end

	--Value from source in percentage
	percentageValue = value - context.options.Min;
	percentageValue = (percentageValue / (context.options.Max - context.options.Min)) * 100

	if percentageValue > 100 then
		percentageValue = 100
	elseif percentageValue < 0 then
		percentageValue = 0
	end

	--min = 5.54
	--max = 0.8
	degrees = 5.51 - (percentageValue / (100 / 4.74));

	x1 = math.floor(context.zone.x + (context.zone.h/2))
	y1 = math.floor(context.zone.y + (context.zone.h/2))
	x2 = math.floor(context.zone.x + (context.zone.h/2) + (math.sin(degrees) * (context.zone.h/2.3)))
	y2 = math.floor(context.zone.y + (context.zone.h/2) + (math.cos(degrees) * (context.zone.h/2.3)))

	--Work arround RC8 horizontal line bug
	if y1 == y2 and x2 < x1 then
		xt = x1
		yt = y1
		x1 = x2
		y1 = y2
		x2 = xt
		y2 = yt
	end

	print("X1: " .. x1)
	print("Y1: " .. y1)
	print("X2: " .. x2)
	print("Y2: " .. y2)

	lcd.setColor(CUSTOM_COLOR, lcd.RGB(255,255,255))
	lcd.drawLine(x1, y1, x2, y2, SOLID, CUSTOM_COLOR)

	flags1 = RIGHT + TEXT_COLOR
	flags2 = RIGHT + TEXT_BGCOLOR

	if context.zone.w < 100 then
		flags1 = flags1 + SMLSIZE
		flags2 = flags2 + SMLSIZE
	end

	if context.zone.w > context.zone.h * 1.6 or context.zone.h > 100 then
		lcd.drawSource(context.zone.x + context.zone.w, context.zone.y + 1, context.options.Source, flags1)
		lcd.drawSource(context.zone.x + context.zone.w - 1, context.zone.y, context.options.Source, flags2)
		lcd.drawNumber(context.zone.x + context.zone.w, context.zone.y + context.zone.h - 19, value, flags1)
		lcd.drawNumber(context.zone.x + context.zone.w - 1, context.zone.y + context.zone.h - 20, value, flags2)
	end

end

function update(context, options)
	context.options = options
	context.back = nil
end

function refresh(context)
	drawGauge(context)
end

return { name="MyGauge", options=options, create=create, update=update, refresh=refresh }
