#! /bin/sh

MACFUSE_SRCROOT=${MACFUSE_SRCROOT:-$1}
MACFUSE_SRCROOT=${MACFUSE_SRCROOT:?}

CFLAGS="-D__FreeBSD__=10 -D_POSIX_C_SOURCE=200112L -I$MACFUSE_SRCROOT/common -O -g -arch i386 -arch ppc -arch x86_64 -arch ppc64 -isysroot /Developer/SDKs/MacOSX10.5.sdk" LDFLAGS="-arch i386 -arch ppc -arch x86_64 -arch ppc64 -framework CoreFoundation" ./configure --prefix=/usr/local --disable-dependency-tracking --disable-static
