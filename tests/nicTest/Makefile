#
# OMNeT++/OMNEST Makefile for nicTest
#
# This file was generated with the command:
#  opp_makemake -f -o nicTest -O out -u CMDENV -I../../src/modules -I../../src/modules/analogueModel -I../../src/modules/messages -I../../src/modules/mac -I../../src/modules/utility -I../../src/modules/mac/ieee80211p -I../../src/base/connectionManager -I../../src/modules/obstacle -I../../src/base/messages -I../../src/modules/phy -I../../src/base/modules -I../../src/base/phyLayer -I../../src/base/utils -L../../out/$(CONFIGNAME)/src/base -L../../out/$(CONFIGNAME)/tests/testUtils -L../../out/$(CONFIGNAME)/src/modules -L../../out/$(CONFIGNAME)/tests/power/utils -lmiximbase -lmiximmodules -lmiximtestUtils
#

# Name of target to be created (-o option)
TARGET = nicTest$(EXE_SUFFIX)

# User interface (uncomment one) (-u option)
#USERIF_LIBS = $(ALL_ENV_LIBS) # that is, $(TKENV_LIBS) $(CMDENV_LIBS)
USERIF_LIBS = $(CMDENV_LIBS)
#USERIF_LIBS = $(TKENV_LIBS)

# C++ include paths (with -I)
INCLUDE_PATH = \
    -I../../src/modules \
    -I../../src/modules/analogueModel \
    -I../../src/modules/messages \
    -I../../src/modules/mac \
    -I../../src/modules/utility \
    -I../../src/modules/mac/ieee80211p \
    -I../../src/base/connectionManager \
    -I../../src/modules/obstacle \
    -I../../src/base/messages \
    -I../../src/modules/phy \
    -I../../src/base/modules \
    -I../../src/base/phyLayer \
    -I../../src/base/utils \
    -I.

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS = -L../../out/$(CONFIGNAME)/src/base -L../../out/$(CONFIGNAME)/tests/testUtils -L../../out/$(CONFIGNAME)/src/modules -L../../out/$(CONFIGNAME)/tests/power/utils  -lmiximbase -lmiximmodules -lmiximtestUtils
LIBS += -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/src/base` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/tests/testUtils` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/src/modules` -Wl,-rpath,`abspath ../../out/$(CONFIGNAME)/tests/power/utils`

# Output directory
PROJECT_OUTPUT_DIR = ../../out
PROJECTRELATIVE_PATH = tests/nicTest
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc and .msg files
OBJS = $O/NicTests.o $O/TestApp.o

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
	-rm -f nicTest nicTest.exe libnicTest.so libnicTest.a libnicTest.dll libnicTest.dylib
	-rm -f ./*_m.cc ./*_m.h

cleanall: clean
	-rm -rf $(PROJECT_OUTPUT_DIR)

depend:
	$(MAKEDEPEND) $(INCLUDE_PATH) -f Makefile -P\$$O/ -- $(MSG_CC_FILES)  ./*.cc

# DO NOT DELETE THIS LINE -- make depend depends on it.
$O/NicTests.o: NicTests.cc \
	../../src/base/messages/MacPkt_m.h \
	../../src/base/modules/BaseBattery.h \
	../../src/base/modules/BaseLayer.h \
	../../src/base/modules/BaseMacLayer.h \
	../../src/base/modules/BaseModule.h \
	../../src/base/modules/BatteryAccess.h \
	../../src/base/phyLayer/Interpolation.h \
	../../src/base/phyLayer/MappingBase.h \
	../../src/base/utils/HostState.h \
	../../src/base/utils/MiXiMDefs.h \
	../../src/base/utils/PassedMessage.h \
	../../src/base/utils/SimpleAddress.h \
	../../src/base/utils/miximkerneldefs.h \
	../../src/modules/mac/Mac80211.h \
	../../src/modules/mac/Mac80211MultiChannel.h \
	../../src/modules/messages/Mac80211Pkt_m.h \
	../../src/modules/utility/Consts80211.h \
	NicTests.h \
	TestApp.h
$O/TestApp.o: TestApp.cc \
	../../src/base/messages/MacPkt_m.h \
	../../src/base/modules/BaseBattery.h \
	../../src/base/modules/BaseLayer.h \
	../../src/base/modules/BaseMacLayer.h \
	../../src/base/modules/BaseModule.h \
	../../src/base/modules/BatteryAccess.h \
	../../src/base/phyLayer/Interpolation.h \
	../../src/base/phyLayer/MappingBase.h \
	../../src/base/utils/FindModule.h \
	../../src/base/utils/HostState.h \
	../../src/base/utils/MiXiMDefs.h \
	../../src/base/utils/NetwToMacControlInfo.h \
	../../src/base/utils/PassedMessage.h \
	../../src/base/utils/SimpleAddress.h \
	../../src/base/utils/miximkerneldefs.h \
	../../src/modules/mac/Mac80211.h \
	../../src/modules/mac/Mac80211MultiChannel.h \
	../../src/modules/messages/Mac80211Pkt_m.h \
	../../src/modules/utility/Consts80211.h \
	TestApp.h

