local snownet = require "snownet"

snownet.register_protocol {
	name = "client",
	id = snownet.PTYPE_CLIENT,
	unpack = snownet.tostring,
}

local gate
local userid, subid

local CMD = {}

function CMD.login(source, uid, sid, secret)
	-- you may use secret to make a encrypted data stream
	snownet.error(string.format("%s is login", uid))
	gate = source
	userid = uid
	subid = sid
	-- you may load user data from database
end

local function logout()
	if gate then
		snownet.call(gate, "lua", "logout", userid, subid)
	end
	snownet.exit()
end

function CMD.logout(source)
	-- NOTICE: The logout MAY be reentry
	snownet.error(string.format("%s is logout", userid))
	logout()
end

function CMD.afk(source)
	-- the connection is broken, but the user may back
	snownet.error(string.format("AFK"))
end

snownet.start(function()
	-- If you want to fork a work thread , you MUST do it in CMD.login
	snownet.dispatch("lua", function(session, source, command, ...)
		local f = assert(CMD[command])
		snownet.ret(snownet.pack(f(source, ...)))
	end)

	snownet.dispatch("client", function(_,_, msg)
		-- the simple echo service
		snownet.sleep(10)	-- sleep a while
		snownet.ret(msg)
	end)
end)
