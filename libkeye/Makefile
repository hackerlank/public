##############################################################################
# keye Makefile. Requires GNU Make.
# Copyright (C) 2005-2012 Keye Leo.
##############################################################################

NAME=    keye
export PREFIX= bin

CC= gcc
CXX= g++ -std=c++11
#
# Since the assembler part does NOT maintain a frame pointer, it's pointless
# to slow down the C part by not omitting it. Debugging, tracebacks and
# unwinding are not affected -- the assembler part has frame unwind
# information and GCC emits it where needed (x64) or with -g (see CCDEBUG).
CCOPT= -O0 -fomit-frame-pointer
#
CCDEBUG= -g
#
CCWARN= -Wall
# Uncomment the next line to enable more warnings:
#CCWARN+= -Wextra -Wdeclaration-after-statement -Wredundant-decls -Wshadow -Wpointer-arith
#
##############################################################################

##############################################################################
################################  BUILD MODE  ################################
##############################################################################
# The default build mode is execute mode,others are static or dynamic
BUILDMODE= execute
#
##############################################################################
# Flags and options for host and target.
##############################################################################

# You can override the following variables at the make command line:
#   CC       HOST_CC       STATIC_CC       DYNAMIC_CC
#   CXX      HOST_CXX      STATIC_CXX      DYNAMIC_CXX
#   CFLAGS   HOST_CFLAGS   TARGET_CFLAGS
#   CXXFLAGS HOST_CXXFLAGS TARGET_CXXFLAGS
#   LDFLAGS  HOST_LDFLAGS  TARGET_LDFLAGS  TARGET_SHLDFLAGS
#   LIBS     HOST_LIBS     TARGET_LIBS
#   CROSS    HOST_SYS      TARGET_SYS      TARGET_FLAGS
#
# Cross-compilation examples:
#   make HOST_CC="gcc -m32" CROSS=i586-mingw32msvc- TARGET_SYS=Windows
#   make HOST_CC="gcc -m32" CROSS=powerpc-linux-gnu-

CCOPTIONS= $(CCDEBUG) $(CCOPT) $(CCWARN) $(CFLAGS)
LDOPTIONS= $(CCDEBUG) $(LDFLAGS)

HOST_CC= $(CC)
HOST_CXX= $(CXX)
HOST_RM= rm -f

HOST_XCFLAGS= -I.
HOST_XLDFLAGS=
HOST_XLIBS=
HOST_ACFLAGS= $(CCOPTIONS) $(HOST_XCFLAGS) $(TARGET_ARCH) $(HOST_CFLAGS)
HOST_ALDFLAGS= $(LDOPTIONS) $(HOST_XLDFLAGS) $(HOST_LDFLAGS)
HOST_ALIBS= $(HOST_XLIBS) $(LIBS) $(HOST_LIBS)

STATIC_CC = $(CC)
DYNAMIC_CC = $(CC) -fPIC
STATIC_CXX = $(CXX)
DYNAMIC_CXX = $(CXX) -fPIC

TARGET_CC= $(STATIC_CC)
TARGET_CXX= $(STATIC_CXX)
TARGET_LD= $(CC)
TARGET_AR= ar rcus
TARGET_STRIP= strip

TARGET_T=  $(NAME)
TARGET_A=  lib$(NAME).a
TARGET_SO=	lib$(NAME).so
TARGETS=   $(TARGET_T)

#TARGET_DYLIBNAME= $(NAME).$(VERSION).dylib
TARGET_DYLIBNAME= $(NAME).dylib
TARGET_DYLIBPATH= $(or $(PREFIX),/usr/local)/lib/$(TARGET_DYLIBNAME)
TARGET_DLLNAME= $(NAME).dll
TARGET_XSHLDFLAGS= -shared -fPIC -Wl,-soname,$(TARGET_SO)
TARGET_DYNXLDOPTS=

SRCS=
INCS=
CFLAGS+= -U_FORTIFY_SOURCE -I./
LDFLAGS+= -L./ -L/usr/lib
LIBS+= -lm -lstdc++
# Target flags and source option.
include Makefile.dep

TARGET_XCFLAGS= $(CCOPTIONS) $(TARGET_FLAGS) $(TARGET_CFLAGS)
TARGET_XLDFLAGS= $(LDOPTIONS) $(TARGET_FLAGS) $(TARGET_LDFLAGS)
TARGET_XSHLDFLAGS+= $(LDOPTIONS) $(TARGET_FLAGS) $(TARGET_SHLDFLAGS)
TARGET_XLIBS= $(LIBS) $(TARGET_LIBS)

##############################################################################
# System detection.
##############################################################################

ifeq (Windows,$(findstring Windows,$(OS))$(MSYSTEM)$(TERM))
  HOST_SYS= Windows
  HOST_RM= del
else
  HOST_SYS:= $(shell uname -s)
  ifneq (,$(findstring MINGW,$(HOST_SYS)))
    HOST_SYS= Windows
    HOST_MSYS= mingw
  endif
  ifneq (,$(findstring CYGWIN,$(HOST_SYS)))
    HOST_SYS= Windows
    HOST_MSYS= cygwin
  endif
endif

TARGET_SYS?= $(HOST_SYS)
ifeq (Windows,$(TARGET_SYS))
  TARGET_STRIP+= --strip-unneeded
  TARGET_XSHLDFLAGS= -shared
  TARGET_DYNXLDOPTS=
