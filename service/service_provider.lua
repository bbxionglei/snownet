local snownet = require "snownet"

local provider = {}

local function new_service(svr, name)
	local s = {}
	svr[name] = s
	s.queue = {}
	return s
end

local svr = setmetatable({}, { __index = new_service })


function provider.query(name)
	local s = svr[name]
	if s.queue then
		table.insert(s.queue, snownet.response())
	else
		if s.address then
			return snownet.ret(snownet.pack(s.address))
		else
			error(s.error)
		end
	end
end

local function boot(addr, name, code, ...)
	local s = svr[name]
	snownet.call(addr, "lua", "init", code, ...)
	local tmp = table.pack( ... )
	for i=1,tmp.n do
		tmp[i] = tostring(tmp[i])
	end

	if tmp.n > 0 then
		s.init = table.concat(tmp, ",")
	end
	s.time = snownet.time()
end

function provider.launch(name, code, ...)
	local s = svr[name]
	if s.booting then
		table.insert(s.queue, snownet.response())
	else
		s.booting = true
		local err
		local ok, addr = pcall(snownet.newservice,"service_cell", name)
		if ok then
			ok, err = xpcall(boot, debug.traceback, addr, name, code, ...)
		else
			err = addr
			addr = nil
		end
		s.booting = nil
		if ok then
			s.address = addr
			for _, resp in ipairs(s.queue) do
				resp(true, addr)
			end
			s.queue = nil
			snownet.ret(snownet.pack(addr))
		else
			if addr then
				snownet.send(addr, "debug", "EXIT")
			end
			s.error = err
			for _, resp in ipairs(s.queue) do
				resp(false)
			end
			s.queue = nil
			error(err)
		end
	end
end

function provider.test(name)
	local s = svr[name]
	if s.booting then
		snownet.ret(snownet.pack(nil, true))	-- booting
	elseif s.address then
		snownet.ret(snownet.pack(s.address))
	elseif s.error then
		error(s.error)
	else
		snownet.ret()	-- nil
	end
end

snownet.start(function()
	snownet.dispatch("lua", function(session, address, cmd, ...)
		provider[cmd](...)
	end)
	snownet.info_func(function()
		local info = {}
		for k,v in pairs(svr) do
			local status
			if v.booting then
				status = "booting"
			elseif v.queue then
				status = "waiting(" .. #v.queue .. ")"
			end
			info[snownet.address(v.address)] = {
				init = v.init,
				name = k,
				time = os.date("%Y %b %d %T %z",math.floor(v.time)),
				status = status,
			}
		end
		return info
	end)
end)
