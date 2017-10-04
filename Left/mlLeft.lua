local inputs = {
	{ "Source", SOURCE, 1 },
	{ "Tank_Size", VALUE, 0, 10000, 2000 },
}


local function run(source, size)
	local left = size - source

	setTelemetryValue(0xc00, 0, 2, left, UNIT_MILLILITERS, 0, "Left")
end

return { run=run, input=inputs }