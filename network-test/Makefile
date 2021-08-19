CC = m68k-palmos-gcc
CFLAGS = -Wall -std=gnu9x -Werror $(if $(DEBUG),-DDEBUG,)

PILRC = pilrc
PILRCOPT = -q -ro

XFER = pilot-xfer

BUILDPRC = build-prc

NAME = nettest
SOURCE = main.c debug.c
PILRC_SOURCE = nettest.rcp
RESOURCE_HEADERS = resids.h

PRC = $(NAME).prc
DEFINITION_FILE = $(NAME).def
BINARY = $(NAME)
OBJECTS = $(SOURCE:.c=.o)
RESOURCE_DBS = $(PILRC_SOURCE:.rcp=.ro)

all: $(PRC)

install: $(PRC)
	$(XFER) -p net: -i $<

clean:
	-rm -f $(BINARY) $(OBJECTS) $(RESOURCE_DBS) $(PRC)

$(PRC): $(BINARY) $(RESOURCE_DBS)
	$(BUILDPRC) $(DEFINITION_FILE) -o $@ $^

$(BINARY): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

$(RESOURCE_DBS) : %.ro : %.rcp $(RESOURCE_HEADERS)
	$(PILRC) $(PILRCOPT) $<

$(OBJECTS) : %.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<
