#
# OMNeT++/OMNEST Makefile for mixim
#
# This file was generated with the command:
#  opp_makemake -f --nolink -O out -d Mac80211 -d ieee80211p -d multiConnectionManager -d annotations -d CSMAMac -d bmac -d obstacles -d lmac -d BaseMoBANNetwork -d veins -d ieee802154a -d traci_launchd -d baseNetwork -d WSNRouting -d ieee802154Narrow -d twoRayInterference -d HoHuT -d analogueModels -I/usr/include/libxml2 -L../out/$(CONFIGNAME)/tests/power/utils -L../out/$(CONFIGNAME)/tests/testUtils -L../out/$(CONFIGNAME)/src/base -L../out/$(CONFIGNAME)/src/modules
#

# C++ include paths (with -I)
INCLUDE_PATH = -I/usr/include/libxml2 -I.

# Output directory
PROJECT_OUTPUT_DIR = ../out
PROJECTRELATIVE_PATH = examples
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
# <<<
#------------------------------------------------------------------------------

# Main target

all: $(OBJS) submakedirs Makefile
	@# Do nothing

submakedirs:  Mac80211_dir ieee80211p_dir multiConnectionManager_dir annotations_dir CSMAMac_dir bmac_dir obstacles_dir lmac_dir BaseMoBANNetwork_dir veins_dir ieee802154a_dir traci_launchd_dir baseNetwork_dir WSNRouting_dir ieee802154Narrow_dir twoRayInterference_dir HoHuT_dir analogueModels_dir

.PHONY: all clean cleanall depend msgheaders  Mac80211 ieee80211p multiConnectionManager annotations CSMAMac bmac obstacles lmac BaseMoBANNetwork veins ieee802154a traci_launchd baseNetwork WSNRouting ieee802154Narrow twoRayInterference HoHuT analogueModels
Mac80211: Mac80211_dir
ieee80211p: ieee80211p_dir
multiConnectionManager: multiConnectionManager_dir
annotations: annotations_dir
CSMAMac: CSMAMac_dir
bmac: bmac_dir
obstacles: obstacles_dir
lmac: lmac_dir
BaseMoBANNetwork: BaseMoBANNetwork_dir
veins: veins_dir
ieee802154a: ieee802154a_dir
traci_launchd: traci_launchd_dir
baseNetwork: baseNetwork_dir
WSNRouting: WSNRouting_dir
ieee802154Narrow: ieee802154Narrow_dir
twoRayInterference: twoRayInterference_dir
HoHuT: HoHuT_dir
analogueModels: analogueModels_dir

Mac80211_dir:
	cd Mac80211 && $(MAKE) all

ieee80211p_dir:
	cd ieee80211p && $(MAKE) all

multiConnectionManager_dir:
	cd multiConnectionManager && $(MAKE) all

annotations_dir:
	cd annotations && $(MAKE) all

CSMAMac_dir:
	cd CSMAMac && $(MAKE) all

bmac_dir:
	cd bmac && $(MAKE) all

obstacles_dir:
	cd obstacles && $(MAKE) all

lmac_dir:
	cd lmac && $(MAKE) all

BaseMoBANNetwork_dir:
	cd BaseMoBANNetwork && $(MAKE) all

veins_dir:
	cd veins && $(MAKE) all

ieee802154a_dir:
	cd ieee802154a && $(MAKE) all

traci_launchd_dir:
	cd traci_launchd && $(MAKE) all

baseNetwork_dir:
	cd baseNetwork && $(MAKE) all

WSNRouting_dir:
	cd WSNRouting && $(MAKE) all

ieee802154Narrow_dir:
	cd ieee802154Narrow && $(MAKE) all

twoRayInterference_dir:
	cd twoRayInterference && $(MAKE) all

HoHuT_dir:
	cd HoHuT && $(MAKE) all

analogueModels_dir:
	cd analogueModels && $(MAKE) all

.SUFFIXES: .cc

$O/%.o: %.cc $(COPTS_FILE)
	@$(MKPATH) $(dir $@)
	$(CXX) -c $(COPTS) -o $@ $<

%_m.cc %_m.h: %.msg
	$(MSGC) -s _m.cc $(MSGCOPTS) $?

