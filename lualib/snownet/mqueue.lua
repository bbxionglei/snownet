-- This is a deprecated module, use snownet.queue instead.

local snownet = require "snownet"
local c = require "snownet.core"

local mqueue = {}
local init_once
local thread_id
local message_queue = {}

snownet.register_protocol {
	name = "queue",
	-- please read snownet.h for magic number 8
	id = 8,
	pack = snownet.pack,
	unpack = snownet.unpack,
	dispatch = function(session, from, ...)
		table.insert(message_queue, {session = session, addr = from, ... })
		if thread_id then
			snownet.wakeup(thread_id)
			thread_id = nil
		end
	end
}

local function do_func(f, msg)
	return pcall(f, table.unpack(msg))
end

local function message_dispatch(f)
	while true do
		if #message_queue==0 then
			thread_id = coroutine.running()
			snownet.wait()
		else
			local msg = table.remove(message_queue,1)
			local session = msg.session
			if session == 0 then
				local ok, msg = do_func(f, msg)
				if ok then
					if msg then
						snownet.fork(message_dispatch,f)
						error(string.format("[:%x] send a message to [:%x] return something", msg.addr, snownet.self()))
					end
				else
					snownet.fork(message_dispatch,f)
					error(string.format("[:%x] send a message to [:%x] throw an error : %s", msg.addr, snownet.self(),msg))
				end
			else
				local data, size = snownet.pack(do_func(f,msg))
				-- 1 means response
				c.send(msg.addr, 1, session, data, size)
			end
		end
	end
end

function mqueue.register(f)
	assert(init_once == nil)
	init_once = true
	snownet.fork(message_dispatch,f)
end

local function catch(succ, ...)
	if succ then
		return ...
	else
		error(...)
	end
end

function mqueue.call(addr, ...)
	return catch(snownet.call(addr, "queue", ...))
end

function mqueue.send(addr, ...)
	return snownet.send(addr, "queue", ...)
end

function mqueue.size()
	return #message_queue
end

return mqueue
