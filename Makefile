# Set compiler flags
CFLAGS = -O3 -Wall -std=c++11
DBG_CFLAGS = -g -Wall -std=c++11

# Add external headers
INCLUDE = -Ilib

# Enumerate source files and autogen obj filenames
sources = $(patsubst src/%, %, $(wildcard src/*.cpp))
objects := ${addprefix obj/,${sources:.cpp=.o}}
objects_dbg := ${addprefix obj/,${sources:.cpp=_dbg.o}}

# Targets
target = main
debug = main_dbg

# Set the source/object/bin directories
OBJ_DIR = obj
SRC_DIR = src
BIN_DIR = bin

# Define the main build target
$(target): $(objects)
	$(CXX) $(CFLAGS) $(objects) -o $(BIN_DIR)/$(target)
	@ln -sf $(BIN_DIR)/$(target) ./

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp 
	$(CXX) $(CFLAGS) $(INCLUDE) -c $< -o $@

# Debug build
debug: $(objects_dbg)
	$(CXX) $(DBG_CFLAGS) $(objects_dbg) -o $(BIN_DIR)/$(debug)
	@ln -sf $(BIN_DIR)/$(debug) ./

$(OBJ_DIR)/%_dbg.o: $(SRC_DIR)/%.cpp 
	$(CXX) $(DBG_CFLAGS) $(INCLUDE) -c $< -o $@

# Test build - just an aid for the dev process
test:
	
# Clean!
clean: 
	rm -f $(objects) $(BIN_DIR)/$(target) $(target)
