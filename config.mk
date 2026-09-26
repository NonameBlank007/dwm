# dwm version
VERSION = 6.8

# Customize below to fit your system

# paths
LOCALPREFIX = /usr/local
MANPREFIX = ${LOCALPREFIX}/share/man
SHAREPREFIX = /usr/share

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

# Xinerama, comment if you don't want it
XINERAMALIBS  = -lXinerama
XINERAMAFLAGS = -DXINERAMA

# freetype
FREETYPELIBS = -lfontconfig -lXft
FREETYPEINC = /usr/include/freetype2
# OpenBSD (uncomment)
#FREETYPEINC = ${X11INC}/freetype2
#MANPREFIX = ${PREFIX}/man
#KVMLIB = -lkvm

# includes and libs
INCS = -I${X11INC} -I${FREETYPEINC}
LIBS = -lX11 ${XINERAMALIBS} ${FREETYPELIBS} -lXrender -lX11-xcb -lxcb -lxcb-res ${KVMLIB}

# optimisation options
OPTIMISATIONS ?= -O2 -fno-plt
NATIVE_OPTIMISATIONS ?= -O3 -march=native -mtune=native -flto=auto -fno-plt

# flags
CPPFLAGS += -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE=700L -DVERSION=\"${VERSION}\" ${XINERAMAFLAGS}
#CFLAGS   = -g -std=c99 -pedantic -Wall -O0 ${INCS} ${CPPFLAGS}
CFLAGS  ?= -std=c99 -pedantic -Wall -Wno-deprecated-declarations ${OPTIMISATIONS} ${INCS} ${CPPFLAGS}
LDFLAGS ?= -L${X11LIB}
LDLIBS  += ${LIBS}

# Solaris
#CFLAGS = -fast ${INCS} -DVERSION=\"${VERSION}\"
#LDFLAGS = ${LIBS}

# compiler and linker
CC ?= cc
