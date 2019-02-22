PLAT ?= none
PLATS = linux freebsd macosx

CC := g++

#伪目标文件 不判断依赖关系，永远执行
.PHONY : none $(PLATS) clean all cleanall

#ifneq ($(PLAT), none)

.PHONY : default

default :
	$(MAKE) $(PLAT)

#endif

none :
	@echo "Please do 'make PLATFORM' where PLATFORM is one of these:"
	@echo "   $(PLATS)"

SNOWNET_LIBS := -lpthread -lm -Wno-unused-function -Wno-write-strings -Wno-sign-compare
SHARED := -fPIC --shared
EXPORT := -Wl,-E

linux : PLAT = linux
macosx : PLAT = macosx
freebsd : PLAT = freebsd

macosx : SHARED := -fPIC -dynamiclib -Wl,-undefined,dynamic_lookup
macosx : EXPORT :=
macosx linux : SNOWNET_LIBS += -ldl
linux freebsd : SNOWNET_LIBS += -lrt

# Turn off jemalloc and malloc hook on macosx

macosx : MALLOC_STATICLIB :=
macosx : SNOWNET_DEFINES :=-DNOUSE_JEMALLOC

linux macosx freebsd :
	@echo -e "\033[33m before make all \033[0m"
	$(MAKE) all PLAT=$@ SNOWNET_LIBS="$(SNOWNET_LIBS)" SHARED="$(SHARED)" EXPORT="$(EXPORT)" MALLOC_STATICLIB="$(MALLOC_STATICLIB)" SNOWNET_DEFINES="$(SNOWNET_DEFINES)"
	@echo -e "\033[33m after  make all \033[0m"
