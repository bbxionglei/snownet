# snownet
改自skynet，自己学习的项目
目的：支持windows、linux、freeBSD
中文译为：雪网


我对skynet框架的理解
1,skynet-src
	a)启三个管理线程和N个工作线程
	b)通过snlua配合参数起动不同的lua逻辑服务
2,service-src
	a)snlua是核心服务，加载lua并执行,如果不想用lua,想用python的脚本，
		模仿这个服务写一个新的服务，并把配置中的bootstrap = "snlua bootstrap" 改成 bootstrap = "snpython bootstrap"
	b)其它几个服务理论上可以直接用lua写
	c)如果其它服务有需要性能的处理也可以在这写
3,lualib-src
	通过luaopen_和里面的luaL_Reg注册"C语言核心接口"，用于与核心框架交互，如：
	注册服务、push or pop消息队列
4,lualib
	调用"C语言核心接口"并封装成"lua核心接口"，提供给其它逻辑服务调用
5,service
	lua写的逻辑服务调用"lua核心接口"，如果做开发，理论上如果"lua核心接口"够用，则只需要在server下写逻辑服务lua就够了
	如果不够，则要先在lualib-src中写一个服务增加对应的接口，然后在lualib-src封装接口并提供lua接口，最后在逻辑服务里面调用即可
6,3rd
	a)jemalloc:第一次听说，貌似是内存管理钓子
	b)lpeg:干什么用的，不明白
	c)lua:云大改写的lua源代码
	d)lua-md5:不知道为什么要写个md5，难道lua本身没有md5的功能？
7,test、examples 
	没什么好写的，参考的地方
