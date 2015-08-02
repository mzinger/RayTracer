TARGET := raytracer
SOURCES := $(wildcard ./src/CS148/*.cpp) $(wildcard ./src/*.cpp)
HEADERS := $(wildcard ./src/CS148/*.h) $(wildcard ./src/*.h)

CC		 := g++
LD		 := g++
OBJSUFFIX	 := .o
LIBPREFIX	 := lib
STATIC_LIBSUFFIX := .a
CFLAGS 		 := -g -std=c++11 -Wno-deprecated
CFLAGS_PLATFORM  := 
LDFLAGS		 := 
FRAMEWORKS	 := GLUT OpenGL
LIBS		 := GL GLUT m objc stdc++ freeimage

# Mac users need to point to the libpng directories
EXTRA_LIB_DIRS := /opt/local/lib /usr/local/lib
EXTRA_INC_DIRS  := /opt/local/include ./include/ /usr/local/include

INCDIRS    := .  $(EXTRA_INC_DIRS)
LIBDIRS    := $(EXTRA_LIB_DIRS)

CFLAGS     += $(addprefix -I, $(INCDIRS))
CFLAGS	   += $(CFLAGS_PLATFORM)
LDFLAGS    += $(addprefix -L, $(LIBDIRS))
LDLIBS	   := $(addprefix -l, $(LIBS))
LDFRAMEWORKS := $(addprefix -framework , $(FRAMEWORKS))
OBJS       :=  $(addsuffix $(OBJSUFFIX), $(FILES))
LIBPATH += -L"/System/Library/Frameworks/OpenGL.framework/Libraries"

.PHONY:%.h

%.o : %.cpp %.h
	@echo "Compiling .cpp to .o:  $<" 
	@$(LD) -c -Wall $(CFLAGS) $< -o $@

OBJECTS = $(SOURCES:.cpp=.o)

$(TARGET): $(OBJECTS)
	@echo "Linking .o files into:  $(TARGET)"
	@$(LD) $(LDFLAGS) $(LDLIBS) $(LDFRAMEWORKS) $(LIBPATH) $(OBJECTS) -o $(TARGET)

default: $(TARGET) 

all: default

clean:
	@echo "Removing compiled files:  $<" 
	/bin/rm -f $(OBJECTS) $(TARGET)


