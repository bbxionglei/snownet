local snownet = require "snownet"
local harbor = require "snownet.harbor"
require "snownet.manager"	-- import snownet.monitor

local function monitor_master()
	harbor.linkmaster()
	print("master is down")
	snownet.exit()
end

snownet.start(function()
	print("Log server start")
	snownet.monitor "simplemonitor"
	local log = snownet.newservice("globallog")
	snownet.fork(monitor_master)
end)

