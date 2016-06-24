##############################################################################
# keye Makefile. Requires GNU Make.
# Copyright (C) 2005-2012 Keye Leo.
##############################################################################

NAME=    keye
MAJVER=  1
MINVER=  0
RELVER=  0
VERSION= $(MAJVER).$(MINVER).$(RELVER)

##############################################################################
# Change the installation path as needed. Note: PREFIX must be an absolute path!
#
export PREFIX= bin
#../$(PREFIX)
##############################################################################

##############################################################################
#############################  COMPILER OPTIONS  #############################
##############################################################################
# Make sure you force a full recompile with "make clean",
# followed by "make" if you change any options.
#
# LuaJIT builds as a native 32 or 64 bit binary by default.
CC= gcc
CXX= g++ -std=c++11
#
# Use this if you want to force a 32 bit build on a 64 bit multilib OS.
#CC= gcc -m32
#
# Since the assembler part does NOT maintain a frame pointer, it's pointless
# to slow down the C part by not omitting it. Debugging, tracebacks and
# unwinding are not affected -- the assembler part has frame unwind
# information and GCC emits it where needed (x64) or with -g (see CCDEBUG).
CCOPT= -O0 -fomit-frame-pointer
# Use this if you want to generate a smaller binary (but it's slower):
#CCOPT= -Os -fomit-frame-pointer
# Note: it's no longer recommended to use -O3 with GCC 4.x.
# The I-Cache bloat usually outweighs the benefits from aggressive inlining.
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

STATIC_CC = $(CROSS)$(CC)
DYNAMIC_CC = $(CROSS)$(CC) -fPIC
TARGET_CC= $(STATIC_CC)
TARGET_DYNCC= $(DYNAMIC_CC)
STATIC_CXX = $(CROSS)$(CXX)
DYNAMIC_CXX = $(CROSS)$(CXX) -fPIC
TARGET_CXX= $(STATIC_CXX)
TARGET_DYNCXX= $(DYNAMIC_CXX)
TARGET_LD= $(CROSS)$(CC)
TARGET_AR= $(CROSS)ar rcus
TARGET_STRIP= $(CROSS)strip

TARGET_SONAME= $(NAME)-$(ABIVER).so.$(MAJVER)
TARGET_DYLIBNAME= $(NAME)-$(ABIVER).$(MAJVER).dylib
TARGET_DYLIBPATH= $(or $(PREFIX),/usr/local)/lib/$(TARGET_DYLIBNAME)
TARGET_DLLNAME= $(NAME).dll
TARGET_XSHLDFLAGS= -shared -fPIC -Wl,-soname,$(TARGET_SONAME)
TARGET_DYNXLDOPTS=

SRCS=
INCS=
CFLAGS+= -U_FORTIFY_SOURCE -I./
LDFLAGS+= -L./ -L/usr/lib
LIBS+= -lm -lrt -lstdc++
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

TARGET_T=  $(NAME)
TARGET_A=  lib$(NAME).a
TARGET_SO= lib$(NAME).so
# TARGETS=   $(TARGET_T) $(TARGET_A) $(TARGET_SO)
TARGETS=   $(TARGET_T)

ifeq (Windows,$(TARGET_SYS))
  TARGET_DYNCC= $(STATIC_CC)
  TARGET_T= $(NAME).exe
  ifeq (cygwin,$(HOST_MSYS))
    TARGET_SO= cyg$(TARGET_DLLNAME)
  else
    TARGET_SO= $(TARGET_DLLNAME)
  endif
endif
ifeq (SunOS,$(TARGET_SYS))
  BUILDMODE= static
endif
ifeq (PS3,$(TARGET_SYS))
  BUILDMODE= static
endif

ifeq (Windows,$(HOST_SYS))
  ifeq (,$(HOST_MSYS))
    ALL_RM:= $(subst /,\,$(ALL_RM))
  endif
endif

ifeq (static,$(BUILDMODE))
  TARGET_DYNCC= @:
  TARGETS= $(TARGET_A)
else
ifeq (dynamic,$(BUILDMODE))
#  ifneq (Windows,$(TARGET_SYS))
    TARGET_CC= $(DYNAMIC_CC)
    TARGET_CXX= $(DYNAMIC_CXX)
#  endif
  TARGET_DYNCC= @:
  TARGETS= $(TARGET_SO)
  TARGET_XLDFLAGS+= $(TARGET_DYNXLDOPTS)
else
ifeq (Darwin,$(TARGET_SYS))
  TARGET_DYNCC= @:
endif
ifeq (iOS,$(TARGET_SYS))
  TARGET_DYNCC= @:
endif
endif
endif

Q= @
E= @echo
#Q=
#E= @:

ALL_T= $(TARGET_T) $(TARGET_A) $(TARGET_SO)
ALL_HDRGEN= 
ALL_GEN= $(ALL_HDRGEN)
WIN_RM= *.obj *.lib *.exp *.dll *.exe *.manifest *.pdb *.ilk
ALL_RM= $(ALL_T) $(ALL_GEN) *.o host/*.o $(WIN_RM)
##############################################################################
# Make targets.
##############################################################################

default all:	$(TARGETS)

clean:
	$(HOST_RM) $(ALL_RM)

depend:
	@for file in $(ALL_HDRGEN); do \
	  test -f $$file || touch $$file; \
	  done
	@$(HOST_CC) $(HOST_ACFLAGS) -MM *.c host/*.c | \
	  sed -e "s| [^ ]*/dasm_\S*\.h||g" \
	      -e "s|^\([^l ]\)|host/\1|" >Makefile.dep
	@for file in $(ALL_HDRGEN); do \
	  test -s $$file || $(HOST_RM) $$file; \
	  done

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

.PHONY: default all clean depend

##############################################################################
# Object file rules.
##############################################################################
OBJS=$(patsubst %.cpp,%.o,$(wildcard $(SRCS)))
$(SRCS):$(INCS)

%.o: %.c
	$(E) "CC        $@"
#	$(Q)$(TARGET_DYNCC) $(TARGET_XCFLAGS) -c -o $(@:.o=_dyn.o) $<
	$(Q)$(TARGET_CC) $(TARGET_XCFLAGS) -c -o $@ $<

%.o: %.cpp
	$(E) "CXX       $@"
#	$(Q)$(TARGET_DYNCXX) $(TARGET_XCFLAGS) -c -o $(@:.o=_dyn.o) $<
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
