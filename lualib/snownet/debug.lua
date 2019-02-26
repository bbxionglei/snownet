local table = table
local extern_dbgcmd = {}

local function init(snownet, export)
	local internal_info_func

	function snownet.info_func(func)
		internal_info_func = func
	end

	local dbgcmd

	local function init_dbgcmd()
		dbgcmd = {}

		function dbgcmd.MEM()
			local kb, bytes = collectgarbage "count"
			snownet.ret(snownet.pack(kb,bytes))
		end

		function dbgcmd.GC()

			collectgarbage "collect"
		end

		function dbgcmd.STAT()
			local stat = {}
			stat.task = snownet.task()
			stat.mqlen = snownet.stat "mqlen"
			stat.cpu = snownet.stat "cpu"
			stat.message = snownet.stat "message"
			snownet.ret(snownet.pack(stat))
		end

		function dbgcmd.TASK(session)
			if session then
				snownet.ret(snownet.pack(snownet.task(session)))
			else
				local task = {}
				snownet.task(task)
				snownet.ret(snownet.pack(task))
			end
		end

		function dbgcmd.INFO(...)
			if internal_info_func then
				snownet.ret(snownet.pack(internal_info_func(...)))
			else
				snownet.ret(snownet.pack(nil))
			end
		end

		function dbgcmd.EXIT()
			snownet.exit()
		end

		function dbgcmd.RUN(source, filename, ...)
			local inject = require "snownet.inject"
			local args = table.pack(...)
			local ok, output = inject(snownet, source, filename, args, export.dispatch, snownet.register_protocol)
			collectgarbage "collect"
			snownet.ret(snownet.pack(ok, table.concat(output, "\n")))
		end

		function dbgcmd.TERM(service)
			snownet.term(service)
		end

		function dbgcmd.REMOTEDEBUG(...)
			local remotedebug = require "snownet.remotedebug"
			remotedebug.start(export, ...)
		end

		function dbgcmd.SUPPORT(pname)
			return snownet.ret(snownet.pack(snownet.dispatch(pname) ~= nil))
		end

		function dbgcmd.PING()
			return snownet.ret()
		end

		function dbgcmd.LINK()
			snownet.response()	-- get response , but not return. raise error when exit
		end

		function dbgcmd.TRACELOG(proto, flag)
			if type(proto) ~= "string" then
				flag = proto
				proto = "lua"
			end
			snownet.error(string.format("Turn trace log %s for %s", flag, proto))
			snownet.traceproto(proto, flag)
			snownet.ret()
		end

		return dbgcmd
	end -- function init_dbgcmd

	local function _debug_dispatch(session, address, cmd, ...)
		dbgcmd = dbgcmd or init_dbgcmd() -- lazy init dbgcmd
		local f = dbgcmd[cmd] or extern_dbgcmd[cmd]
		assert(f, cmd)
		f(...)
	end

	snownet.register_protocol {
		name = "debug",
		id = assert(snownet.PTYPE_DEBUG),
		pack = assert(snownet.pack),
		unpack = assert(snownet.unpack),
		dispatch = _debug_dispatch,
	}
end

local function reg_debugcmd(name, fn)
	extern_dbgcmd[name] = fn
end

return {
	init = init,
	reg_debugcmd = reg_debugcmd,
}
