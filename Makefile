CC = gcc
CFLAGS = -Wall -Wextra -std=gnu99 -g
LIBS = -lcurl -ljson-c
TARGET = ollama_client
SOURCES = main.c ollama_client.c

# Default target
all: $(TARGET)

# Build the main executable
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LIBS)

# Clean build artifacts
clean:
	rm -f $(TARGET) *.o

# Install dependencies (Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install -y libcurl4-openssl-dev libjson-c-dev

# Install dependencies (Fedora/RHEL)
install-deps-fedora:
	sudo dnf install -y libcurl-devel json-c-devel

# Run the program
run: $(TARGET)
	./$(TARGET)

# Debug build
debug: CFLAGS += -DDEBUG -O0
debug: $(TARGET)

# Release build
release: CFLAGS += -O2 -DNDEBUG
release: $(TARGET)

.PHONY: all clean install-deps install-deps-fedora run debug release
