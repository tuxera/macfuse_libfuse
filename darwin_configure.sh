#! /bin/sh

MACFUSE_SRCROOT=${MACFUSE_SRCROOT:-$1}
MACFUSE_SRCROOT=${MACFUSE_SRCROOT:?}

CFLAGS="-D__FreeBSD__=10 -D_POSIX_C_SOURCE=200112L -I$MACFUSE_SRCROOT/common -O -g -arch i386 -arch ppc -isysroot /Developer/SDKs/MacOSX10.4u.sdk -mmacosx-version-min=10.4" LDFLAGS="-arch i386 -arch ppc -Wl,-no_uuid -framework CoreFoundation" ./configure --prefix=/usr/local --disable-dependency-tracking
