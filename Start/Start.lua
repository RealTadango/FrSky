local outputs = { "A_ON", "A_TH" }
local inputs = {
	{ "Activate", SOURCE, 1 },
	{ "Activ_Low", VALUE, -100, 100, 0 },
	{ "Activ_High", VALUE, -100, 100, 100 },
	{ "Delay", VALUE, 1, 30, 5 }, 
	{ "Duration", VALUE, 1, 120, 20 }, 
}

local start
local isActive = false
local lastCall

local function run(activate, low, high, delay, duration)
	if (activate / 10.24) >= low and (activate / 10.24) <= high then
		if not isActive then
			isActive = true
			start = getTime()
			lastCall = 0
		end
	else
		isActive = false
	end
	
	on = -1024
	th = 0
	
	if isActive then
		d = (getTime() - start) / 100
		
		if d < delay then
			till = math.ceil(delay - d)
			if lastCall ~= till then
				lastCall = till
				playNumber(till, 0)
			end
		end
		
		if d < (delay + duration) then
			on = 1024

			if d >= delay then
				th = 2048
			end
		end
	end

	return on, th
end

return { run=run, output=outputs, input=inputs }