local snownet = require "snownet"
local harbor = require "snownet.harbor"
require "snownet.manager"	-- import snownet.launch, ...
local memory = require "snownet.memory"

snownet.start(function()
	local sharestring = tonumber(snownet.getenv "sharestring" or 4096)
	memory.ssexpand(sharestring)

	local standalone = snownet.getenv "standalone"

	local launcher = assert(snownet.launch("snlua","launcher"))
	snownet.name(".launcher", launcher)

	local harbor_id = tonumber(snownet.getenv "harbor" or 0)
	if harbor_id == 0 then
		assert(standalone ==  nil)
		standalone = true
		snownet.setenv("standalone", "true")

		local ok, slave = pcall(snownet.newservice, "cdummy")
		if not ok then
			snownet.abort()
		end
		snownet.name(".cslave", slave)

	else
		if standalone then
			if not pcall(snownet.newservice,"cmaster") then
				snownet.abort()
			end
		end

		local ok, slave = pcall(snownet.newservice, "cslave")
		if not ok then
			snownet.abort()
		end
		snownet.name(".cslave", slave)
	end

	if standalone then
		local datacenter = snownet.newservice "datacenterd"
		snownet.name("DATACENTER", datacenter)
	end
	snownet.newservice "service_mgr"
	pcall(snownet.newservice,snownet.getenv "start" or "main")
	snownet.exit()
end)
