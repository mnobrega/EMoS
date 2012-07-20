#
# OMNeT++/OMNEST Makefile for mixim-hohut
#
# This file was generated with the command:
#  opp_makemake -f --nolink -O out -d Nic80211Test -d deviceTest -d deviceTestAccts -d utils -d deviceTestMulti -d deviceTestAll -L../../out/$(CONFIGNAME)/src/base -L../../out/$(CONFIGNAME)/tests/testUtils -L../../out/$(CONFIGNAME)/src/modules -L../../out/$(CONFIGNAME)/tests/power/utils
#

# C++ include paths (with -I)
INCLUDE_PATH = -I.

# Output directory
PROJECT_OUTPUT_DIR = ../../out
PROJECTRELATIVE_PATH = tests/power
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
# preserve default target
all: $(TARGET)

deviceTest_dir deviceTestAccts_dir deviceTestAll_dir deviceTestMulti_dir Nic80211Test_dir: utils_dir

.PHONY: utils

# <<<
#------------------------------------------------------------------------------

# Main target

all: $(OBJS) submakedirs Makefile
	@# Do nothing

submakedirs:  Nic80211Test_dir deviceTest_dir deviceTestAccts_dir utils_dir deviceTestMulti_dir deviceTestAll_dir

.PHONY: all clean cleanall depend msgheaders  Nic80211Test deviceTest deviceTestAccts utils deviceTestMulti deviceTestAll
Nic80211Test: Nic80211Test_dir
deviceTest: deviceTest_dir
deviceTestAccts: deviceTestAccts_dir
utils: utils_dir
deviceTestMulti: deviceTestMulti_dir
deviceTestAll: deviceTestAll_dir

Nic80211Test_dir:
	cd Nic80211Test && $(MAKE) all

deviceTest_dir:
	cd deviceTest && $(MAKE) all

deviceTestAccts_dir:
	cd deviceTestAccts && $(MAKE) all

utils_dir:
	cd utils && $(MAKE) all

deviceTestMulti_dir:
	cd deviceTestMulti && $(MAKE) all

deviceTestAll_dir:
	cd deviceTestAll && $(MAKE) all

.SUFFIXES: .cc

$O/%.o: %.cc $(COPTS_FILE)
	@$(MKPATH) $(dir $@)
	$(CXX) -c $(COPTS) -o $@ $<

%_m.cc %_m.h: %.msg
	$(MSGC) -s _m.cc $(MSGCOPTS) $?

msgheaders: $(MSGFILES:.msg=_m.h)
	cd Nic80211Test && $(MAKE) msgheaders
	cd deviceTest && $(MAKE) msgheaders
	cd deviceTestAccts && $(MAKE) msgheaders
	cd utils && $(MAKE) msgheaders
	cd deviceTestMulti && $(MAKE) msgheaders
	cd deviceTestAll && $(MAKE) msgheaders

clean:
	-rm -rf $O
	-rm -f mixim-hohut mixim-hohut.exe libmixim-hohut.so libmixim-hohut.a libmixim-hohut.dll libmixim-hohut.dylib
	-rm -f ./*_m.cc ./*_m.h

	-cd Nic80211Test && $(MAKE) clean
	-cd deviceTest && $(MAKE) clean
	-cd deviceTestAccts && $(MAKE) clean
	-cd utils && $(MAKE) clean
	-cd deviceTestMulti && $(MAKE) clean
	-cd deviceTestAll && $(MAKE) clean

cleanall: clean
	-rm -rf $(PROJECT_OUTPUT_DIR)

depend:
	$(MAKEDEPEND) $(INCLUDE_PATH) -f Makefile -P\$$O/ -- $(MSG_CC_FILES)  ./*.cc
	-cd Nic80211Test && if [ -f Makefile ]; then $(MAKE) depend; fi
	-cd deviceTest && if [ -f Makefile ]; then $(MAKE) depend; fi
	-cd deviceTestAccts && if [ -f Makefile ]; then $(MAKE) depend; fi
	-cd utils && if [ -f Makefile ]; then $(MAKE) depend; fi
	-cd deviceTestMulti && if [ -f Makefile ]; then $(MAKE) depend; fi
	-cd deviceTestAll && if [ -f Makefile ]; then $(MAKE) depend; fi

# DO NOT DELETE THIS LINE -- make depend depends on it.

