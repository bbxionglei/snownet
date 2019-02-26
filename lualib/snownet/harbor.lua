local snownet = require "snownet"

local harbor = {}

function harbor.globalname(name, handle)
	handle = handle or snownet.self()
	snownet.send(".cslave", "lua", "REGISTER", name, handle)
end

function harbor.queryname(name)
	return snownet.call(".cslave", "lua", "QUERYNAME", name)
end

function harbor.link(id)
	snownet.call(".cslave", "lua", "LINK", id)
end

function harbor.connect(id)
	snownet.call(".cslave", "lua", "CONNECT", id)
end

function harbor.linkmaster()
	snownet.call(".cslave", "lua", "LINKMASTER")
end

return harbor
