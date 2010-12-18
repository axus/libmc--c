############################
# Makefile for MSYS + MinGW
############################
# GNU C++ Compiler
CC=g++

SRCDIR=src
BUILD=build

# -mconsole: Create a console application
# -mwindows: Create a GUI application
# -Wl,--enable-auto-import: Let the ld.exe linker automatically import from libraries
LDFLAGS=-mwindows -Wl,--enable-auto-import

#Minimum Windows version: Windows XP, IE 6.01
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
	@[ -d $@ ] || sudo mkdir -p $@
	
$(INSTALL_INC):
	@[ -d $@ ] || sudo mkdir -p $@

UNINSTALL_HPP = $(addprefix $(INSTALL_INC)/, $(HEADERS))

install: $(INSTALL_LIB) $(INSTALL_INC)
	sudo cp $(BBIN) $(INSTALL_LIB)/
	sudo cp $(HPP) $(INSTALL_INC)/

#How to uninstall
uninstall:
	-rm $(INSTALL_LIB)/$(BIN)
	-rm $(UNINSTALL_HPP)

# Remove object files and core files with "clean" (- prevents errors from exiting)
RM=rm -f
.clean: clean
clean:
	-$(RM) $(BBIN) $(OBJ) core $(LOGFILES)