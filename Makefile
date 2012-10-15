
#Make based on operating system

#MacOSX
UNAME := $(shell uname -s)
ifeq ($(UNAME), Darwin)
include Makefile.linux-x64
endif

#Linux, MinGW
UNAME := $(shell uname -o)
ifeq ($(UNAME), GNU/Linux)
include Makefile.linux-x64
endif

ifeq ($(UNAME), Msys)
include Makefile.MinGW
endif
