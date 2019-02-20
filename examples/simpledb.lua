local snownet = require "snownet"
require "snownet.manager"	-- import snownet.register
local db = {}

local command = {}

function command.GET(key)
	return db[key]
end

function command.SET(key, value)
	local last = db[key]
	db[key] = value
	return last
end

snownet.start(function()
	snownet.dispatch("lua", function(session, address, cmd, ...)
		cmd = cmd:upper()
		if cmd == "PING" then
			assert(session == 0)
			local str = (...)
			if #str > 20 then
				str = str:sub(1,20) .. "...(" .. #str .. ")"
			end
			snownet.error(string.format("%s ping %s", snownet.address(address), str))
			return
		end
		local f = command[cmd]
		if f then
			snownet.ret(snownet.pack(f(...)))
		else
			error(string.format("Unknown command %s", tostring(cmd)))
		end
	end)
--	snownet.traceproto("lua", false)	-- true off tracelog
	snownet.register "SIMPLEDB"
end)
