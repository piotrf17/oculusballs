SRC = balls.cc gl.cc

CC=g++

OBJ = $(SRC:%.cc=%.o)

LIBOVR_BASE ?= ../OculusSDK/LibOVR

INCLUDES ?= -I$(LIBOVR_BASE)

LIBS ?= $(LIBOVR_BASE)/Lib/Linux/Debug/x86_64/libovr.a -ludev -lpthread -lGL -lX11 -lXrandr -lGLEW

FLAGS ?= -std=c++11 -g3

%.o : %.cc
	$(CC) -c $< $(INCLUDES) $(FLAGS)

balls : $(OBJ)
	$(CC) -o $@ $(OBJ) $(LIBS) $(FLAGS)

.PHONY: clean
clean :
	rm -f $(OBJ) balls