msgheaders: $(MSGFILES:.msg=_m.h)
	cd Mac80211 && $(MAKE) msgheaders
	cd ieee80211p && $(MAKE) msgheaders
	cd multiConnectionManager && $(MAKE) msgheaders
	cd annotations && $(MAKE) msgheaders
	cd CSMAMac && $(MAKE) msgheaders
	cd bmac && $(MAKE) msgheaders
	cd obstacles && $(MAKE) msgheaders
	cd lmac && $(MAKE) msgheaders
	cd BaseMoBANNetwork && $(MAKE) msgheaders
	cd veins && $(MAKE) msgheaders
	cd ieee802154a && $(MAKE) msgheaders
	cd traci_launchd && $(MAKE) msgheaders
	cd baseNetwork && $(MAKE) msgheaders
	cd WSNRouting && $(MAKE) msgheaders
	cd ieee802154Narrow && $(MAKE) msgheaders
	cd twoRayInterference && $(MAKE) msgheaders
	cd HoHuT && $(MAKE) msgheaders
	cd analogueModels && $(MAKE) msgheaders

clean:
	-rm -rf $O
	-rm -f mixim mixim.exe libmixim.so libmixim.a libmixim.dll libmixim.dylib
	-rm -f ./*_m.cc ./*_m.h

	-cd Mac80211 && $(MAKE) clean
	-cd ieee80211p && $(MAKE) clean
	-cd multiConnectionManager && $(MAKE) clean
	-cd annotations && $(MAKE) clean
	-cd CSMAMac && $(MAKE) clean
	-cd bmac && $(MAKE) clean
	-cd obstacles && $(MAKE) clean
	-cd lmac && $(MAKE) clean
	-cd BaseMoBANNetwork && $(MAKE) clean
	-cd veins && $(MAKE) clean
	-cd ieee802154a && $(MAKE) clean
	-cd traci_launchd && $(MAKE) clean
	-cd baseNetwork && $(MAKE) clean
	-cd WSNRouting && $(MAKE) clean
	-cd ieee802154Narrow && $(MAKE) clean
	-cd twoRayInterference && $(MAKE) clean
	-cd HoHuT && $(MAKE) clean
	-cd analogueModels && $(MAKE) clean

cleanall: clean
	-rm -rf $(PROJECT_OUTPUT_DIR)

depend:
	$(MAKEDEPEND) $(INCLUDE_PATH) -f Makefile -P\$$O/ -- $(MSG_CC_FILES)  ./*.cc
	-cd Mac80211 && if [ -f Makefile ]; then $(MAKE) depend; fi
	-cd ieee80211p && if [ -f Makefile ]; then $(MAKE) depend; fi
	-cd multiConnectionManager && if [ -f Makefile ]; then $(MAKE) depend; fi
	-cd annotations && if [ -f Makefile ]; then $(MAKE) depend; fi
	-cd CSMAMac && if [ -f Makefile ]; then $(MAKE) depend; fi
	-cd bmac && if [ -f Makefile ]; then $(MAKE) depend; fi
	-cd obstacles && if [ -f Makefile ]; then $(MAKE) depend; fi
	-cd lmac && if [ -f Makefile ]; then $(MAKE) depend; fi
	-cd BaseMoBANNetwork && if [ -f Makefile ]; then $(MAKE) depend; fi
	-cd veins && if [ -f Makefile ]; then $(MAKE) depend; fi
	-cd ieee802154a && if [ -f Makefile ]; then $(MAKE) depend; fi
	-cd traci_launchd && if [ -f Makefile ]; then $(MAKE) depend; fi
	-cd baseNetwork && if [ -f Makefile ]; then $(MAKE) depend; fi
	-cd WSNRouting && if [ -f Makefile ]; then $(MAKE) depend; fi
	-cd ieee802154Narrow && if [ -f Makefile ]; then $(MAKE) depend; fi
	-cd twoRayInterference && if [ -f Makefile ]; then $(MAKE) depend; fi
	-cd HoHuT && if [ -f Makefile ]; then $(MAKE) depend; fi
	-cd analogueModels && if [ -f Makefile ]; then $(MAKE) depend; fi

# DO NOT DELETE THIS LINE -- make depend depends on it.

