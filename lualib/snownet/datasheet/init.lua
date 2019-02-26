local snownet = require "snownet"
local service = require "snownet.service"
local core = require "snownet.datasheet.core"

local datasheet_svr

snownet.init(function()
	datasheet_svr = service.query "datasheet"
end)

local datasheet = {}
local sheets = setmetatable({}, {
	__gc = function(t)
		for _,v in pairs(t) do
			snownet.send(datasheet_svr, "lua", "release", v.handle)
		end
	end,
})

local function querysheet(name)
	return snownet.call(datasheet_svr, "lua", "query", name)
end

local function updateobject(name)
	local t = sheets[name]
	if not t.object then
		t.object = core.new(t.handle)
	end
	local function monitor()
		local handle = t.handle
		local newhandle = snownet.call(datasheet_svr, "lua", "monitor", handle)
		core.update(t.object, newhandle)
		t.handle = newhandle
		snownet.send(datasheet_svr, "lua", "release", handle)
		return monitor()
	end
	snownet.fork(monitor)
end

function datasheet.query(name)
	local t = sheets[name]
	if not t then
		t = {}
		sheets[name] = t
	end
	if t.error then
		error(t.error)
	end
	if t.object then
		return t.object
	end
	if t.queue then
		local co = coroutine.running()
		table.insert(t.queue, co)
		snownet.wait(co)
	else
		t.queue = {}	-- create wait queue for other query
		local ok, handle = pcall(querysheet, name)
		if ok then
			t.handle = handle
			updateobject(name)
		else
			t.error = handle
		end
		local q = t.queue
		t.queue = nil
		for _, co in ipairs(q) do
			snownet.wakeup(co)
		end
	end
	if t.error then
		error(t.error)
	end
	return t.object
end

return datasheet
