local snownet = require "snownet"

local list = {}

local function timeout_check(ti)
	if not next(list) then
		return
	end
	snownet.sleep(ti)	-- sleep 10 sec
	for k,v in pairs(list) do
		snownet.error("timout",ti,k,v)
	end
end

snownet.start(function()
	snownet.error("ping all")
	local list_ret = snownet.call(".launcher", "lua", "LIST")
	for addr, desc in pairs(list_ret) do
		list[addr] = desc
		snownet.fork(function()
			snownet.call(addr,"debug","INFO")
			list[addr] = nil
		end)
	end
	snownet.sleep(0)
	timeout_check(100)
	timeout_check(400)
	timeout_check(500)
	snownet.exit()
end)
