CXX = m68k-palmos-g++ -std=gnu++98
CXXFLAGS = -Os -Wall  -Werror $(if $(DEBUG),-DDEBUG,)

LD = m68k-palmos-g++
LDFLAGS =

PILRC = pilrc
PILRCOPT = -q -ro

XFER = pilot-xfer

BUILDPRC = build-prc

NAME = nettest
SOURCE = main.cxx debug.cxx
PILRC_SOURCE = nettest.rcp

PRC = $(NAME).prc
DEFINITION_FILE = $(NAME).def
BINARY = $(NAME)
OBJECTS = $(SOURCE:.cxx=.o)
RESOURCE_DBS = $(PILRC_SOURCE:.rcp=.ro)

all: $(PRC)

install: $(PRC)
	$(XFER) -p net: -i $<

clean:
	-rm -f $(BINARY) $(OBJECTS) $(RESOURCE_DBS) $(PRC)

$(PRC): $(BINARY) $(RESOURCE_DBS)
	$(BUILDPRC) $(DEFINITION_FILE) -o $@ $^

$(BINARY): $(OBJECTS)
	$(LD) $(LDFLAGS) $^ -o $@

$(RESOURCE_DBS) : %.ro : %.rcp
	$(PILRC) $(PILRCOPT) $<

$(OBJECTS) : %.o : %.cxx
	$(CXX) $(CXXFLAGS) -c -o $@ $<

main.o: resource.h debug.h
debug.o: debug.h
$(PILRC_SOURCE): resource.h
