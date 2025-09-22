# Makefile for Parallel Spotify Data Processing with MPI
# Compiler and flags
CC = /usr/lib64/mpich/bin/mpicc
MPIRUN = /usr/lib64/mpich/bin/mpirun
CFLAGS = -Wall -Wextra -O2 -std=c99
LDFLAGS = -lm

# Target executable
TARGET = spotify_processor

# Source files
SOURCES = main.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Default target - build and run with 4 processes
all: $(TARGET)
	@echo "Building and running parallel Spotify data processing..."
	$(MPIRUN) -np 4 ./$(TARGET)

# Build the main executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Build only (don't run)
build: $(TARGET)

# Compile source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)

# Run with different number of processes
run-1: $(TARGET)
	$(MPIRUN) -np 1 ./$(TARGET)

run-2: $(TARGET)
	$(MPIRUN) -np 2 ./$(TARGET)

run-4: $(TARGET)
	$(MPIRUN) -np 4 ./$(TARGET)

run-8: $(TARGET)
	$(MPIRUN) -np 8 ./$(TARGET)

# Run with custom number of processes
run: $(TARGET)
	@echo "Usage: make run NP=<number_of_processes>"
	@echo "Example: make run NP=4"
	@if [ -z "$(NP)" ]; then echo "Please specify NP=<number_of_processes>"; exit 1; fi
	$(MPIRUN) -np $(NP) ./$(TARGET)

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

# Performance profiling build
profile: CFLAGS += -pg -O0
profile: $(TARGET)

# Install dependencies (Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install -y mpich libmpich-dev

# Install dependencies (CentOS/RHEL/Fedora)
install-deps-fedora:
	sudo dnf install -y mpich mpich-devel

# Check if MPI is available
check-mpi:
	@test -f $(CC) && echo "MPI compiler found: $(CC)" || echo "MPI compiler not found. Please install MPI."
	@test -f $(MPIRUN) && echo "MPI runtime found: $(MPIRUN)" || echo "MPI runtime not found. Please install MPI."

# Help target
help:
	@echo "Available targets:"
	@echo "  all          - Build and run with 4 processes (default)"
	@echo "  build        - Build the executable only (don't run)"
	@echo "  clean        - Remove build artifacts"
	@echo "  run-1        - Run with 1 process"
	@echo "  run-2        - Run with 2 processes"
	@echo "  run-4        - Run with 4 processes"
	@echo "  run-8        - Run with 8 processes"
	@echo "  run NP=n     - Run with n processes"
	@echo "  debug        - Build with debug symbols"
	@echo "  profile      - Build with profiling support"
	@echo "  install-deps - Install MPI dependencies (Ubuntu/Debian)"
	@echo "  install-deps-fedora - Install MPI dependencies (Fedora/CentOS)"
	@echo "  check-mpi    - Check if MPI is properly installed"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Quick start:"
	@echo "  make         - Build and run the application"
	@echo "  make build   - Just build (don't run)"
	@echo "  make run-8   - Run with 8 processes"

.PHONY: all build clean run-1 run-2 run-4 run-8 run debug profile install-deps install-deps-fedora check-mpi help
