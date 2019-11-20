include ./Rules.make

TARGETPATH	            := ../Firmware/appfs
TARGET      := audio_msg_mgr
#LIBTARGET   := audio_msg_mgr.a


AUDIO_SDK_PATH			:=
AUDIO_SDK_LIBS			:=
AUDIO_SDK_INCLUDES		:=

DEFINES                 := -D_GLIBCXX_USE_CXX11_ABI=0 -D__STDC_CONSTANT_MACROS

#源码目录
SOURCEDIRLIST 			:= ./Main ./AudioMsgManager ./IotMsg ./Lua ./CJson ./MultiTask ./BaseFunction ./jsoncpp ./AudioSDK ./AlsaManager ./MediaPlayer ./Dlna

CPLUSPLUS_SOURCES 		:= $(foreach SUBDIR,$(SOURCEDIRLIST),$(wildcard $(SUBDIR)/*.cpp))
C_SOURCES				:= $(foreach SUBDIR,$(SOURCEDIRLIST),$(wildcard $(SUBDIR)/*.c))

#忽略文件
_SOURCES        :=

OBJDIR          := objs/app
OBJDIRLIST 	    := $(foreach SUBDIR,$(SOURCEDIRLIST),$(OBJDIR)/$(subst ./,,$(SUBDIR)))

CPLUSPLUS_SOURCES     	:= $(filter-out $(_SOURCES),$(CPLUSPLUS_SOURCES))
C_SOURCES     			:= $(filter-out $(_SOURCES),$(C_SOURCES))

CPLUSPLUS_OBJS        	:= $(patsubst %.cpp,$(OBJDIR)/%.o,$(subst ./,,$(CPLUSPLUS_SOURCES)))
C_OBJS        			:= $(patsubst %.c,$(OBJDIR)/%.o,$(subst ./,,$(C_SOURCES)))

CPLUSPLUS_DEPENDENCE        := $(CPLUSPLUS_OBJS:.o=.cpp.dep)
C_DEPENDENCE                := $(C_OBJS:.o=.c.dep)

FFMPEG_INCDIR	:= -I/data/wuweiming/rk3308_sdk/buildroot/output/firefly_rk3308_release/build/ffmpeg3-3.2.7
INCLUDEDIRS	    := -I./include -I./AudioMsgManager -I./IotMsg -I./Lua -I./CJson -I./MultiTask -I./BaseFunction -I./jsoncpp -I./AudioSDK -I./lua-5.3.5 -I./AlsaManager -I./MediaPlayer $(FFMPEG_INCDIR) -I./Dlna

FFMPEG_LIBS_PATH	:= -L/data/wuweiming/rk3308_sdk/buildroot/output/firefly_rk3308_release/target/usr/lib64
XML_LIBS_PATH	:= -L/data/wuweiming/prj_rk3308/buildroot/output/firefly_rk3308_release/target/usr/lib
LIBS_PATH		:= -L./libs/iflytek -L./libs/lua $(FFMPEG_LIBS_PATH) $(XML_LIBS_PATH)
FFMPEG_LIBS		:= -lavformat -lavcodec -lswscale -lavutil -lswresample
XML_LIBS        := -lixml -lupnp
LIBS            =

COMPILE_FLAGS   = -g -Wall -O0 -fPIC -ffunction-sections -fdata-sections $(DEFINES)
LINK_FLAGS      = -Wl,--gc-sections $(LIBS) $(LIBS_PATH) $(FFMPEG_LIBS) $(XML_LIBS) -fPIC -lpthread -llua -laiui -lcae -lIvw60 -lmsc -lasound -ldl -lm


ifeq ($(RELEASE), 1)
#	LINK_FLAGS 		+= -s
else
#	COMPILE_FLAGS 	+= -g -DDEBUG
#	LINK_FLAGS 		+= -rdynamic
COMPILE_FLAGS       += -D_DEBUG -D_RESAMPLE -D_DEBUG_DLNA
endif

RELEASE		?= 0

#all:prebuild libidb $(TARGET)
all:prebuild $(TARGET)

prebuild:
	$(RM) $(OBJDIR)/Main/Main.o
release:
	make clean
	make RELEASE=1

$(TARGET): $(CPLUSPLUS_OBJS) $(C_OBJS)
	$(CPLUSPLUS_COMPILER) $^ $(LIBTARGET) $(LINK_FLAGS) -o $@
ifeq ($(RELEASE), 1)
	strip $(TARGET)
endif
#	-cp $(TARGET)
	test -d $(TARGETPATH) || mkdir -p $(TARGETPATH)
	cp $(TARGET) $(TARGETPATH)

update_prebuild:
	@make -f Makefile.update prebuild
update: update_prebuild
	@make -f Makefile.update

libidb:
	make -f Makefile.lib RELEASE=$(RELEASE)

$(CPLUSPLUS_OBJS): $(OBJDIR)/%.o: %.cpp
	$(CPLUSPLUS_COMPILER) $(COMPILE_FLAGS) $(INCLUDEDIRS) -c -o $@ $<

$(C_OBJS): $(OBJDIR)/%.o: %.c
	$(C_COMPILER) $(COMPILE_FLAGS) $(INCLUDEDIRS) -c -o $@ $<

clean:
	$(RM) $(CPLUSPLUS_OBJS) $(C_OBJS) $(TARGET) $(LIBTARGET) $(CPLUSPLUS_DEPENDENCE) $(C_DEPENDENCE)
#	make -f Makefile.update clean
#	make -f Makefile.lib USE_IO_REDIRECT=1 clean

#依赖
sinclude $(CPLUSPLUS_DEPENDENCE)
sinclude $(C_DEPENDENCE)
$(CPLUSPLUS_DEPENDENCE): $(OBJDIR)/%.cpp.dep: %.cpp
	mkdir -p $(OBJDIRLIST)
	$(CPLUSPLUS_COMPILER) -MM $(COMPILE_FLAGS) $(INCLUDEDIRS) $< > $@.tmp
	sed 's,\([a-zA-Z0-9_]*\)\.o[ :]*,$(OBJDIR)/$*\.o $@ : ,g' < $@.tmp > $@
	$(RM) $@.tmp

$(C_DEPENDENCE): $(OBJDIR)/%.c.dep: %.c
	mkdir -p $(OBJDIRLIST)
	$(C_COMPILER) -MM $(COMPILE_FLAGS) $(INCLUDEDIRS) $< > $@.tmp
	sed 's,\([a-zA-Z0-9_]*\)\.o[ :]*,$(OBJDIR)/$*\.o $@ : ,g' < $@.tmp > $@
	$(RM) $@.tmp
