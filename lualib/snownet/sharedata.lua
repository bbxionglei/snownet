local snownet = require "snownet"
local sd = require "snownet.sharedata.corelib"

local service

snownet.init(function()
	service = snownet.uniqueservice "sharedatad"
end)

local sharedata = {}
local cache = setmetatable({}, { __mode = "kv" })

local function monitor(name, obj, cobj)
	local newobj = cobj
	while true do
		newobj = snownet.call(service, "lua", "monitor", name, newobj)
		if newobj == nil then
			break
		end
		sd.update(obj, newobj)
	end
	if cache[name] == obj then
		cache[name] = nil
	end
end

function sharedata.query(name)
	if cache[name] then
		return cache[name]
	end
	local obj = snownet.call(service, "lua", "query", name)
	local r = sd.box(obj)
	snownet.send(service, "lua", "confirm" , obj)
	snownet.fork(monitor,name, r, obj)
	cache[name] = r
	return r
end

function sharedata.new(name, v, ...)
	snownet.call(service, "lua", "new", name, v, ...)
end

function sharedata.update(name, v, ...)
	snownet.call(service, "lua", "update", name, v, ...)
end

function sharedata.delete(name)
	snownet.call(service, "lua", "delete", name)
end

function sharedata.flush()
	for name, obj in pairs(cache) do
		sd.flush(obj)
	end
	collectgarbage()
end

function sharedata.deepcopy(name, ...)
	if cache[name] then
		local cobj = cache[name].__obj
		return sd.copy(cobj, ...)
	end

	local cobj = snownet.call(service, "lua", "query", name)
	local ret = sd.copy(cobj, ...)
	snownet.send(service, "lua", "confirm" , cobj)
	return ret
end

return sharedata
