# Makefile for a C++ program linked with X11 and Xtst libraries

# Compiler
CXX = g++

# Source files
SRCS = Main.cpp

# Libraries to link
LIBS = -lX11 -lXtst

# Default target
all: rebuild; 

# Clean up object files and the executable
clean:
	rm -f ntapx

# Compilation and linking
ntapx: $(SRCS)
	$(CXX) $^ -o $@ $(LIBS)


# clean & build
rebuild:
	@$(MAKE) clean
	@$(MAKE) ntapx
	echo "SUCCESS!"

