#
# $Id: Makefile.inc,v 1.6 2000-01-06 04:48:27 fjoe Exp $
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
LDFLAGS		= -g -export-dynamic
SO_CFLAGS	= -fPIC
SO_LDFLAGS	= -shared
INSTALL_FLAGS	= -c -m 770
LIBS		=
INSTALL_PATH	= ../..
MAKE		= make

#
# common rules
#

OFILES?=$(CFILES:T:.c=.o)
OFILES+=$(OFILES_ADD)

MODULE_EXT=.so
MODULE_NAME?=$(MODULE)$(MODULE_EXT).$(ABI_VERSION)

all: $(MODULE_NAME)

$(MODULE_NAME): $(OFILES)
	$(LD) -o $(MODULE_NAME) $(LDFLAGS) $(OFILES) $(LIBS)

depend:
	env MKDEP_CPP_OPTS=-MM mkdep $(CFLAGS) $(CFILES)

module_clean:
	rm -f $(MODULE_NAME) $(MODULE)$(MODULE_EXT).*

clean: module_clean
	rm -f $(OFILES) .depend $(CLEAN_FILES)

install: $(MODULE_NAME)
	install $(INSTALL_FLAGS) $(MODULE_NAME) $(INSTALL_PATH)/$(INSTALL_DIR)

