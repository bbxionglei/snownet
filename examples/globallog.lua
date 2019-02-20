local snownet = require "snownet"
require "snownet.manager"	-- import snownet.register

snownet.start(function()
	snownet.dispatch("lua", function(session, address, ...)
		print("[GLOBALLOG]", snownet.address(address), ...)
	end)
	snownet.register ".log"
	snownet.register "LOG"
end)
