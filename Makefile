#
# OMNeT++/OMNEST Makefile for mixim-hohut
#
# This file was generated with the command:
#  opp_makemake -f --nolink -O out -d tests -d src/base -d examples -d src/modules -L./out/$(CONFIGNAME)/src/base -L./out/$(CONFIGNAME)/tests/testUtils -L./out/$(CONFIGNAME)/src/modules -L./out/$(CONFIGNAME)/tests/power/utils
#

# C++ include paths (with -I)
INCLUDE_PATH = -I.

# Output directory
PROJECT_OUTPUT_DIR = out
PROJECTRELATIVE_PATH =
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc and .msg files
OBJS =

# Message files
MSGFILES =

#------------------------------------------------------------------------------

# Pull in OMNeT++ configuration (Makefile.inc or configuser.vc)

ifneq ("$(OMNETPP_CONFIGFILE)","")
CONFIGFILE = $(OMNETPP_CONFIGFILE)
else
ifneq ("$(OMNETPP_ROOT)","")
CONFIGFILE = $(OMNETPP_ROOT)/Makefile.inc
else
CONFIGFILE = $(shell opp_configfilepath)
endif
endif

ifeq ("$(wildcard $(CONFIGFILE))","")
$(error Config file '$(CONFIGFILE)' does not exist -- add the OMNeT++ bin directory to the path so that opp_configfilepath can be found, or set the OMNETPP_CONFIGFILE variable to point to Makefile.inc)
endif

include $(CONFIGFILE)

COPTS = $(CFLAGS)  $(INCLUDE_PATH) -I$(OMNETPP_INCL_DIR)
MSGCOPTS = $(INCLUDE_PATH)

# we want to recompile everything if COPTS changes,
# so we store COPTS into $COPTS_FILE and have object
# files depend on it (except when "make depend" was called)
COPTS_FILE = $O/.last-copts
ifneq ($(MAKECMDGOALS),depend)
ifneq ("$(COPTS)","$(shell cat $(COPTS_FILE) 2>/dev/null || echo '')")
$(shell $(MKPATH) "$O" && echo "$(COPTS)" >$(COPTS_FILE))
endif
endif

#------------------------------------------------------------------------------
# User-supplied makefile fragment(s)
# >>>
# inserted from file 'makefrag':
all: Makefile
tests_dir: src__base_dir src__modules_dir
examples_dir: src__base_dir src__modules_dir
src__modules_dir: src__base_dir

# <<<
#------------------------------------------------------------------------------

# Main target

all: $(OBJS) submakedirs Makefile
	@# Do nothing

submakedirs:  tests_dir src__base_dir examples_dir src__modules_dir

.PHONY: all clean cleanall depend msgheaders  tests src__base examples src__modules
tests: tests_dir
src__base: src__base_dir
examples: examples_dir
src__modules: src__modules_dir

tests_dir:
	cd tests && $(MAKE) all

src__base_dir:
	cd src/base && $(MAKE) all

examples_dir:
	cd examples && $(MAKE) all

src__modules_dir:
	cd src/modules && $(MAKE) all

.SUFFIXES: .cc

$O/%.o: %.cc $(COPTS_FILE)
	@$(MKPATH) $(dir $@)
	$(CXX) -c $(COPTS) -o $@ $<

%_m.cc %_m.h: %.msg
	$(MSGC) -s _m.cc $(MSGCOPTS) $?

msgheaders: $(MSGFILES:.msg=_m.h)
	cd tests && $(MAKE) msgheaders
	cd src/base && $(MAKE) msgheaders
	cd examples && $(MAKE) msgheaders
	cd src/modules && $(MAKE) msgheaders

clean:
	-rm -rf $O
	-rm -f mixim-hohut mixim-hohut.exe libmixim-hohut.so libmixim-hohut.a libmixim-hohut.dll libmixim-hohut.dylib
	-rm -f ./*_m.cc ./*_m.h

	-cd tests && $(MAKE) clean
	-cd src/base && $(MAKE) clean
	-cd examples && $(MAKE) clean
	-cd src/modules && $(MAKE) clean

cleanall: clean
	-rm -rf $(PROJECT_OUTPUT_DIR)

depend:
	$(MAKEDEPEND) $(INCLUDE_PATH) -f Makefile -P\$$O/ -- $(MSG_CC_FILES)  ./*.cc
	-cd tests && if [ -f Makefile ]; then $(MAKE) depend; fi
	-cd src/base && if [ -f Makefile ]; then $(MAKE) depend; fi
	-cd examples && if [ -f Makefile ]; then $(MAKE) depend; fi
	-cd src/modules && if [ -f Makefile ]; then $(MAKE) depend; fi

# DO NOT DELETE THIS LINE -- make depend depends on it.

