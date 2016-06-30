##############################################################################
# keye Makefile. Requires GNU Make.
# Copyright (C) 2005-2012 Keye Leo.
##############################################################################
NAME=    keye
export PREFIX= bin

CC= gcc
CXX= g++ -std=c++11
#
##############################################################################
################################  BUILD MODE  ################################
##############################################################################
# The default build mode is execute mode,others are static or dynamic
BUILDMODE= execute
#
##############################################################################
# Flags and options for host and target.
##############################################################################
Q= @
E= @echo
HOST_CC= $(CC)
HOST_CXX= $(CXX)
HOST_RM= rm -f

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

SRCS=
INCS=
CFLAGS= -U_FORTIFY_SOURCE -I./
LDFLAGS= -L./ -L/usr/lib
LIBS= -lm -lstdc++
CCDEBUG=
CCOPT= -fomit-frame-pointer
CCWARN= -Wall

# Override: NAME,BUILDMODE,CFLAGS,LDFLAGS,LIBS,SRCS,INCS,CCDEBUG,CCOPT
include Makefile.dep

CCOPTIONS= $(CCDEBUG) $(CCOPT) $(CCWARN) $(CFLAGS)
LDOPTIONS= $(CCDEBUG) $(LDFLAGS)

TARGET_XCFLAGS= $(CCOPTIONS) $(TARGET_FLAGS) $(TARGET_CFLAGS)
TARGET_XLDFLAGS= $(LDOPTIONS) $(TARGET_FLAGS) $(TARGET_LDFLAGS)
TARGET_XSHLDFLAGS= $(LDOPTIONS) $(TARGET_FLAGS) $(TARGET_SHLDFLAGS)
TARGET_XSHLDFLAGS+= -shared -fPIC -Wl,-soname,$(TARGET_SO)
TARGET_XLIBS= $(LIBS)

##############################################################################
# System detection.
##############################################################################

HOST_SYS:= $(shell uname -s)
TARGET_SYS?= $(HOST_SYS)
ifneq (,$(findstring stack-protector,$(shell $(TARGET_CC) -dumpspecs)))
  TARGET_XCFLAGS+= -fno-stack-protector
endif
ifeq (Linux,$(TARGET_SYS))
  TARGET_XLIBS+= -ldl
endif
ifeq (GNU/kFreeBSD,$(TARGET_SYS))
  TARGET_XLIBS+= -ldl
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
endif
endif

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
	$(E)
	$(E) NAME=$(NAME)
	$(E) TARGET_SYS=$(TARGET_SYS)
	$(E) BUILDMODE=$(BUILDMODE)
	$(E) TARGETS=$(TARGETS)
	$(E) TARGET_CC=$(TARGET_CC)
	$(E) TARGET_CXX=$(TARGET_CXX)
	$(E)
	$(E) TARGET_XCFLAGS=$(TARGET_XCFLAGS)
	$(E) TARGET_XLDFLAGS=$(TARGET_XLDFLAGS)
	$(E) TARGET_XSHLDFLAGS=$(TARGET_XSHLDFLAGS)
	$(E) TARGET_XLIBS=$(TARGET_XLIBS)
	$(E)
	$(E) SRCS=$(SRCS)
	$(E) OBJS=$(OBJS)

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
