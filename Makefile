PREFIX		:= /usr/local
UDEVPREFIX	:= /etc/udev

TARGETS	:= \
	footswitch \
	scythe \
	scythe2 \
	footswitch1p

INCDIR		:= include
SRCDIR		:= src
OBJDIR		:= obj

COMMONSRC	:= \
	common.c \
	debug.c

INSTALL	:= /usr/bin/install -c
INSTALLDATA	:= /usr/bin/install -c -m 644
CFLAGS		:= -Wall -I$(INCDIR)
UNAME		:= $(shell uname)

ifeq ($(UNAME), Darwin)
	CFLAGS	+= -DOSX $(shell pkg-config --cflags hidapi)
	LDLIBS	:= $(shell pkg-config --libs hidapi)
else
	ifeq ($(UNAME), Linux)
		CFLAGS	+= $(shell pkg-config --cflags hidapi-libusb)
		LDLIBS	:= $(shell pkg-config --libs hidapi-libusb)
	else
		LDLIBS	:= -lhidapi
	endif