else
ifeq (Darwin,$(TARGET_SYS))
  ifeq (,$(MACOSX_DEPLOYMENT_TARGET))
    export MACOSX_DEPLOYMENT_TARGET=10.4
  endif
  TARGET_STRIP+= -x
  TARGET_AR+= 2>/dev/null
  TARGET_XCFLAGS+= -fno-stack-protector
  TARGET_XSHLDFLAGS= -dynamiclib -single_module -undefined dynamic_lookup -fPIC
  TARGET_DYNXLDOPTS=
  TARGET_XSHLDFLAGS+= -install_name $(TARGET_DYLIBPATH) -compatibility_version $(MAJVER).$(MINVER) -current_version $(MAJVER).$(MINVER).$(RELVER)
  ifeq (x64,$(TARGET_LJARCH))
    TARGET_XLDFLAGS+= -pagezero_size 10000 -image_base 100000000
    TARGET_XSHLDFLAGS+= -image_base 7fff04c4a000
  endif
else
ifeq (iOS,$(TARGET_SYS))
  TARGET_STRIP+= -x
  TARGET_AR+= 2>/dev/null
  TARGET_XCFLAGS+= -fno-stack-protector
  TARGET_XSHLDFLAGS= -dynamiclib -single_module -undefined dynamic_lookup -fPIC
  TARGET_DYNXLDOPTS=
  TARGET_XSHLDFLAGS+= -install_name $(TARGET_DYLIBPATH) -compatibility_version $(MAJVER).$(MINVER) -current_version $(MAJVER).$(MINVER).$(RELVER)
else
  ifneq (,$(findstring stack-protector,$(shell $(TARGET_CC) -dumpspecs)))
    TARGET_XCFLAGS+= -fno-stack-protector
  endif
  ifneq (SunOS,$(TARGET_SYS))
    ifneq (PS3,$(TARGET_SYS))
      TARGET_XLDFLAGS+= -Wl,-E
    endif
  endif
  ifeq (Linux,$(TARGET_SYS))
    TARGET_XLIBS+= -ldl
  endif
  ifeq (GNU/kFreeBSD,$(TARGET_SYS))
    TARGET_XLIBS+= -ldl
  endif
endif
endif
endif

ifneq (,$(CCDEBUG))
  TARGET_STRIP= @:
endif

##############################################################################
# Build mode handling.
##############################################################################
# Mixed mode defaults.

ifeq (static,$(BUILDMODE))
  TARGETS= $(TARGET_A)
else
ifeq (dynamic,$(BUILDMODE))
  TARGET_CC= $(DYNAMIC_CC)
  TARGET_CXX= $(DYNAMIC_CXX)
  TARGETS= $(TARGET_SO)
  TARGET_XLDFLAGS+= $(TARGET_DYNXLDOPTS)
endif
endif

Q= @
E= @echo
#Q=
#E= @:

ALL_T= $(TARGET_T)
ifeq (static,$(BUILDMODE))
  ALL_T= $(TARGET_A)
else
ifeq (dynamic,$(BUILDMODE))
  ALL_T= $(TARGET_SO)
endif
endif
ALL_RM= $(ALL_T) *.o
##############################################################################
# Make targets.
##############################################################################

default all:	$(TARGETS)

clean:
	$(HOST_RM) $(ALL_RM)
	find ./ -name *.o -exec rm -r {} \;

install:
	rm -fr /usr/include/keye
	mkdir /usr/include/keye
	cp -Rf * /usr/include/keye
	find /usr/include/keye/ -name *.hpp -exec rm -f {} \;
	find /usr/include/keye/ -name *.cpp -exec rm -f {} \;
	find /usr/include/keye/ -name *.o -exec rm -f {} \;
	cp $(TARGET_SO) /usr/lib
	ln -s /usr/lib/$(TARGET_SO) /usr/lib/$(TARGET_SO)

echo:
	$(E) TARGET_SYS=$(TARGET_SYS)
	$(E) BUILDMODE=$(BUILDMODE)
	$(E) NAME=$(NAME)
	$(E) SRCS=$(SRCS)
	$(E) OBJS=$(OBJS)

	$(E) TARGET_XCFLAGS=$(TARGET_XCFLAGS)
	$(E) TARGET_XLDFLAGS=$(TARGET_XLDFLAGS)
	$(E) TARGET_XSHLDFLAGS=$(TARGET_XSHLDFLAGS)
	$(E) TARGET_XLIBS=$(TARGET_XLIBS)

	$(E) TARGET_CC=$(TARGET_CC)
	$(E) TARGET_CXX=$(TARGET_CXX)
	$(E) TARGETS=$(TARGETS)

.PHONY: default all clean

##############################################################################
# Object file rules.
##############################################################################
OBJS=$(patsubst %.cpp,%.o,$(wildcard $(SRCS)))
$(SRCS):$(INCS)

%.o: %.c
	$(E) "CC        $@"
	$(Q)$(TARGET_CC) $(TARGET_XCFLAGS) -c -o $@ $<

%.o: %.cpp
	$(E) "CXX       $@"
	$(Q)$(TARGET_CXX) $(TARGET_XCFLAGS) -c -o $@ $<

#$(OBJS):%.o

##############################################################################
# Target file rules.
##############################################################################

$(TARGET_A): $(OBJS)
	$(E) "AR        $@"
	$(Q)$(TARGET_AR) $@ $(OBJS)

# The dependency on _O, but linking with _DYNO is intentional.
$(TARGET_SO): $(OBJS)
	$(E) "DYNLINK   $@"
	$(Q)$(TARGET_LD) $(TARGET_XSHLDFLAGS) -o $@ $(OBJS) $(TARGET_XLIBS)
	$(Q)$(TARGET_STRIP) $@

$(TARGET_T): $(OBJS)
	$(E) "LINK      $@"
	$(Q)$(TARGET_LD) $(TARGET_XLDFLAGS) -o $@ $(OBJS) $(TARGET_XLIBS)
	$(Q)$(TARGET_STRIP) $@
	$(E) "OK        Successfully built $@"

##############################################################################
