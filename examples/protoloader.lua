-- module proto as examples/proto.lua
package.path = "./examples/?.lua;" .. package.path

local snownet = require "snownet"
local sprotoparser = require "sprotoparser"
local sprotoloader = require "sprotoloader"
local proto = require "proto"

snownet.start(function()
	sprotoloader.save(proto.c2s, 1)
	sprotoloader.save(proto.s2c, 2)
	-- don't call snownet.exit() , because sproto.core may unload and the global slot become invalid
end)
