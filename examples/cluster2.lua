local snownet = require "snownet"
local cluster = require "snownet.cluster"

snownet.start(function()
	local proxy = cluster.proxy "db@sdb"	-- cluster.proxy("db", "@sdb")
	local largekey = string.rep("X", 128*1024)
	local largevalue = string.rep("R", 100 * 1024)
	snownet.call(proxy, "lua", "SET", largekey, largevalue)
	local v = snownet.call(proxy, "lua", "GET", largekey)
	assert(largevalue == v)
	snownet.send(proxy, "lua", "PING", "proxy")

	snownet.fork(function()
		snownet.trace("cluster")
		print(cluster.call("db", "@sdb", "GET", "a"))
		print(cluster.call("db2", "@sdb", "GET", "b"))
		cluster.send("db2", "@sdb", "PING", "db2:longstring" .. largevalue)
	end)

	-- test snax service
	snownet.timeout(300,function()
		cluster.reload {
			db = false,	-- db is down
			db3 = "127.0.0.1:2529"
		}
		print(pcall(cluster.call, "db", "@sdb", "GET", "a"))	-- db is down
	end)
	cluster.reload { __nowaiting = false }
	local pingserver = cluster.snax("db3", "pingserver")
	print(pingserver.req.ping "hello")
end)
