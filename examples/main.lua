local snownet = require "snownet"
local sprotoloader = require "sprotoloader"

local max_client = 64

snownet.start(function()
	snownet.error("Server start")
	snownet.uniqueservice("protoloader")
	if not snownet.getenv "daemon" then
		local console = snownet.newservice("console")
	end
	snownet.newservice("debug_console",8000)
	snownet.newservice("simpledb")
	local watchdog = snownet.newservice("watchdog")
	snownet.call(watchdog, "lua", "start", {
		port = 8888,
		maxclient = max_client,
		nodelay = true,
	})
	snownet.error("Watchdog listen on", 8888)
	snownet.exit()
end)
