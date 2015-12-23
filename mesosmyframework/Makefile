# Using g++ compiler
CC = g++

# Standard
STANDARD = -std=c++11

# Include paths
INCLUDE_PATH =  -I./include
INCLUDE_PATH += -I/home/kevin/Documents/mesos/mesos-0.25.0/kevinbuild/include
INCLUDE_PATH += -I/home/kevin/Documents/deps/protobuf-2.5.0/kevinbuild/include
INCLUDE_PATH += -I/home/kevin/Documents/deps/boost/boost_1_60_0
INCLUDE_PATH += -I/home/kevin/Documents/deps/glog-0.3.4/kevinbuild/include
LIBRARY_PATH = -L/home/kevin/Documents/mesos/mesos-0.25.0/kevinbuild/lib

# Optimisation
OPTIMISATION = -O3

# Name of the output
EXECUTABLE = mesos-my-framework
UNAME := $(shell uname)
ifneq ($(UNAME), Linux)
EXECUTABLE := $(EXECUTABLE).exe
endif

# Use bits example -m64
ARCHITECTURE =

# Parameters passed in linking
LFLAGS = $(LIBRARY_PATH) $(ARCHITECTURE) -lmesos

# Parameters passed in compilation
CFLAGS = -Wall $(STANDARD) $(INCLUDE_PATH) $(ARCHITECTURE) $(OPTIMISATION)

# Useful functions
rwildcard = $(wildcard $(1)$(2)) $(foreach d,$(wildcard $(1)*),$(call rwildcard,$(d)/,$(2)))

# Sependent object files - expand immediately
SOURCES := $(call rwildcard,./src/,*.cpp)
OBJECTS := $(patsubst ./src/%.cpp,./obj/%.o,$(SOURCES))
DIRECTORIES := ./obj/ $(patsubst ./src/%,./obj/%,$(filter-out $(call rwildcard,./src/,*.*), $(call rwildcard,./src/,*)))

all: $(EXECUTABLE)

debug: OPTIMISATION =
debug: CFLAGS := -g $(CFLAGS)
debug: LFLAGS := -g $(LFLAGS)
debug: $(EXECUTABLE)

# The executable depends on the objects.. Link Them!
$(EXECUTABLE): $(DIRECTORIES) $(OBJECTS)
	$(CC) $(OBJECTS) $(LFLAGS) -o $@

# Every something.o depends on something.c
# $< is the name of the first dependency
# All source files depend also on respective header (the least)
./obj/%.o: ./src/%.cpp ./include/%.h
	$(CC) $(CFLAGS) -c $< -o $@

# No headers case.. assuming in order
./obj/%.o: ./src/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@
	
# obj directory creation
./obj/:
	mkdir $@

./obj/%:
	mkdir $@

# Always call clean if any header file is changed!
clean:
	rm -rf obj $(EXECUTABLE)
