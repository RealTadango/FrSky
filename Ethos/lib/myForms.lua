-- parameters { name, type, default, value, min, max, suffix, remoteid }

local myForms = {}

function myForms.getValue(parameter)
	if parameter[4] == nil then
	  	return 0
	else
	  	return parameter[4]
	end
end

function myForms.setValue(parameter, value)
	parameter[4] = value
end

function myForms.createChoiceField(line, parameter, parameterChanged)
	field = form.addChoiceField(line, nil, parameter[5], 
		function() --get value
			return myForms.getValue(parameter) 
		end, --set value
		function(value) 
			myForms.setValue(parameter, value) 
			if  parameterChanged ~= nil then
				parameterChanged(parameter) 
			end
		end
	)

	if #parameter >= 7 and parameter[7] ~= nil then
		field:suffix(parameter[7])
	end

	if parameter[4] == nil then
		field:enable(false)
  	end

	return field
end

function myForms.createNumberField(line, parameter, parameterChanged)
	field = form.addNumberField(line, nil, parameter[5], parameter[6], 
		function() --get value
			return myForms.getValue(parameter) 
		end, --set value
		function(value) 
			myForms.setValue(parameter, value) 
			if  parameterChanged ~= nil then
				parameterChanged(parameter) 
			end
		end
	)

	field:enableInstantChange(false)

	if #parameter >= 7 and parameter[7] ~= nil then
		field:suffix(parameter[7])
	end
	field:enable(false)
	return field
end

function myForms.clear()
	form.clear()
	myForms.parameters = nil
	myForms.fields = nil
end

function myForms.load(parameters, parameterChanged)
	myForms.parameters = parameters
	myForms.fields = {}

	for index = 1, #parameters do
		parameter = parameters[index]
		line = form.addLine(parameter[1])
		field = parameter[2](line, parameter, parameterChanged)
		myForms.fields[#myForms.fields + 1] = field
	end
end

function myForms.updateField(index, value)
	myForms.parameters[index][4] = value
	if myForms.parameters[index][4] ~= nil then
		myForms.fields[index]:enable(true)
	end
end

return myForms