include config.mk
DEBUG = y

CFLAGS=-fPIC -g  -Wall -ggdb -std=c++0x
EXCFLAGS:=
EXTERLIBS:=

ifeq ($(CPUARCH),AT91SAM9263)
	CROSS_COMPILE = armv5-linux-
#
else ifeq ($(CPUARCH),AT91SAM9260)
	CROSS_COMPILE = armv5-linux-
#
else ifeq ($(CPUARCH),AT91SAM9G20)
	CROSS_COMPILE = armv5-linux-
#
else ifeq ($(CPUARCH),AT91SAM9G45)
	CROSS_COMPILE = armv5-linux-
#
else ifeq ($(CPUARCH),AT91SAM9X5)
	CROSS_COMPILE = armv5-linux-
#
else ifeq ($(CPUARCH),TIAM335X)
	CROSS_COMPILE = armv7-linux-
#
else ifeq ($(CPUARCH),MPC8315E)
	CROSS_COMPILE = powerpc-mpc83xx-linux-
	CFLAGS += -DCPU_MPC8315E -mcpu=603e
#
else ifeq ($(CPUARCH),LOONGSON1B)
	CROSS_COMPILE = mipsel-openwrt-linux-
endif

CFLAGS += -D$(DEVICETYPE)

CC=$(CROSS_COMPILE)g++
STRIP=$(CROSS_COMPILE)strip
AR=$(CROSS_COMPILE)ar

CFLAGS += $(EXCFLAGS) -I/mnt/hgfs/workspaceforemfuture/projectforcir3/Libs_odm_src  
LDFLAGS += $(EXLDFLAGS) -L/mnt/hgfs/workspaceforemfuture/projectforcir3/Libs_odm_src -lrt -lpthread -lEM_Middleware_Lib -D_GLIBCXX_USE_NANOSLEEP

#LIB_OBJS = _AtCommand.o _CAnolog450.o _DispacherCommand.o _EngineNumber.o _FnStruct.o _GprsSendData.o \
#			_HeadFileForAll.o _IndexInfo.o _InfoPackage.o _InfoToFile.o _RawInfo.o _SaveData.o _SendData.o \
#			_TrainNumber.o BlockingQueue.o CanHandle.o EthernetHandle.o LogInstance.o SerialHandle.o TrainState.o		
#all: newcir
#newcir: main.cpp $(LIB_OBJS)
#	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ main.cpp $(LIB_OBJS) 
#	$(STRIP) $@
#	rm -rf $(LIB_OBJS)
#%.o: %.cpp
#	$(CC) $(CFLAGS) $(LDFLAGS) -c $<

exe=newcir
SOURCES=$(wildcard *.cpp)
global_obj=$(patsubst %.cpp,%.o,$(SOURCES))

CC2=$(CROSS_COMPILE)gcc
SOURCES2=$(wildcard *.c)
global_obj2=$(patsubst %.c,%.o,$(SOURCES2))

all:$(exe)
$(exe):$(global_obj) $(global_obj2)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(exe) $(global_obj) $(global_obj2)
	$(STRIP) $@
$(global_obj): %.o : %.cpp
	$(CC) -c $(CFLAGS) $(LDFLAGS) -o $@ $< 
$(global_obj2): %.o : %.c
	$(CC2) -c $(CFLAGS) $(LDFLAGS) -o $@ $< 	

.PHONY:clean
clean:
	rm -rf *.o
