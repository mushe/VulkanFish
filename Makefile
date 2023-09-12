PROJ_DIR = $(realpath $(CURDIR))
SRC_DIR := $(PROJ_DIR)/Sources
BUILD_DIR := $(PROJ_DIR)/Build
IMGUI_DIR := $(PROJ_DIR)/Libraries/imgui/
STB_IMAGE_DIR := $(PROJ_DIR)/Libraries/stb_image/
CC=clang
CXX=clang++
IMGUI_LIB=libimgui.a
TARGET=$(BUILD_DIR)/vulkanfish
RMDIR = rm -rf 
MKDIR = mkdir -p
CXXFLAGS = -I$(SRC_DIR) -I$(STB_IMAGE_DIR) -I$(IMGUI_DIR)
LDFLAGS = -lglfw -lvulkan -L$(IMGUI_DIR) -l:$(IMGUI_LIB)

SRCS=$(shell printf "%s " $(SRC_DIR)/*.cpp)
OBJS=$(subst $(SRC_DIR),$(BUILD_DIR),$(subst .cpp,.o,$(SRCS)))

.PHONY: all clean builddir

all: builddir $(TARGET)

builddir:
	$(MKDIR) $(BUILD_DIR)

$(IMGUI_LIB):
	$(MAKE) -C $(IMGUI_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

$(TARGET): $(OBJS) $(IMGUI_LIB)
	$(CXX) $(OBJS) -o $(TARGET) $(CXXFLAGS) $(LDFLAGS)

clean:
	$(RMDIR) $(BUILD_DIR)
	$(MAKE) -s -C $(IMGUI_DIR) clean
