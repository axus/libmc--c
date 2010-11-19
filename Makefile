# Minecraft client library functions

BIN         = libmc--c.exe
SRCFILES    = main.cpp
LIBS        = -L$(HOME)/lib -lopengl32 -lglu32 -lDevIL -lILU -lsfml-system -lsfml-window -lsfml-graphics
INCLUDES    = -I$(HOME)/include
###DEBUG       = on
MOREFLAGS   =

#Generic MinGW build rules for a binary
include bin.MinGW.mak

#How to install
INSTALL_BIN = $(HOME)/bin
install:
	cp $(BIN) $(INSTALL_BIN)/

#How to uninstall
uninstall:
	-rm $(INSTALL_BIN)/$(BIN)
