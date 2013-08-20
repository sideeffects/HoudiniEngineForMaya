LIBNAME = houdiniEngine

SOSUFFIX = so
SONAME = $(LIBNAME).$(SOSUFFIX)

# external directories
MAYA_DIR = /usr/autodesk/maya2014-x64

# tools
CXX = g++
LD = g++

# Maya flags
CPPFLAGS += -I$(MAYA_DIR)/include
CPPFLAGS += -D_BOOL -DREQUIRE_IOSTREAM
LDLIBS += -L$(MAYA_DIR)/lib -lOpenMaya -lFoundation

# Houdini flags
CPPFLAGS += -I$(HFS)/toolkit/include
LDFLAGS += -Wl,-rpath,$(HFS)/dsolib
LDLIBS += -L$(HFS)/dsolib -lHAPI

# general flags
CXXFLAGS += -m64 -pthread -pipe -fPIC
LDFLAGS += -Wl,-Bsymbolic

# debug/release flags
ifeq ($(MAKETYPE),)
    CXXFLAGS += -g
else ifeq ($(MAKETYPE),Release)
    CXXFLAGS += -O3
endif

CXXFILES = \
	   AssetNode.C \
	   AssetCommand.C \
	   AssetManager.C \
	   Asset.C \
	   Object.C \
	   util.C \
	   GeometryObject.C \
	   InstancerObject.C \
	   AssetNodeMonitor.C \
	   GeometryPart.C \
	   plugin.C

OBJ_DIR = .obj
DEP_DIR = .dep

OBJFILES = $(patsubst %.C, $(OBJ_DIR)/%.o, $(CXXFILES))

DEPFILES = $(patsubst %.C, $(DEP_DIR)/%.d, $(CXXFILES))

# check build requirement
CAN_BUILD := $(and \
	$(realpath $(MAYA_DIR)), \
	1)

.PHONY: all
all:
ifeq ($(CAN_BUILD), 1)
all: $(SONAME)
endif

$(SONAME): $(OBJFILES)
	@mkdir -p $(dir $(@))
	$(LD) -shared $(LDFLAGS) -o $(@) $(OBJFILES) $(LDLIBS)

$(OBJ_DIR)/%.o: %.C
	@mkdir -p $(dir $(@))
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -o $(@) $(<)

$(DEP_DIR)/%.d: %.C
	@echo generating dependency for $(<)
	@mkdir -p $(dir $(@))
	@$(CXX) -MM -MP -MT $(OBJ_DIR)/$(*).o $(CPPFLAGS) $(<) -o $(@)

# if the build requirement isn't met, then we cannot even generate
# dependencies, because that requires header files
ifeq ($(CAN_BUILD), 1)
ifneq ($(MAKECMDGOALS),clean)
-include $(DEPFILES)
endif
endif

.PHONY: clean
clean:
	rm -f $(SONAME)
	rm -f $(OBJFILES)
	rm -f $(DEPFILES)
