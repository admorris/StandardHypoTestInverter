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
ROOTLIBFLAGS   = -L$(ROOTLIBDIR) -Wl,--as-needed $(ROOTLIBS) $(EXTRA_ROOTLIBS) -Wl,-rpath,$(ROOTLIBDIR)
# ExtraRooFit flags
ERFDIR      = ExtraRooFit
ERFCXXFLAGS = $(shell make -sC $(ERFDIR) cflags)
ERFLIBS     = $(shell make -sC $(ERFDIR) libs)
ERFLIBDIR   = $(shell make -sC $(ERFDIR) libdir)
ERFLIBFLAGS = -L$(ERFLIBDIR) -Wl,--no-as-needed $(ERFLIBS) -Wl,-rpath,$(ERFLIBDIR)
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
LIBFLAGS   = -Wl,--no-undefined -Wl,--no-allow-shlib-undefined $(ROOTLIBFLAGS) $(ERFLIBFLAGS) -lboost_program_options
# Rules
.PHONY : all clean $(ERFDIR)
# Default
all : bin/$(MAIN)
# Compile ExtraRooFit
$(ERFDIR) :
	make -C $@
# Build binary. In principle you can call `make bin/whatever` to rename the output binary
$(BINDIR)/% : $(OBJS) | $(BINDIR) $(ERFDIR)
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
	make -C $(ERFDIR) clean

