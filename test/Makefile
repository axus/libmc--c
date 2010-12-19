UNAME := $(shell uname -o)

#Make based on operating system

ifeq ($(UNAME), GNU/Linux)
include Makefile.linux-x64
endif

ifeq ($(UNAME), Msys)
include Makefile.MinGW
endif
