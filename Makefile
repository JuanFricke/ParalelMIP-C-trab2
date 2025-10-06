CC = gcc
CFLAGS = -Wall -Wextra -std=gnu99 -g
LIBS = -lcurl -ljson-c
MPI_CC = mpicc
MPI_CFLAGS = -Wall -Wextra -std=gnu99 -g
MPI_LIBS = -lcurl -ljson-c

# Targets
TARGET = ollama_client
HELPER_TARGET = example_helper
HELPER_LIB = libhelper.a

# Sources
SOURCES = main.c ollama_client.c
HELPER_SOURCES = helper.c
HELPER_OBJECTS = helper.o

# Default target
all: $(TARGET)

# Build the main executable
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LIBS)

# Build helper library
$(HELPER_LIB): $(HELPER_OBJECTS)
	ar rcs $(HELPER_LIB) $(HELPER_OBJECTS)

# Build helper object file
$(HELPER_OBJECTS): $(HELPER_SOURCES)
	$(MPI_CC) $(MPI_CFLAGS) -c $(HELPER_SOURCES) -o $(HELPER_OBJECTS)

# Build example helper program
$(HELPER_TARGET): example_helper.c $(HELPER_LIB)
	$(MPI_CC) $(MPI_CFLAGS) -o $(HELPER_TARGET) example_helper.c -L. -lhelper $(MPI_LIBS)

# Clean build artifacts
clean:
	rm -f $(TARGET) $(HELPER_TARGET) $(HELPER_LIB) *.o

# Install dependencies (Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install -y libcurl4-openssl-dev libjson-c-dev libopenmpi-dev

# Install dependencies (Fedora/RHEL)
install-deps-fedora:
	sudo dnf install -y libcurl-devel json-c-devel openmpi-devel

# Run the program
run: $(TARGET)
	./$(TARGET)

# Run helper example with MPI
run-helper: $(HELPER_TARGET)
	mpirun -np 4 ./$(HELPER_TARGET)

# Debug build
debug: CFLAGS += -DDEBUG -O0
debug: $(TARGET)

# Release build
release: CFLAGS += -O2 -DNDEBUG
release: $(TARGET)

.PHONY: all clean install-deps install-deps-fedora run run-helper debug release
