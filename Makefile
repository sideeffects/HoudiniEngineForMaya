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
CPPFLAGS += -I..
LDFLAGS += -Wl,-rpath,$(HDSO)
LDLIBS += -L$(HDSO) -lHAPI

# general flags
CXXFLAGS += -m64 -pthread -pipe -fPIC -Wno-deprecated -fno-gnu-keywords
LDFLAGS += -Wl,-Bsymbolic

# debug/release flags
ifeq ($(MAKETYPE),)
    CXXFLAGS += -O0 -g -gstabs+
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

ALL_TARGETS := $(shell if [ -d $(MAYA_DIR) ]; then echo $(SONAME); fi)

.PHONY: all
all: $(ALL_TARGETS)

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

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPFILES)
endif

.PHONY: clean
clean:
	rm -f $(SONAME)
	rm -f $(OBJFILES)
	rm -f $(DEPFILES)
