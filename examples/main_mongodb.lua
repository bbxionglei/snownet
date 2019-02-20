local snownet = require "snownet"


snownet.start(function()
	print("Main Server start")
	local console = snownet.newservice(
		"testmongodb", "127.0.0.1", 27017, "testdb", "test", "test"
	)
	
	print("Main Server exit")
	snownet.exit()
end)
