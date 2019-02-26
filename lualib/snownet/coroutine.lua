-- You should use this module (snownet.coroutine) instead of origin lua coroutine in snownet framework

local coroutine = coroutine
-- origin lua coroutine module
local coroutine_resume = coroutine.resume
local coroutine_yield = coroutine.yield
local coroutine_status = coroutine.status
local coroutine_running = coroutine.running

local select = select
local snownetco = {}

snownetco.isyieldable = coroutine.isyieldable
snownetco.running = coroutine.running
snownetco.status = coroutine.status

local snownet_coroutines = setmetatable({}, { __mode = "kv" })

function snownetco.create(f)
	local co = coroutine.create(f)
	-- mark co as a snownet coroutine
	snownet_coroutines[co] = true
	return co
end

do -- begin snownetco.resume

	local profile = require "snownet.profile"
	-- snownet use profile.resume_co/yield_co instead of coroutine.resume/yield

	local snownet_resume = profile.resume_co
	local snownet_yield = profile.yield_co

	local function unlock(co, ...)
		snownet_coroutines[co] = true
		return ...
	end

	local function snownet_yielding(co, from, ...)
		snownet_coroutines[co] = false
		return unlock(co, snownet_resume(co, from, snownet_yield(from, ...)))
	end

	local function resume(co, from, ok, ...)
		if not ok then
			return ok, ...
		elseif coroutine_status(co) == "dead" then
			-- the main function exit
			snownet_coroutines[co] = nil
			return true, ...
		elseif (...) == "USER" then
			return true, select(2, ...)
		else
			-- blocked in snownet framework, so raise the yielding message
			return resume(co, from, snownet_yielding(co, from, ...))
		end
	end

	-- record the root of coroutine caller (It should be a snownet thread)
	local coroutine_caller = setmetatable({} , { __mode = "kv" })

function snownetco.resume(co, ...)
	local co_status = snownet_coroutines[co]
	if not co_status then
		if co_status == false then
			-- is running
			return false, "cannot resume a snownet coroutine suspend by snownet framework"
		end
		if coroutine_status(co) == "dead" then
			-- always return false, "cannot resume dead coroutine"
			return coroutine_resume(co, ...)
		else
			return false, "cannot resume none snownet coroutine"
		end
	end
	local from = coroutine_running()
	local caller = coroutine_caller[from] or from
	coroutine_caller[co] = caller
	return resume(co, caller, coroutine_resume(co, ...))
end

function snownetco.thread(co)
	co = co or coroutine_running()
	if snownet_coroutines[co] ~= nil then
		return coroutine_caller[co] , false
	else
		return co, true
	end
end

end -- end of snownetco.resume

function snownetco.status(co)
	local status = coroutine.status(co)
	if status == "suspended" then
		if snownet_coroutines[co] == false then
			return "blocked"
		else
			return "suspended"
		end
	else
		return status
	end
end

function snownetco.yield(...)
	return coroutine_yield("USER", ...)
end

do -- begin snownetco.wrap

	local function wrap_co(ok, ...)
		if ok then
			return ...
		else
			error(...)
		end
	end

function snownetco.wrap(f)
	local co = snownetco.create(function(...)
		return f(...)
	end)
	return function(...)
		return wrap_co(snownetco.resume(co, ...))
	end
end

end	-- end of snownetco.wrap

return snownetco
