# Compiler and shell
CC         = g++
SHELL      = /bin/bash
RM         = rm -f

# Name the library
MAIN       = ExtraRooFit

# ROOT
ROOTCFLAGS     = $(shell root-config --cflags)
ROOTLIBS       = $(shell root-config --libs)
ROOTLIBDIR     = $(shell root-config --libdir)
EXTRA_ROOTLIBS = -lRooFit -lRooStats -lRooFitCore
ROOTLIBFLAGS   = -L$(ROOTLIBDIR) -Wl,--as-needed $(ROOTLIBS) $(EXTRA_ROOTLIBS) -Wl,-rpath,$(ROOTLIBDIR)
RC             = rootcling

# Extensions
SRCEXT     = cpp
HDREXT     = h
OBJEXT     = o
LIBEXT     = so

# Directories
SRCDIR     = src
HDRDIR     = include
OBJDIR     = build
LIBDIR     = lib

# Get files and make list of objects and libraries
SRCS      := $(shell find $(SRCDIR) -name '*.$(SRCEXT)')
HDRS      := $(shell find $(HDRDIR) -name '*.$(HDREXT)' | grep -v 'LinkDef') # LinkDef.h must be last, and find can result in a random order
OBJS      := $(patsubst $(SRCDIR)/%.$(SRCEXT),$(OBJDIR)/%.$(OBJEXT),$(SRCS))

# Where the output is
OUTPUT     = $(OBJDIR)/*.$(OBJEXT) $(LIBDIR)/*.$(LIBEXT) $(LIBDIR)/*.pcm

# Compiler flags
CXXFLAGS   = -Wall -Werror -fPIC -I. -I$(HDRDIR) $(ROOTCFLAGS)

# Make the libraries
all : $(LIBDIR)/lib$(MAIN).$(LIBEXT)
# Return flags for compiling with this library
cflags :
	@echo -I$(shell pwd)/$(HDRDIR)
libs :
	@echo -l$(MAIN)
libdir :
	@echo $(shell pwd)/$(LIBDIR)
# Build libraries
$(LIBDIR)/lib%.$(LIBEXT) : $(OBJS) $(OBJDIR)/$(MAIN)_dict.$(OBJEXT) | $(LIBDIR)
	@echo "Making shared object $@"
	@$(CC) -shared $^ -o $@ $(ROOTLIBFLAGS)
$(SRCDIR)/%_dict.$(SRCEXT) : $(HDRS) $(HDRDIR)/LinkDef.h | $(LIBDIR)
	@echo "Generating $@"
	@$(RC) -f $@ -c $^
	@mv $(patsubst %_dict.$(SRCEXT),%_dict_rdict.pcm,$@) $(LIBDIR)
# Build objects
$(OBJDIR)/%.$(OBJEXT) : $(SRCDIR)/%.$(SRCEXT) $(HDRS) | $(OBJDIR)
	@echo "Compiling $@"
	@$(CC) -c $< -o $@ $(CXXFLAGS)
$(OBJDIR) $(LIBDIR) :
	mkdir -p $@
# Remove all the output
clean :
	$(RM) $(OUTPUT)
.PHONY : all clean cflags libs libdir
