local snownet = require "snownet"


snownet.start(function()
	print("Main Server start")
	local console = snownet.newservice("testmysql")
	
	print("Main Server exit")
	snownet.exit()
end)
