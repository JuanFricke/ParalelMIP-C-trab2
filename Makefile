# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=gnu99 -O3 -march=native -mtune=native -funroll-loops -ffast-math
LIBS = -lcurl -ljson-c
MPI_CC = mpicc
MPI_CFLAGS = -Wall -Wextra -std=gnu99 -O3 -march=native -mtune=native -funroll-loops -ffast-math
MPI_LIBS = -lcurl -ljson-c

# Targets
TARGET = ollama_client
MAIN_TARGET = main

# Sources
SOURCES = ollama_client.c
MAIN_SOURCES = main.c ollama_client.c

# Default target
all: $(TARGET) $(MAIN_TARGET)

# Build the ollama client executable
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LIBS)

# Build main MPI program
$(MAIN_TARGET): $(MAIN_SOURCES)
	export PATH=/usr/lib64/openmpi/bin:$$PATH && $(MPI_CC) $(MPI_CFLAGS) -o $(MAIN_TARGET) $(MAIN_SOURCES) $(MPI_LIBS)

# Clean build artifacts
clean:
	rm -f $(TARGET) $(MAIN_TARGET) *.o

# Install dependencies (Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install -y libcurl4-openssl-dev libjson-c-dev libopenmpi-dev

# Run the main program with 14 processes
run: $(MAIN_TARGET)
	export PATH=/usr/lib64/openmpi/bin:$$PATH && mpirun -np 14 --oversubscribe ./$(MAIN_TARGET)

# Run with 1 process (single thread)
run-single: $(MAIN_TARGET)
	export PATH=/usr/lib64/openmpi/bin:$$PATH && mpirun -np 1 ./$(MAIN_TARGET)

# Quick benchmark
benchmark: $(MAIN_TARGET)
	@echo "🏁 Quick Benchmark: Parallel vs Single Thread"
	@echo "============================================="
	@echo "Testing with 1000 songs..."
	@echo ""
	@echo "🚀 Running parallel (14 processes)..."
	@timeout 60s bash -c 'export PATH=/usr/lib64/openmpi/bin:$$PATH && time mpirun -np 14 --oversubscribe ./$(MAIN_TARGET) > /dev/null 2>&1' || echo "Parallel execution completed or timed out"
	@echo ""
	@echo "🐌 Running single thread (1 process)..."
	@timeout 60s bash -c 'export PATH=/usr/lib64/openmpi/bin:$$PATH && time mpirun -np 1 ./$(MAIN_TARGET) > /dev/null 2>&1' || echo "Single thread execution completed or timed out"
	@echo ""
	@echo "✅ Benchmark completed!"

.PHONY: all clean install-deps run run-single benchmark