local snownet = require "snownet"

local service_name = (...)
local init = {}

function init.init(code, ...)
	local start_func
	snownet.start = function(f)
		start_func = f
	end
	snownet.dispatch("lua", function() error("No dispatch function")	end)
	local mainfunc = assert(load(code, service_name))
	assert(snownet.pcall(mainfunc,...))
	if start_func then
		start_func()
	end
	snownet.ret()
end

snownet.start(function()
	snownet.dispatch("lua", function(_,_,cmd,...)
		init[cmd](...)
	end)
end)
