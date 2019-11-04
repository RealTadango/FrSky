local inputs = {
	{ "Activate", SOURCE, 1 },
	{ "Activ_Low", VALUE, -100, 100, 0 },
	{ "Activ_High", VALUE, -100, 100, 100 },
	{ "Input", SOURCE, 1 },
	{ "Stall_SPD", VALUE, 1, 200, 60 }, 
	{ "High_SPD", VALUE, 1, 200, 80 }, 
}

local lastCall = 0

local function run(activate, low, high, input, stall, overspeed)
	if (activate / 10.24) >= low and (activate / 10.24) <= high then
		if input <= stall then
			if (getTime() - lastCall) > 180 then
				print("Stall")
				lastCall = getTime()
				playTone(400, 300, 100, 0, 1)
				playTone(400, 300, 100, 0, 1)
				playTone(400, 500, 200, 0, 1)
			end
		elseif input >= overspeed then
			if (getTime() - lastCall) > 180 then
				print("Speed")
				lastCall = getTime()
				playNumber(input, 0)
			end
		end
	end
	
end

return { run=run, input=inputs }