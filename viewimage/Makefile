TOOLCHAINDIR := /dingux
BINPATH    := $(TOOLCHAINDIR)/bin
LIBPATH    := $(TOOLCHAINDIR)/lib

INCLUDES := -I${TOOLCHAINDIR}/mipsel-linux-uclibc/include -I${TOOLCHAINDIR}/mipsel-linux-uclibc/include/SDL

ARCH	:= mipsel-linux-uclibc-
CC		:= ${BINPATH}/$(ARCH)gcc
CXX = ${BINPATH}/$(ARCH)g++
CFLAGS  := -O2 -Wall $(INCLUDES) $(shell $(BINPATH)/sdl-config --cflags)
LDFLAGS := -O2 -Wall $(shell $(BINPATH)/sdl-config --libs) -lSDL_gfx -lSDL_image
CXXFLAGS += $(CFLAGS)
SOURCES := *.o

PROGRAM = main
TARGET	:= viewimage.dge
VERSION	=	1.0

all: $(PROGRAM)
	mv $(PROGRAM) $(TARGET)

clean:
	rm -f $(TARGET) *.o *~
