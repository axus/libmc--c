# Minecraft client library functions

BIN         = libmc--c.a
SRCFILES    = Chunk.cpp World.cpp Viewer.cpp
HEADERS     = Chunk.h World.h Viewer.h
LIBS        = -L$(HOME)/lib -lopengl32 -lglu32 -lDevIL -lILU -lsfml-system -lsfml-window -lsfml-graphics
INCLUDES    = -I$(HOME)/include
###DEBUG       = on
MOREFLAGS   =

#Generic MinGW build rules for a library
include lib.MinGW.mak

#How to install
INSTALL_LIB = $(HOME)/lib
INSTALL_INC = $(HOME)/include/mc--
UNINSTALL_HPP = $(addprefix $(INSTALL_INC)/, $(HEADERS))

#Create install directories if needed
$(INSTALL_LIB): 
	@[ -d $@ ] || mkdir -p $@
	
$(INSTALL_INC):
	@[ -d $@ ] || mkdir -p $@

install: $(INSTALL_LIB) $(INSTALL_INC)
	cp $(BBIN) $(INSTALL_LIB)/
	cp $(HPP) $(INSTALL_INC)/

#How to uninstall
uninstall:
	-rm $(INSTALL_LIB)/$(BIN)
	-rm $(UNINSTALL_HPP)
