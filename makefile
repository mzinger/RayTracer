#
# THIS IS THE DEFAULT MAKEFILE FOR CS184
# Author: njoubert
#

#Code   -------------------------------

### YOU CAN CHANGE THESE VARIABLES AS YOU ADD CODE:
TARGET := raytracer
SOURCES := $(wildcard ./src/CS148/*.cpp) $(wildcard ./src/*.cpp)

#Libraries -------------------------------

#Check for OSX. Works on 10.5 (and 10.4 but untested)
#This line still kicks out an annoying error on Solaris.
ifeq ($(shell sw_vers 2>/dev/null | grep Mac | awk '{ print $$2}'),Mac)
	#Assume Mac
	INCLUDE := -I./include/ -I/opt/local/include 
	LIBRARY := -L./lib/ -lm -lstdc++ -L/opt/local/lib -lfreeimage
	FRAMEWORK :=
	MACROS := -DOSX
	PLATFORM := Mac
else
	#Assume X11
	INCLUDE := -I./include/
	LIBRARY := -L./lib/
	FRAMEWORK := 
	MACROS := 
	PLATFORM := *Nix
endif

#Basic Stuff -----------------------------

CC := gcc
CXX := g++
CXXFLAGS := -g -Wall -O3 -fmessage-length=0 $(INCLUDE) $(MACROS)
LDFLAGS := $(FRAMEWORK) $(LIBRARY)
#-----------------------------------------
%.o : %.cpp
	@echo "Compiling .cpp to .o for $(PLATFORM) Platform:  $<" 
	@$(CXX) -c -Wall $(CXXFLAGS) $< -o $@

OBJECTS = $(SOURCES:.cpp=.o)

$(TARGET): $(OBJECTS)
	@echo "Linking .o files into:  $(TARGET)"
	@$(CXX) $(LDFLAGS) $(OBJECTS) -o $(TARGET)

default: $(TARGET) 

all: default

clean:
	@echo "Removing compiled files:  $<" 
	/bin/rm -f $(OBJECTS) $(TARGET)


