local snownet = require "snownet"
local sharedata = require "snownet.sharedata"

local mode = ...

if mode == "host" then

snownet.start(function()
	snownet.error("new foobar")
	sharedata.new("foobar", { a=1, b= { "hello",  "world" } })

	snownet.fork(function()
		snownet.sleep(200)	-- sleep 2s
		snownet.error("update foobar a = 2")
		sharedata.update("foobar", { a =2 })
		snownet.sleep(200)	-- sleep 2s
		snownet.error("update foobar a = 3")
		sharedata.update("foobar", { a = 3, b = { "change" } })
		snownet.sleep(100)
		snownet.error("delete foobar")
		sharedata.delete "foobar"
	end)
end)

else


snownet.start(function()
	snownet.newservice(SERVICE_NAME, "host")

	local obj = sharedata.query "foobar"

	local b = obj.b
	snownet.error(string.format("a=%d", obj.a))

	for k,v in ipairs(b) do
		snownet.error(string.format("b[%d]=%s", k,v))
	end

	-- test lua serialization
	local s = snownet.packstring(obj)
	local nobj = snownet.unpack(s)
	for k,v in pairs(nobj) do
		snownet.error(string.format("nobj[%s]=%s", k,v))
	end
	for k,v in ipairs(nobj.b) do
		snownet.error(string.format("nobj.b[%d]=%s", k,v))
	end

	for i = 1, 5 do
		snownet.sleep(100)
		snownet.error("second " ..i)
		for k,v in pairs(obj) do
			snownet.error(string.format("%s = %s", k , tostring(v)))
		end
	end

	local ok, err = pcall(function()
		local tmp = { b[1], b[2] }	-- b is invalid , so pcall should failed
	end)

	if not ok then
		snownet.error(err)
	end

	-- obj. b is not the same with local b
	for k,v in ipairs(obj.b) do
		snownet.error(string.format("b[%d] = %s", k , tostring(v)))
	end

	collectgarbage()
	snownet.error("sleep")
	snownet.sleep(100)
	b = nil
	collectgarbage()
	snownet.error("sleep")
	snownet.sleep(100)

	snownet.exit()
end)

end
