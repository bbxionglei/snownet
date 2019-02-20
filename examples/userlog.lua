local snownet = require "snownet"
require "snownet.manager"

-- register protocol text before snownet.start would be better.
snownet.register_protocol {
	name = "text",
	id = snownet.PTYPE_TEXT,
	unpack = snownet.tostring,
	dispatch = function(_, address, msg)
		print(string.format(":%08x(%.2f): %s", address, snownet.time(), msg))
	end
}

snownet.register_protocol {
	name = "SYSTEM",
	id = snownet.PTYPE_SYSTEM,
	unpack = function(...) return ... end,
	dispatch = function()
		-- reopen signal
		print("SIGHUP")
	end
}

snownet.start(function()
end)