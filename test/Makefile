# Test MineCraft client for libmc--c.

BIN         = bin/mc--c.exe
SRCFILES    = UserInterface.cpp main.cpp
LIBS        = -L$(HOME)/lib -lnet-- -lwsock32 -lmc--c -lopengl32 -lglu32 -lDevIL -lILU \
-lsfml-system -lsfml-window -lsfml-graphics -lz

INCLUDES    = -I$(HOME)/include
LOGFILES    = network.log
###DEBUG       = on
MOREFLAGS   = -std=c++0x

#Where to install or uninstall
INSTALL_BIN = $(HOME)/bin

#Generic MinGW build rules for a binary
include bin.MinGW.mak
