INCDIRS       = -I/include -I/usr/include/opencv/ -I/opt/local/include/opencv/
LIBDIRS       = -L/lib -L/usr/local/lib -L/opt/local/lib
LIBS          = -lstdc++ -lcv -lcvaux -lcxcore -lhighgui -lm -lpthread --cppflags --cxxflags --ldflags
BINDIR        = ./bin/
BIN           = $(BINDIR)main

CC	          = g++

CXXFLAGS      = -c -Wall -ggdb $(INCDIRS)
CFLAGS        = $(CXXFLAGS)

OBJ           = main.o \
                motion_detection.o 
                

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN) $(LIBDIRS) $(LIBS)


.PHONY: clean
clean:
	rm -f $(OBJ) $(BIN)
