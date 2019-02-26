local snownet = require "snownet"
local c = require "snownet.core"

function snownet.launch(...)
	local addr = c.command("LAUNCH", table.concat({...}," "))
	if addr then
		return tonumber("0x" .. string.sub(addr , 2))
	end
end

function snownet.kill(name)
	if type(name) == "number" then
		snownet.send(".launcher","lua","REMOVE",name, true)
		name = snownet.address(name)
	end
	c.command("KILL",name)
end

function snownet.abort()
	c.command("ABORT")
end

local function globalname(name, handle)
	local c = string.sub(name,1,1)
	assert(c ~= ':')
	if c == '.' then
		return false
	end

	assert(#name <= 16)	-- GLOBALNAME_LENGTH is 16, defined in snownet_harbor.h
	assert(tonumber(name) == nil)	-- global name can't be number

	local harbor = require "snownet.harbor"

	harbor.globalname(name, handle)

	return true
end

function snownet.register(name)
	if not globalname(name) then
		c.command("REG", name)
	end
end

function snownet.name(name, handle)
	if not globalname(name, handle) then
		c.command("NAME", name .. " " .. snownet.address(handle))
	end
end

local dispatch_message = snownet.dispatch_message

function snownet.forward_type(map, start_func)
	c.callback(function(ptype, msg, sz, ...)
		local prototype = map[ptype]
		if prototype then
			dispatch_message(prototype, msg, sz, ...)
		else
			local ok, err = pcall(dispatch_message, ptype, msg, sz, ...)
			c.trash(msg, sz)
			if not ok then
				error(err)
			end
		end
	end, true)
	snownet.timeout(0, function()
		snownet.init_service(start_func)
	end)
end

function snownet.filter(f ,start_func)
	c.callback(function(...)
		dispatch_message(f(...))
	end)
	snownet.timeout(0, function()
		snownet.init_service(start_func)
	end)
end

function snownet.monitor(service, query)
	local monitor
	if query then
		monitor = snownet.queryservice(true, service)
	else
		monitor = snownet.uniqueservice(true, service)
	end
	assert(monitor, "Monitor launch failed")
	c.command("MONITOR", string.format(":%08x", monitor))
	return monitor
end

return snownet
