include platform.mk

#����C������������� <n>.o��Ŀ�������Ŀ����Զ��Ƶ�Ϊ<n>.c������������������$(CC) �Cc $(CPPFLAGS) $(CFLAGS)
#����C++�������������<n>.o��Ŀ�������Ŀ����Զ��Ƶ�Ϊ<n>.cc����<n>.C������������������$(CXX) �Cc $(CPPFLAGS) $(CFLAGS)

LUA_CLIB_PATH ?= luaclib
CSERVICE_PATH ?= cservice

SNOWNET_BUILD_PATH ?= ../bin

CFLAGS = -g -O0 -Wall -std=c++11
#CFLAGS = -g -O0 -Wall -I$(LUA_INC) $(MYCFLAGS)
#CFLAGS = -g -O2 -Wall -I$(LUA_INC) $(MYCFLAGS)
# CFLAGS += -DUSE_PTHREAD_LOCK

# lua

#LUA_STATICLIB := 3rd/lua/liblua.a
#LUA_LIB ?= $(LUA_STATICLIB)
#LUA_INC ?= 3rd/lua

#$(LUA_STATICLIB) :
	#@echo "LUA_STATICLIB $(LUA_STATICLIB)"
	#cd 3rd/lua && $(MAKE) CC='$(CC) -std=gnu99' $(PLAT)

# jemalloc 

#JEMALLOC_STATICLIB := 3rd/jemalloc/lib/libjemalloc_pic.a
#JEMALLOC_INC := 3rd/jemalloc/include/jemalloc

#all : jemalloc
	
#.PHONY : jemalloc update3rd

#MALLOC_STATICLIB := $(JEMALLOC_STATICLIB)

#$(JEMALLOC_STATICLIB) : 3rd/jemalloc/Makefile
	#cd 3rd/jemalloc && $(MAKE) CC=$(CC) 

#3rd/jemalloc/autogen.sh :
	#git submodule update --init

#3rd/jemalloc/Makefile : | 3rd/jemalloc/autogen.sh
	#cd 3rd/jemalloc && ./autogen.sh --with-jemalloc-prefix=je_ --disable-valgrind

#jemalloc : $(MALLOC_STATICLIB)

#update3rd :
	#rm -rf 3rd/jemalloc && git submodule update --init

# snownet

CSERVICE = snlua logger gate harbor
LUA_CLIB = snownet \
  client \
  bson md5 sproto lpeg

LUA_CLIB_SNOWNET = 

SNOWNET_SRC = \
	snownet_main.cc snownet_test.cc 

all : 
	@echo -e "\033[36m before all \033[0m"
	$(MAKE) clean
	$(MAKE) $(SNOWNET_BUILD_PATH)/snownet 
	#$(foreach v, $(LUA_CLIB), $(LUA_CLIB_PATH)/$(v).so) 
	#$(MAKE) $(foreach v, $(CSERVICE), $(CSERVICE_PATH)/$(v).so) 
	@echo -e "\033[36m after  all \033[0m"

$(SNOWNET_BUILD_PATH)/snownet : $(foreach v, $(SNOWNET_SRC), snownet-src/$(v)) $(LUA_LIB) $(MALLOC_STATICLIB)
	@echo -e "\033[32m before snownet \033[0m"
	@echo $(foreach v, $(SNOWNET_SRC), snownet-src/$(v))
	$(CC) $(CFLAGS) -o $@ $^ -Isnownet-src -I$(JEMALLOC_INC) $(LDFLAGS) $(EXPORT) $(SNOWNET_LIBS) $(SNOWNET_DEFINES)
	@echo -e "\033[32m after  snownet \033[0m"

$(LUA_CLIB_PATH) :
	@echo -e "\033[36m LUA_CLIB_PATH \033[0m"
	mkdir $(LUA_CLIB_PATH)

$(CSERVICE_PATH) :
	@echo -e "\033[36m CSERVICE_PATH \033[0m"
	mkdir $(CSERVICE_PATH)

define CSERVICE_TEMP
  $$(CSERVICE_PATH)/$(1).so : service-src/service_$(1).cc | $$(CSERVICE_PATH)
	$$(CC) $$(CFLAGS) $$(SHARED) $$< -o $$@ -Isnownet-src
endef

$(foreach v, $(CSERVICE), $(eval $(call CSERVICE_TEMP,$(v))))

$(LUA_CLIB_PATH)/snownet.so : $(addprefix lualib-src/,$(LUA_CLIB_SNOWNET)) | $(LUA_CLIB_PATH)
	@echo -e "\033[36m snownet.so \033[0m"
	$(CC) $(CFLAGS) $(SHARED) $^ -o $@ -Isnownet-src -Iservice-src -Ilualib-src

$(LUA_CLIB_PATH)/bson.so : lualib-src/lua-bson.cc | $(LUA_CLIB_PATH)
	@echo -e "\033[36m bson.so \033[0m"
	$(CC) $(CFLAGS) $(SHARED) -Isnownet-src $^ -o $@ -Isnownet-src

$(LUA_CLIB_PATH)/md5.so : 3rd/lua-md5/md5.cc 3rd/lua-md5/md5lib.cc 3rd/lua-md5/compat-5.2.cc | $(LUA_CLIB_PATH)
	@echo -e "\033[36m md5.so \033[0m"
	$(CC) $(CFLAGS) $(SHARED) -I3rd/lua-md5 $^ -o $@ 

$(LUA_CLIB_PATH)/client.so : lualib-src/lua-clientsocket.cc lualib-src/lua-crypt.cc lualib-src/lsha1.cc | $(LUA_CLIB_PATH)
	@echo -e "\033[36m client.so \033[0m"
	$(CC) $(CFLAGS) $(SHARED) $^ -o $@ -lpthread

$(LUA_CLIB_PATH)/sproto.so : lualib-src/sproto/sproto.cc lualib-src/sproto/lsproto.cc | $(LUA_CLIB_PATH)
	@echo -e "\033[36m sproto.so \033[0m"
	$(CC) $(CFLAGS) $(SHARED) -Ilualib-src/sproto $^ -o $@ 

$(LUA_CLIB_PATH)/lpeg.so : 3rd/lpeg/lpcap.cc 3rd/lpeg/lpcode.cc 3rd/lpeg/lpprint.cc 3rd/lpeg/lptree.cc 3rd/lpeg/lpvm.cc | $(LUA_CLIB_PATH)
	@echo -e "\033[36m lpeg.so \033[0m"
	$(CC) $(CFLAGS) $(SHARED) -I3rd/lpeg $^ -o $@ 

clean :
	@echo -e "\033[31m before clean \033[0m"
	rm -f $(SNOWNET_BUILD_PATH)/snownet $(CSERVICE_PATH)/*.so $(LUA_CLIB_PATH)/*.so
	@echo -e "\033[31m after  clean \033[0m"

cleanall: clean
#ifneq (,$(wildcard 3rd/jemalloc/Makefile))
	#cd 3rd/jemalloc && $(MAKE) clean && rm Makefile
#endif
	#cd 3rd/lua && $(MAKE) clean
	#rm -f $(LUA_STATICLIB)
