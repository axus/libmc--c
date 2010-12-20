############################
# Makefile for Linux (Red Hat, ...)
############################
# GNU C++ Compiler
CC=g++

SRCDIR=src
BUILD=build

# Superuser command
SUDO=

# Remove command
RM=rm -f

#   Your makefile that includes bin.MinGW.mak must define these:
# BIN           binary name
# SRCFILES      .cpp source file names
# LIBS          -L(lib path) and -l(library name) for all paths and lib names
# INCLUDES      -I(include path) for all include paths
# INSTALL_BIN   What path to install to for "make install"
#   Optional:
# LOGFILES      names of log files to clean up with make clean
# DEBUG         "on" to turn on debugging
# MOREFLAGS     Add custom flags to object compile phase

#No LDFLAGS needed for Linux
LDFLAGS=

#More flags for Linux
CPPFLAGS=$(MOREFLAGS) -Wno-deprecated

#SRC files in SRCDIR directory
SRC=$(addprefix $(SRCDIR)/, $(SRCFILES))

# Choose object file names from source file names
OBJFILES=$(SRCFILES:.cpp=.o)
OBJ=$(addprefix $(BUILD)/, $(OBJFILES))
BBIN=$(addprefix bin/, $(BIN))

# Debug, or optimize
ifeq ($(DEBUG),on)
  CFLAGS=-Wall -Wextra -O0 -fno-omit-frame-pointer -ggdb3 -pg -DDEBUG
else
  # All warnings, optimization level 3
  CFLAGS=-Wall -Wextra -O3
endif


# Default target of make is "all"
.all: all      
all: $(BUILD) $(BBIN)

#Create build directories if needed
$(BUILD): 
	@[ -d $@ ] || mkdir -p $@

# Build object files with chosen options
$(BUILD)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ -c $<

# Build executable from objects and libraries to current directory
$(BBIN): $(OBJ)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@

#Build again, don't care why
rebuild: 
	$(CC) $(OBJ) $(CFLAGS) $(LDFLAGS) $(LIBS) -o $(BBIN)
	
#Create install directories if needed
$(INSTALL_BIN): 
	@[ -d $@ ] || mkdir -p $@

install: $(INSTALL_BIN)
	cp $(BBIN) $(INSTALL_BIN)/

#How to uninstall
uninstall:
	-$(RM) $(INSTALL_BIN)/$(BIN)

# Remove object files and core files with "clean" (- prevents errors from exiting)
.clean: clean
clean:
	-$(RM) $(BBIN) $(OBJ) core $(LOGFILES)
