# Console command(s) for compiling on Cygwin/MinGW:
#   Example: make

MODNAME = mysql
SRCFILES = amx_module.cpp mysql.cpp
CSRCFILES =

EXTRA_LIBS_LINUX = libraries/libmysqlclient.a -lz

EXTRA_INCLUDEDIRS =

EXTRA_FLAGS = -Dstrcmpi=strcasecmp

SDKTOP=../HLSDK
SDKSRC=$(SDKTOP)/multiplayer
METADIR=../metamod/metamod
AMXDIR=amx
SQLDIR=mysql
SRCDIR=.

OBJDIR_LINUX=obj.linux

CC_LINUX=gcc
CXX_LINUX=g++

LIBFILE_LINUX = $(MODNAME)_i386.so

TARGET_LINUX = $(OBJDIR_LINUX)/$(LIBFILE_LINUX)

FILES_ALL = *.cpp *.h [A-Z]* *.rc
OBJ_LINUX := $(SRCFILES:%.cpp=$(OBJDIR_LINUX)/%.o)
OBJC_LINUX := $(CSRCFILES:%.c=$(OBJDIR_LINUX)/%.o)

# optimisation level; overridden for certain problematic files
CCO = -O2 -fomit-frame-pointer -funroll-loops -fno-strict-aliasing -pipe

CCOPT = $(CCO) -march=i386 -m32 -Wno-write-strings -fno-exceptions -fno-rtti -DNDEBUG -DJITORNOT

INCLUDEDIRS=-I../curl/include -I$(SQLDIR) -I$(SRCDIR) -I$(AMXDIR) -I$(METADIR) -I$(SDKSRC)/engine -I$(SDKSRC)/common -I$(SDKSRC)/pm_shared -I$(SDKSRC)/dlls -I$(SDKSRC) $(EXTRA_INCLUDEDIRS)

CFLAGS=-Wall -Wno-unknown-pragmas
ODEF=-DOPT_TYPE=\"optimized\"
CFLAGS:=$(CCOPT) $(CFLAGS) $(ODEF) $(EXTRA_FLAGS)

DO_CC_LINUX=$(CC_LINUX) $(CFLAGS) -fPIC $(INCLUDEDIRS) -DLINUX -o $@ -c $<
DO_CXX_LINUX=$(CXX_LINUX) $(CFLAGS) -fPIC $(INCLUDEDIRS) -DLINUX -o $@ -c $<

LINK_LINUX=$(CC_LINUX) $(CFLAGS) -shared -ldl -lm -static-libgcc $(OBJ_LINUX) $(OBJC_LINUX) $(EXTRA_LIBS_LINUX) -s -o $@ 

$(OBJDIR_LINUX)/%.o: $(SRCDIR)/%.c
	$(DO_CC_LINUX)

$(OBJDIR_LINUX)/%.o: $(SRCDIR)/%.cpp
	$(DO_CXX_LINUX)

default: $(TARGET_LINUX)

$(TARGET_LINUX): $(OBJDIR_LINUX) $(OBJ_LINUX) $(OBJC_LINUX)
	$(LINK_LINUX)

$(OBJDIR_LINUX):
	mkdir $@

clean:
	rm -rf $(OBJDIR_LINUX)
