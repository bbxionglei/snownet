local snownet = require "snownet"

local datacenter = {}

function datacenter.get(...)
	return snownet.call("DATACENTER", "lua", "QUERY", ...)
end

function datacenter.set(...)
	return snownet.call("DATACENTER", "lua", "UPDATE", ...)
end

function datacenter.wait(...)
	return snownet.call("DATACENTER", "lua", "WAIT", ...)
end

return datacenter

