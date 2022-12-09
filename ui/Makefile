include ../dpf/Makefile.base.mk

FILES=cairohelper.cpp knob.cpp raisedpanel.cpp textlabel.cpp

DPF_PATH=../dpf

BUILD_DIR=../build/ui
OBJS=$(FILES:%=$(BUILD_DIR)/%.o)
LIBUI=../build/ui/libui.a

BUILD_CXX_FLAGS += -std=c++17

BUILD_CXX_FLAGS += `pkg-config --cflags pangocairo`
BUILD_CXX_FLAGS += `pkg-config --cflags fontconfig`

BUILD_CXX_FLAGS += -I. -I$(DPF_PATH)/distrho -I$(DPF_PATH)/dgl

all: $(LIBUI)

$(LIBUI): $(OBJS)
	@echo "Creating static library for common UI stuff"
	$(SILENT)rm -f $@
	$(SILENT)$(AR) crs $@ $^

$(BUILD_DIR)/%.cpp.o: %.cpp
	-@mkdir -p "$(shell dirname $(BUILD_DIR)/$<)"
	@echo "Compiling $<"
	$(SILENT)$(CXX) $< $(BUILD_CXX_FLAGS) -c -o $@
