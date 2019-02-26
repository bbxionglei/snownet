local snownet = require "snownet"
local memory = require "snownet.memory"

memory.dumpinfo()
--memory.dump()
local info = memory.info()
for k,v in pairs(info) do
	print(string.format(":%08x %gK",k,v/1024))
end

print("Total memory:", memory.total())
print("Total block:", memory.block())

snownet.start(function() snownet.exit() end)
