local snownet = require "snownet"
require "snownet.manager"	-- import snownet.launch, ...

local globalname = {}
local queryname = {}
local harbor = {}
local harbor_service

snownet.register_protocol {
	name = "harbor",
	id = snownet.PTYPE_HARBOR,
	pack = function(...) return ... end,
	unpack = snownet.tostring,
}

snownet.register_protocol {
	name = "text",
	id = snownet.PTYPE_TEXT,
	pack = function(...) return ... end,
	unpack = snownet.tostring,
}

local function response_name(name)
	local address = globalname[name]
	if queryname[name] then
		local tmp = queryname[name]
		queryname[name] = nil
		for _,resp in ipairs(tmp) do
			resp(true, address)
		end
	end
end

function harbor.REGISTER(name, handle)
	assert(globalname[name] == nil)
	globalname[name] = handle
	response_name(name)
	snownet.redirect(harbor_service, handle, "harbor", 0, "N " .. name)
end

function harbor.QUERYNAME(name)
	if name:byte() == 46 then	-- "." , local name
		snownet.ret(snownet.pack(snownet.localname(name)))
		return
	end
	local result = globalname[name]
	if result then
		snownet.ret(snownet.pack(result))
		return
	end
	local queue = queryname[name]
	if queue == nil then
		queue = { snownet.response() }
		queryname[name] = queue
	else
		table.insert(queue, snownet.response())
	end
end

function harbor.LINK(id)
	snownet.ret()
end

function harbor.CONNECT(id)
	snownet.error("Can't connect to other harbor in single node mode")
end

snownet.start(function()
	local harbor_id = tonumber(snownet.getenv "harbor")
	assert(harbor_id == 0)

	snownet.dispatch("lua", function (session,source,command,...)
		local f = assert(harbor[command])
		f(...)
	end)
	snownet.dispatch("text", function(session,source,command)
		-- ignore all the command
	end)

	harbor_service = assert(snownet.launch("harbor", harbor_id, snownet.self()))
end)
