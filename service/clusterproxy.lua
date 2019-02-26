local snownet = require "snownet"
local cluster = require "snownet.cluster"
require "snownet.manager"	-- inject snownet.forward_type

local node, address = ...

snownet.register_protocol {
	name = "system",
	id = snownet.PTYPE_SYSTEM,
	unpack = function (...) return ... end,
}

local forward_map = {
	[snownet.PTYPE_SNAX] = snownet.PTYPE_SYSTEM,
	[snownet.PTYPE_LUA] = snownet.PTYPE_SYSTEM,
	[snownet.PTYPE_RESPONSE] = snownet.PTYPE_RESPONSE,	-- don't free response message
}

snownet.forward_type( forward_map ,function()
	local clusterd = snownet.uniqueservice("clusterd")
	local n = tonumber(address)
	if n then
		address = n
	end
	snownet.dispatch("system", function (session, source, msg, sz)
		if session == 0 then
			snownet.send(clusterd, "lua", "push", node, address, msg, sz)
		else
			snownet.ret(snownet.rawcall(clusterd, "lua", snownet.pack("req", node, address, msg, sz)))
		end
	end)
end)
