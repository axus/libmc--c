############################
# Makefile for Linux (Red Hat, ...)
############################
# GNU C++ Compiler
CC=g++

SRCDIR=src
BUILD=build

# Superuser command
SUDO=sudo

#No LDFLAGS needed for Linux
LDFLAGS=

#Other flags, if any
CPPFLAGS=$(MOREFLAGS)

#SRC files in SRCDIR directory
SRC=$(addprefix $(SRCDIR)/, $(SRCFILES))
HPP=$(addprefix $(SRCDIR)/, $(HEADERS))

# Choose object file names from source file names
OBJFILES=$(SRCFILES:.cpp=.o)
OBJ=$(addprefix $(BUILD)/, $(OBJFILES))
BBIN=$(addprefix $(BUILD)/, $(BIN))

# Debug, or optimize
ifeq ($(DEBUG),on)
  CFLAGS=-Wall -O1 -g -pg -DDEBUG
else
  # All warnings, optimization level 3
  CFLAGS=-Wall -O3
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
	@[ -d $@ ] || $(SUDO) "mkdir -p $@"
	
$(INSTALL_INC):
	@[ -d $@ ] || $(SUDO) "mkdir -p $@"

UNINSTALL_HPP = $(addprefix $(INSTALL_INC)/, $(HEADERS))

install: $(INSTALL_LIB) $(INSTALL_INC)
	$(SUDO) "cp $(BBIN) $(INSTALL_LIB)/"
	$(SUDO) "cp $(HPP) $(INSTALL_INC)/"

#How to uninstall
uninstall:
	-rm $(INSTALL_LIB)/$(BIN)
	-rm $(UNINSTALL_HPP)

# Remove object files and core files with "clean" (- prevents errors from exiting)
RM=rm -f
.clean: clean
clean:
	-$(RM) $(BBIN) $(OBJ) core $(LOGFILES)
