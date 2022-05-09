
#
#		MarlinOSD
#			bin*
#			data
#				c_src*
#			static
#			templates
#


# file name, no extension
#NAME=marlin_osd

# executable directory
#INSTALL_DIR = "../../bin"

#EXEC_NAME = marlin_osd

EXEC_NAME = marlin_osd
EXEC_PATH = marlin_osd

# for didpmanx :
INCLUDES += -I/opt/vc/include

 # to user directory

CC = g++

# compiler options
CXXFLAGS += -Wswitch -Wno-deprecated-declarations -Wempty-body -Wconversion -Wreturn-type
CXXFLAGS += -Wparentheses -Wno-pointer-sign -Wno-format -Wuninitialized -Wunreachable-code
CXXFLAGS += -Wunused-function -Wunused-variable -std=c++11 -Wall -fno-strict-aliasing
CXXFLAGS += -g1 "g++" -O3 -fthreadsafe-statics -D NDEBUG 
CXXFLAGS += -frtti -fomit-frame-pointer -std=c11 -fexceptions -o

# linker options
LDFLAGS += -o -Wl,-z,relro -Wl,-z,noexecstack -Wl,--no-undefined g++
LDFLAGS += -lwiringPi -Wl -L/opt/vc/lib -Wl,-z,now 

# files
OBJ_FILES = main.o Demo.o Errors.o EventGrabber.o MarlinBridge.o MarlinWnd.o Settings.o

#libraries
LIBS = -lpthread -lbcm_host -lwiringPi -L/opt/vc/lib 

$(EXEC_NAME) : $(OBJ_FILES)
	$(CC) -o $(EXEC_NAME) $(OBJ_FILES) $(LIBS)

all : $(EXEC_NAME)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

cleanobj :
	make
	chmod +x $(EXEC_NAME)
	mv $(EXEC_NAME) ../../bin/
	rm $(OBJ_FILES)

