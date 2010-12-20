############################
# Makefile for MSYS + MinGW
############################
# GNU C++ Compiler
CC=g++

SRCDIR=src
BUILD=build

# Superuser command
SUDO=

# Remove command
RM=rm -f

# -mconsole: Create a console application
# -mwindows: Create a GUI application
# -Wl,--enable-auto-import: Let the ld.exe linker automatically import from libraries
LDFLAGS=-mwindows -Wl,--enable-auto-import

#Minimum Windows version: Windows XP, IE 6.01
CPPFLAGS=-D_WIN32_WINNT=0x0500 -DWINVER=0x0500 -D_WIN32_IE=0x0601 $(MOREFLAGS)

#SRC files in SRCDIR directory
SRC=$(addprefix $(SRCDIR)/, $(SRCFILES))
HPP=$(addprefix $(SRCDIR)/, $(HEADERS))

# Choose object file names from source file names
OBJFILES=$(SRCFILES:.cpp=.o)
OBJ=$(addprefix $(BUILD)/, $(OBJFILES))
BBIN=$(addprefix $(BUILD)/, $(BIN))

# Debug, or optimize
ifeq ($(DEBUG),on)
  CFLAGS=-Wall -Wextra -O0 -fno-omit-frame-pointer -ggdb3 -pg -DDEBUG
else
  # All warnings, optimization level 3
  CFLAGS=-Wall -Wextra -O3
endif

#
.PHONY: rebuild

# Default target of make is "all"
.all: all
all: $(BBIN) $(LIBBIN)

# Build object files with chosen options
$(BUILD)/%.o: $(SRCDIR)/%.cpp $(SRCDIR)/%.hpp
	$(CC) $(CFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ -c $<

# Build library
$(BBIN): $(OBJ)
	ar r $@ $^
	ranlib $@

#Build again, don't care why
rebuild: $(OBJ)
	ar r $(BBIN) $^
	ranlib $(BBIN)

#Create install directories if needed
$(INSTALL_LIB): 
	@[ -d $@ ] || $(SUDO) mkdir -p $@
	
$(INSTALL_INC):
	@[ -d $@ ] || $(SUDO) mkdir -p $@

UNINSTALL_HPP = $(addprefix $(INSTALL_INC)/, $(HEADERS))

install: $(INSTALL_LIB) $(INSTALL_INC)
	$(SUDO) cp -p $(BBIN) $(INSTALL_LIB)/
	$(SUDO) cp -p $(HPP) $(INSTALL_INC)/

#How to uninstall
uninstall:
	-$(RM) $(INSTALL_LIB)/$(BIN)
	-$(RM) $(UNINSTALL_HPP)

# Remove object files and core files with "clean" (- prevents errors from exiting)
.clean: clean
clean:
	-$(RM) $(BBIN) $(OBJ) core $(LOGFILES)
