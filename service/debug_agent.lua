local snownet = require "snownet"
local debugchannel = require "snownet.debugchannel"

local CMD = {}

local channel

function CMD.start(address, fd)
	assert(channel == nil, "start more than once")
	snownet.error(string.format("Attach to :%08x", address))
	local handle
	channel, handle = debugchannel.create()
	local ok, err = pcall(snownet.call, address, "debug", "REMOTEDEBUG", fd, handle)
	if not ok then
		snownet.ret(snownet.pack(false, "Debugger attach failed"))
	else
		-- todo hook
		snownet.ret(snownet.pack(true))
	end
	snownet.exit()
end

function CMD.cmd(cmdline)
	channel:write(cmdline)
end

function CMD.ping()
	snownet.ret()
end

snownet.start(function()
	snownet.dispatch("lua", function(_,_,cmd,...)
		local f = CMD[cmd]
		f(...)
	end)
end)
