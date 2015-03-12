# ===== Set compiler flags =====
PROF_FLAGS = -O3 -pg -std=c++11 
CFLAGS = -O3 -std=c++11 
DBG_CFLAGS = -g -Wall -std=c++11

# ===== Add external headers =====
INCLUDE = -Ilib

# ===== Enumerate source files and autogen obj filenames =====
sources = $(patsubst src/%, %, $(wildcard src/*.cpp))
objects := ${addprefix obj/,${sources:.cpp=.o}}
objects_dbg := ${addprefix obj/,${sources:.cpp=_dbg.o}}
objects_prof := ${addprefix obj/,${sources:.cpp=_prof.o}}

# ===== Targets =====
target = main
debug = main_dbg
prof = main_prof 

# ===== Set the source/object/bin directories =====
OBJ_DIR = obj
SRC_DIR = src
BIN_DIR = bin

# ===== Define the main build target =====
$(target): $(objects)
	$(CXX) $(CFLAGS) $(objects) -o $(BIN_DIR)/$(target)
	@ln -sf $(BIN_DIR)/$(target) ./

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp 
	$(CXX) $(CFLAGS) $(INCLUDE) -c $< -o $@

# ===== Profiler build =====
prof: $(objects_prof)
	$(CXX) $(PROF_FLAGS) $(objects_prof) -o $(BIN_DIR)/$(prof)
	@ln -sf $(BIN_DIR)/$(prof) ./

$(OBJ_DIR)/%_prof.o: $(SRC_DIR)/%.cpp 
	$(CXX) $(PROF_FLAGS) $(INCLUDE) -c $< -o $@

# ===== Debug build =====
debug: $(objects_dbg)
	$(CXX) $(DBG_CFLAGS) $(objects_dbg) -o $(BIN_DIR)/$(debug)
	@ln -sf $(BIN_DIR)/$(debug) ./

$(OBJ_DIR)/%_dbg.o: $(SRC_DIR)/%.cpp 
	$(CXX) $(DBG_CFLAGS) $(INCLUDE) -c $< -o $@

# ===== Clean! =====
clean: 
	rm -f $(objects) $(BIN_DIR)/$(target) $(target)
	rm -f $(objects_dbg) $(BIN_DIR)/$(debug) $(debug)

