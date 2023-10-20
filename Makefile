CXX := g++

SIM_EXECS := addm_simulate
MLE_EXECS := addm_mle 
TEST_EXECS := tutorial custom 

LIB_DIR := lib
OBJ_DIR := obj
INC_DIR := include
BUILD_DIR := bin
SRC_DIR := sample

CXXFLAGS := -Ofast -march=native -fPIC -std=c++17

SHAREDFLAGS = -I $(INC_DIR) -lpthread
LDFLAGS := -shared
LIB := -L lib -lpthread
INC := -I $(INC_DIR)

INSTALL_LIB_DIR := /usr/lib
INSTALL_INC_DIR := /usr/include

PY_SUFFIX := $(shell python3-config --extension-suffix)
PY_INCLUDES := $(shell python3 -m pybind11 --includes)
PY_SO_FILE := $(addsuffix $(PY_SUFFIX), addm_toolbox_cpp)

CPP_FILES := $(filter-out $(LIB_DIR)/bindings.cpp, $(wildcard $(LIB_DIR)/*.cpp))
CPP_OBJ_FILES := $(patsubst $(LIB_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(CPP_FILES))

$(OBJ_DIR): 
	mkdir -p $(OBJ_DIR)

$(BUILD_DIR): 
	mkdir -p $(BUILD_DIR)

$(OBJ_DIR)/%.o: $(LIB_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $(SHAREDFLAGS) -o $@ $<


define compile_target
	$(CXX) $(CXXFLAGS) -c $(addprefix $(SRC_DIR)/, $1.cpp) $(LIB) $(INC) -o $(addprefix $(OBJ_DIR)/, $1.o)
	$(CXX) $(addprefix $(OBJ_DIR)/, $1.o) $(CPP_OBJ_FILES) -o $(addprefix $(BUILD_DIR)/, $1)
endef


sim: $(OBJ_DIR) $(BUILD_DIR) $(CPP_OBJ_FILES) $(CU_OBJ_FILES)
	$(foreach source, $(SIM_EXECS), $(call compile_target, $(source));)

mle: $(OBJ_DIR) $(BUILD_DIR) $(CPP_OBJ_FILES) $(CU_OBJ_FILES)
	$(foreach source, $(MLE_EXECS), $(call compile_target, $(source));)

test: $(OBJ_DIR) $(BUILD_DIR) $(CPP_OBJ_FILES) $(CU_OBJ_FILES)
	$(foreach source, $(TEST_EXECS), $(call compile_target, $(source));)

all: sim mle test


install: $(OBJ_DIR) $(BUILD_DIR) $(CPP_OBJ_FILES) $(CU_OBJ_FILES)
	$(CXX) $(LDFLAGS) -o $(INSTALL_LIB_DIR)/libaddm.so $(CPP_OBJ_FILES)
	cp -TRv $(INC_DIR) $(INSTALL_INC_DIR)/addm


pybind: 
	$(CXX) $(LDFLAGS) $(CXXFLAGS) $(PY_INCLUDES) $(INC) $(CPP_FILES) $(LIB_DIR)/bindings.cpp -o $(PY_SO_FILE)


.PHONY: clean
clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(BUILD_DIR)
	rm -rf docs
