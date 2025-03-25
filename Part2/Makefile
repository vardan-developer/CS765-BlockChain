# Part1/Makefile

# Compiler and flags
CXX = g++ 
CXXFLAGS = -std=c++23
DEBUG = 1
WARNINGS = 0
TOTAL_NODES = 10
TTX_TIME = 5000
BLK_TIME = 600000
BLK_LIMIT = 10
TIME_LIMIT = 10000000
TIMEOUT = 100
ECLIPSE = 1
MALICIOUS = 0.4
ifeq ($(DEBUG),1)
    CXXFLAGS += -g
endif
ifeq ($(WARNINGS),1)
    CXXFLAGS += -Wall
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

# Default target: build the executable
all: $(TARGET)

run: all
	rm -rf logs graphs
	mkdir -p logs
	mkdir -p graphs
	./sim --total-nodes $(TOTAL_NODES) --ttx-time $(TTX_TIME) --blk-time $(BLK_TIME) --blk-limit $(BLK_LIMIT) --timeout $(TIMEOUT)  --eclipse $(ECLIPSE) --malicious $(MALICIOUS)
	dot -Tpng honestGraph.dot -o graphs/honestGraph.png 1> /dev/null 2> /dev/null
	dot -Tpng maliciousGraph.dot -o graphs/maliciousGraph.png 1> /dev/null 2> /dev/null
	rm honestGraph.dot
	rm maliciousGraph.dot
	for file in blockTree-*.dot; do \
		dot -Tpng $$file -o graphs/$${file%.dot}.png 1> /dev/null  2>/dev/null; \
		rm $$file; \
	done

analyze: run
	for file in blockTree-*.dot; do \
		dot -Tpng $$file -o graphs/$${file%.dot}.png 1> /dev/null  2>/dev/null; \
		rm $$file; \
	done

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Pattern rule: compile each source file individually
%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Debug build target: simply clean and rebuild using current CXXFLAGS (which may include -g if DEBUG=1)
g: clean $(TARGET)

# Clean up build files
clean:
	rm -f $(TARGET) $(OBJS) *.logs miners.txt *.dot
	rm -rf logs/ graphs/
