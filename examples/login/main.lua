local snownet = require "snownet"

snownet.start(function()
	local loginserver = snownet.newservice("logind")
	local gate = snownet.newservice("gated", loginserver)

	snownet.call(gate, "lua", "open" , {
		port = 8888,
		maxclient = 64,
		servername = "sample",
	})
end)
