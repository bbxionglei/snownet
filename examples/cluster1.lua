local snownet = require "snownet"
local cluster = require "snownet.cluster"
local snax = require "snownet.snax"

snownet.start(function()
	cluster.reload {
		db = "127.0.0.1:2528",
		db2 = "127.0.0.1:2529",
	}

	local sdb = snownet.newservice("simpledb")
	-- register name "sdb" for simpledb, you can use cluster.query() later.
	-- See cluster2.lua
	cluster.register("sdb", sdb)

	print(snownet.call(sdb, "lua", "SET", "a", "foobar"))
	print(snownet.call(sdb, "lua", "SET", "b", "foobar2"))
	print(snownet.call(sdb, "lua", "GET", "a"))
	print(snownet.call(sdb, "lua", "GET", "b"))
	cluster.open "db"
	cluster.open "db2"
	-- unique snax service
	snax.uniqueservice "pingserver"
end)
