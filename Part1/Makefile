# Part1/Makefile

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall
DEBUG = 1
# If DEBUG=1 is passed in the command line, add the -g flag for debugging
ifeq ($(DEBUG),1)
    CXXFLAGS += -g
endif

# Source files (adjust path if needed)
SRCS = src/block.cpp \
       src/blockTree.cpp \
       src/main.cpp \
       src/miner.cpp \
       src/parser.cpp \
       src/sim.cpp \
       src/transaction.cpp \
       src/utils.cpp

# Object files (same names but with .o extension)
OBJS = $(SRCS:.cpp=.o)

# Target executable
TARGET = sim

run: all
	./sim  --total-nodes 5 --slow 0.5 --low-cpu 0.5 --ttx-time 50000 --blk-time 600000 > miners.txt

# Default target: build the executable
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Pattern rule: compile each source file individually
%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Debug build target: simply clean and rebuild using current CXXFLAGS (which may include -g if DEBUG=1)
g: clean $(TARGET)

# Clean up build files
clean:
	rm -f $(TARGET) $(OBJS) *.logs miners.txt