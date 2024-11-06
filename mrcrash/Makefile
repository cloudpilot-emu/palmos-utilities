CXX = m68k-palmos-g++
LD = m68k-palmos-g++
PILRC = pilrc
XFER = pilot-xfer
BUILDPRC = build-prc

CXXFLAGS = -Os -Wall -std=gnu++98 -Werror $(if $(DEBUG),-DDEBUG,)
LDFLAGS =
PILRCOPT = -q -ro

NAME = mrcrash
SOURCE = main.cxx debug.cxx assembly.cxx
PILRC_SOURCE = mrcrash.rcp
RESOURCE_HEADERS = resource.h

PRC = $(NAME).prc
DEFINITION_FILE = $(NAME).def
BINARY = $(NAME)
OBJECTS = $(SOURCE:.cxx=.o)
DEPEND = $(SOURCE:.cxx=.d)
RESOURCE_DBS = $(PILRC_SOURCE:.rcp=.ro)

all: $(PRC)

install: $(PRC)
	$(XFER) -p net: -i $<

clean:
	-rm -f $(BINARY) $(OBJECTS) $(RESOURCE_DBS) $(PRC) $(DEPEND)

$(PRC): $(BINARY) $(RESOURCE_DBS)
	$(BUILDPRC) $(DEFINITION_FILE) -o $@ $^

$(BINARY): $(OBJECTS)
	$(LD) $(LDFLAGS) $^ -o $@

%.ro : %.rcp $(RESOURCE_HEADERS)
	$(PILRC) $(PILRCOPT) $<

%.o : %.cxx
	$(CXX) $(CXXFLAGS) -MMD -c -o $@ $<

.PHONY: all install clean
.SUFFIXES:

-include $(DEPEND)
