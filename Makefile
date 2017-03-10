# Compiler and shell
CC         = g++
SHELL      = /bin/bash
RM         = rm -f
# Default name of the program
MAIN       = main
# ROOT flags
ROOTCFLAGS = $(shell root-config --cflags)
ROOTLIBS   = $(shell root-config --libs)
ROOTLIBDIR = $(shell root-config --libdir)
EXTRA_ROOTLIBS = -lRooFit -lRooStats -lRooFitCore
# Extensions
SRCEXT     = cpp
HDREXT     = h
OBJEXT     = o
# Directories
SRCDIR     = src
HDRDIR     = include
OBJDIR     = build
BINDIR     = bin
# Get files and make list of objects and libraries
SRCS      := $(shell find $(SRCDIR) -name '*.$(SRCEXT)')
HDRS      := $(shell find $(HDRDIR) -name '*.$(HDREXT)')
OBJS      := $(patsubst $(SRCDIR)/%.$(SRCEXT), $(OBJDIR)/%.$(OBJEXT), $(SRCS))
# Where the output is
OUTPUT     = $(OBJDIR)/*.$(OBJEXT) $(BINDIR)/*
# Compiler flags
CXXFLAGS   = -Wall -fPIC -I$(HDRDIR) $(ROOTCFLAGS)
LIBFLAGS   = -Wl,--no-undefined -Wl,--no-allow-shlib-undefined $(ROOTLIBS) $(EXTRA_ROOTLIBS) -Wl,-rpath,$(ROOTLIBDIR)
# Rules
.PHONY : all clean
# Default
all : bin/$(MAIN)
# Build binary. In principle you can call `make bin/whatever` to rename the output binary
$(BINDIR)/% : $(OBJS) | $(BINDIR)
	@echo "Linking $@"
	@$(CC) $^ -o $@ $(LIBFLAGS)
# Build objects
$(OBJDIR)/%.$(OBJEXT) : $(SRCDIR)/%.$(SRCEXT) | $(OBJDIR)
	@echo "Compiling $@"
	@$(CC) -c $< -o $@ $(CXXFLAGS)
# Make directories
$(BINDIR) $(OBJDIR) :
	mkdir -p $@
# Remove all the output
clean :
	$(RM) $(OUTPUT)

