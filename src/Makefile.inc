#
# $Id: Makefile.inc,v 1.3 1999-10-12 13:56:06 avn Exp $
#

#
# core version
#
ABI_VERSION = 8

#
# common defines
#
CC		= gcc
LD		= gcc
INCLUDES	= -I ../include -I ..

CFLAGS		= -g -Wall -O -pipe $(INCLUDES)
LFLAGS		= -g -export-dynamic
SO_CFLAGS	= -fPIC
SO_LFLAGS	= -shared
INSTALL_FLAGS	= -c -m 770
LIBS		=
INSTALL_PATH	= ../..
MAKE		= make

#
# common rules
#

OFILES?=$(CFILES:T:.c=.o)

MODULE_EXT=.so
MODULE_NAME?=$(MODULE)$(MODULE_EXT).$(ABI_VERSION)

all: $(MODULE_NAME)

$(MODULE_NAME): $(OFILES)
	$(LD) -o $(MODULE_NAME) $(LFLAGS) $(OFILES) $(LIBS)

depend:
	mkdep $(CFLAGS) $(CFILES)

module_clean:
	rm -f $(MODULE_NAME) $(MODULE)$(MODULE_EXT).*

clean: module_clean
	rm -f $(OFILES) .depend

install: $(MODULE_NAME)
	install $(INSTALL_FLAGS) $(MODULE_NAME) $(INSTALL_PATH)/$(INSTALL_DIR)
