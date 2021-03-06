#
# OMNeT++/OMNEST Makefile for ieee802154Narrow
#
# This file was generated with the command:
#  opp_makemake -f -o ieee802154Narrow -O out -I/usr/include/libxml2 -I../../src/base/messages -I../../src/modules/utility -I../../src/modules/phy -I../../src/modules -I../../src/base/connectionManager -I../../src/modules/messages -I../../src/modules/analogueModel -I../../src/modules/obstacle -I../../src/base/phyLayer -I../../src/base/modules -I../../src/modules/mac/ieee80211p -I../../src/base/utils -L../../out/$(CONFIGNAME)/tests/power/utils -L../../out/$(CONFIGNAME)/tests/testUtils -L../../out/$(CONFIGNAME)/src/base -L../../out/$(CONFIGNAME)/src/modules -lmiximbase -lmiximmodules
#

# Name of target to be created (-o option)
TARGET = ieee802154Narrow$(EXE_SUFFIX)

# User interface (uncomment one) (-u option)
USERIF_LIBS = $(ALL_ENV_LIBS) # that is, $(TKENV_LIBS) $(CMDENV_LIBS)
#USERIF_LIBS = $(CMDENV_LIBS)
#USERIF_LIBS = $(TKENV_LIBS)

# C++ include paths (with -I)
INCLUDE_PATH = \
    -I/usr/include/libxml2 \
    -I../../src/base/messages \
    -I../../src/modules/utility \
    -I../../src/modules/phy \
    -I../../src/modules \
    -I../../src/base/connectionManager \
    -I../../src/modules/messages \
    -I../../src/modules/analogueModel \
    -I../../src/modules/obstacle \
    -I../../src/base/phyLayer \
    -I../../src/base/modules \
    -I../../src/modules/mac/ieee80211p \
    -I../../src/base/utils \
    -I.

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS = -L../../out/$(CONFIGNAME)/tests/power/utils -L../../out/$(CONFIGNAME)/tests/testUtils -L../../out/$(CONFIGNAME)/src/base -L../../out/$(CONFIGNAME)/src/modules  -lmiximbase -lmiximmodules
LIBS += -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/tests/power/utils` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/tests/testUtils` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/src/base` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/src/modules`

# Output directory
PROJECT_OUTPUT_DIR = ../../out
PROJECTRELATIVE_PATH = examples/ieee802154Narrow
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc and .msg files
OBJS = $O/WorldUtilityStats.o $O/NetworkStackTrafficGen.o

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

# Simulation kernel and user interface libraries
OMNETPP_LIB_SUBDIR = $(OMNETPP_LIB_DIR)/$(TOOLCHAIN_NAME)
OMNETPP_LIBS = -L"$(OMNETPP_LIB_SUBDIR)" -L"$(OMNETPP_LIB_DIR)" -loppmain$D $(USERIF_LIBS) $(KERNEL_LIBS) $(SYS_LIBS)

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
all: $O/$(TARGET)
	$(LN) $O/$(TARGET) .

$O/$(TARGET): $(OBJS)  $(wildcard $(EXTRA_OBJS)) Makefile
	@$(MKPATH) $O
	$(CXX) $(LDFLAGS) -o $O/$(TARGET)  $(OBJS) $(EXTRA_OBJS) $(AS_NEEDED_OFF) $(WHOLE_ARCHIVE_ON) $(LIBS) $(WHOLE_ARCHIVE_OFF) $(OMNETPP_LIBS)

.PHONY: all clean cleanall depend msgheaders

.SUFFIXES: .cc

$O/%.o: %.cc $(COPTS_FILE)
	@$(MKPATH) $(dir $@)
	$(CXX) -c $(COPTS) -o $@ $<

%_m.cc %_m.h: %.msg
	$(MSGC) -s _m.cc $(MSGCOPTS) $?

msgheaders: $(MSGFILES:.msg=_m.h)

clean:
	-rm -rf $O
	-rm -f ieee802154Narrow ieee802154Narrow.exe libieee802154Narrow.so libieee802154Narrow.a libieee802154Narrow.dll libieee802154Narrow.dylib
	-rm -f ./*_m.cc ./*_m.h

cleanall: clean
	-rm -rf $(PROJECT_OUTPUT_DIR)

depend:
	$(MAKEDEPEND) $(INCLUDE_PATH) -f Makefile -P\$$O/ -- $(MSG_CC_FILES)  ./*.cc

# DO NOT DELETE THIS LINE -- make depend depends on it.
$O/NetworkStackTrafficGen.o: NetworkStackTrafficGen.cc \
	NetworkStackTrafficGen.h \
	../../src/base/messages/NetwPkt_m.h \
	../../src/base/modules/AddressingInterface.h \
	../../src/base/modules/ArpInterface.h \
	../../src/base/modules/BaseArp.h \
	../../src/base/modules/BaseBattery.h \
	../../src/base/modules/BaseLayer.h \
	../../src/base/modules/BaseMacLayer.h \
	../../src/base/modules/BaseModule.h \
	../../src/base/modules/BaseWorldUtility.h \
	../../src/base/modules/BatteryAccess.h \
	../../src/base/phyLayer/Interpolation.h \
	../../src/base/phyLayer/MappingBase.h \
	../../src/base/utils/Coord.h \
	../../src/base/utils/FWMath.h \
	../../src/base/utils/FindModule.h \
	../../src/base/utils/HostState.h \
	../../src/base/utils/MiXiMDefs.h \
	../../src/base/utils/NetwToMacControlInfo.h \
	../../src/base/utils/PassedMessage.h \
	../../src/base/utils/SimpleAddress.h \
	../../src/base/utils/miximkerneldefs.h \
	../../src/modules/utility/Packet.h
$O/WorldUtilityStats.o: WorldUtilityStats.cc \
	WorldUtilityStats.h \
	../../src/base/modules/BaseBattery.h \
	../../src/base/modules/BaseLayer.h \
	../../src/base/modules/BaseModule.h \
	../../src/base/modules/BaseWorldUtility.h \
	../../src/base/modules/BatteryAccess.h \
	../../src/base/utils/Coord.h \
	../../src/base/utils/FWMath.h \
	../../src/base/utils/HostState.h \
	../../src/base/utils/MiXiMDefs.h \
	../../src/base/utils/PassedMessage.h \
	../../src/base/utils/SimpleAddress.h \
	../../src/base/utils/miximkerneldefs.h \
	../../src/modules/utility/Packet.h

