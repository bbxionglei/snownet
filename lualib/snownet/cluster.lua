local snownet = require "snownet"

local clusterd
local cluster = {}

function cluster.call(node, address, ...)
	-- snownet.pack(...) will free by cluster.core.packrequest
	return snownet.call(clusterd, "lua", "req", node, address, snownet.pack(...))
end

function cluster.send(node, address, ...)
	-- push is the same with req, but no response
	snownet.send(clusterd, "lua", "push", node, address, snownet.pack(...))
end

function cluster.open(port)
	if type(port) == "string" then
		snownet.call(clusterd, "lua", "listen", port)
	else
		snownet.call(clusterd, "lua", "listen", "0.0.0.0", port)
	end
end

function cluster.reload(config)
	snownet.call(clusterd, "lua", "reload", config)
end

function cluster.proxy(node, name)
	return snownet.call(clusterd, "lua", "proxy", node, name)
end

function cluster.snax(node, name, address)
	local snax = require "snownet.snax"
	if not address then
		address = cluster.call(node, ".service", "QUERY", "snaxd" , name)
	end
	local handle = snownet.call(clusterd, "lua", "proxy", node, address)
	return snax.bind(handle, name)
end

function cluster.register(name, addr)
	assert(type(name) == "string")
	assert(addr == nil or type(addr) == "number")
	return snownet.call(clusterd, "lua", "register", name, addr)
end

function cluster.query(node, name)
	return snownet.call(clusterd, "lua", "req", node, 0, snownet.pack(name))
end

snownet.init(function()
	clusterd = snownet.uniqueservice("clusterd")
end)

return cluster
