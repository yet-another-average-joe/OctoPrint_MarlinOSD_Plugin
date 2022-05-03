
# file name, no extension
NAME=marlin_osd

# systemd directories
# executable
INSTALL_DIR     = //run/systemd/system
# editable text configuration file
ETC_SERVICE_DIR = //etc/systemd/system



EXEC_NAME = $(NAME).svc
ETC_SERVICE_FILE = $(NAME).service
ETC_SERVICE_PATH = $(ETC_SERVICE_DIR)/$(ETC_SERVICE_FILE)
EXEC_SERVICE_PATH = $(INSTALL_DIR)/$(EXEC_NAME)



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

install :
	make
	echo [Unit] > $(ETC_SERVICE_FILE)
	echo "  Description=Marlin_OSD"            >> $(ETC_SERVICE_FILE)
	echo [Service]                             >> $(ETC_SERVICE_FILE)
	echo "  ExecStart="$(EXEC_SERVICE_PATH)    >> $(ETC_SERVICE_FILE)
	echo "  User=root"                         >> $(ETC_SERVICE_FILE)
	echo [Install]                             >> $(ETC_SERVICE_FILE)
	echo "  WantedBy=multi-user.target"        >> $(ETC_SERVICE_FILE)
	echo >> $(ETC_SERVICE_FILE)
	sudo cp $(EXEC_NAME) $(INSTALL_DIR)
	sudo cp $(ETC_SERVICE_FILE) $(ETC_SERVICE_DIR)
	sudo systemctl enable $(ETC_SERVICE_FILE)
	sudo service $(NAME) start
	rm $(OBJ_FILES) $(EXEC_NAME)
	rm $(ETC_SERVICE_FILE)

uninstall :
	sudo service $(NAME) stop
	sudo systemctl disable $(ETC_SERVICE_FILE)
	sudo rm $(INSTALL_DIR)/$(EXEC_NAME)
	sudo rm $(ETC_SERVICE_PATH)

