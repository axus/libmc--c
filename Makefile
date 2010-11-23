# Minecraft client library functions

BIN         = libmc--c.a
SRCFILES    = Chunk.cpp World.cpp Viewer.cpp
HEADERS     = Chunk.h World.h Viewer.h
LIBS        = -L$(HOME)/lib -lopengl32 -lglu32 -lDevIL -lILU -lz
INCLUDES    = -I$(HOME)/include
###DEBUG       = on
MOREFLAGS   =

#I suggest using SFML for OpenGL:  -lsfml-system -lsfml-window -lsfml-graphics

#How to install
INSTALL_LIB = $(HOME)/lib
INSTALL_INC = $(HOME)/include/mc--

#Generic MinGW build rules for a library
include lib.MinGW.mak
