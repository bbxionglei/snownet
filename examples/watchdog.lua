local snownet = require "snownet"

local CMD = {}
local SOCKET = {}
local gate
local agent = {}

function SOCKET.open(fd, addr)
	snownet.error("New client from : " .. addr)
	agent[fd] = snownet.newservice("agent")
	snownet.call(agent[fd], "lua", "start", { gate = gate, client = fd, watchdog = snownet.self() })
end

local function close_agent(fd)
	local a = agent[fd]
	agent[fd] = nil
	if a then
		snownet.call(gate, "lua", "kick", fd)
		-- disconnect never return
		snownet.send(a, "lua", "disconnect")
	end
end

function SOCKET.close(fd)
	print("socket close",fd)
	close_agent(fd)
end

function SOCKET.error(fd, msg)
	print("socket error",fd, msg)
	close_agent(fd)
end

function SOCKET.warning(fd, size)
	-- size K bytes havn't send out in fd
	print("socket warning", fd, size)
end

function SOCKET.data(fd, msg)
end

function CMD.start(conf)
	snownet.call(gate, "lua", "open" , conf)
end

function CMD.close(fd)
	close_agent(fd)
end

snownet.start(function()
	snownet.dispatch("lua", function(session, source, cmd, subcmd, ...)
		if cmd == "socket" then
			local f = SOCKET[subcmd]
			f(...)
			-- socket api don't need return
		else
			local f = assert(CMD[cmd])
			snownet.ret(snownet.pack(f(subcmd, ...)))
		end
	end)

	gate = snownet.newservice("gate")
end)
