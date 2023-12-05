# Set the project home directory to the current directory
MY_PROJECT_ROOT = $(shell pwd)

# Include the headers in your project's include folder
INCLUDES = $(wildcard interface/*.h)

# The include path is your own include directory
INCLUDE_PATH = -Iinterface

# Set your library name here
LIBNAME = HGCALRawDataUnpackingTools

LIBRARY = lib/lib${LIBNAME}.so
LIBRARY_SOURCES = $(wildcard src/*.cc)
LIBRARY_OBJECT_FILES = $(patsubst src/%.cc,obj/%.o,${LIBRARY_SOURCES})

LIBRARY_PATH = -L${MY_PROJECT_ROOT}/lib 

LIBRARIES=
EXECUTABLE_SOURCES = $(wildcard test/*.cpp)
CPP_FLAGS = -O2 -Wall -fPIC -g ${INCLUDE_PATH}  #-DDEBUG
ifdef ROOTSYS
	INCLUDE_PATH := ${INCLUDE_PATH} -I$(ROOTSYS)/include
	ROOTLIBS=`root-config --glibs`
	LIBRARIES = ${ROOTLIBS}
	CPP_FLAGS := ${CPP_FLAGS} `root-config --cflags`
else
	EXECUTABLE_SOURCES := $(filter-out test/packer_fromtb.cpp, $(EXECUTABLE_SOURCES))
endif


EXECUTABLE_OBJECT_FILES = $(patsubst test/%.cpp,obj/%.o,${EXECUTABLE_SOURCES})
EXECUTABLES = $(patsubst test/%.cpp,bin/%,${EXECUTABLE_SOURCES})


LINK_LIBRARY_FLAGS = -shared -fPIC -Wall -O2 ${LIBRARY_PATH} ${LIBRARIES}
LINK_EXECUTABLE_FLAGS = -Wall -g -O2 ${LIBRARIES} 

.PHONY: all _all build _buildall clean _cleanall

default: build

clean: _cleanall
_cleanall:
	rm -rf bin
	rm -rf obj
	rm -rf lib
 
all: _all
build: _all
buildall: _all

_all: obj bin lib ${LIBRARY} ${EXECUTABLES}

bin:
	mkdir -p bin

obj:
	mkdir -p obj

lib:
	mkdir -p lib

${EXECUTABLES}: bin/%: obj/%.o ${EXECUTABLE_OBJECT_FILES}
	g++ ${LINK_EXECUTABLE_FLAGS}  $< ${LIBRARY} -o $@

${EXECUTABLE_OBJECT_FILES}: obj/%.o : test/%.cpp
	g++ -c ${CPP_FLAGS}  $< -o $@

${LIBRARY}: ${LIBRARY_OBJECT_FILES}
	g++ ${LINK_LIBRARY_FLAGS} ${LIBRARY_OBJECT_FILES} -o $@

${LIBRARY_OBJECT_FILES}: obj/%.o : src/%.cc 
	g++ -c ${CPP_FLAGS} $< -o $@
