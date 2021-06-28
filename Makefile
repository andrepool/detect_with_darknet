# Copyright 2021 Andre Pool
# SPDX-License-Identifier: Apache-2.0

TARGET = detect_with_darknet
OBJECTS = $(TARGET).o

MAKEFLAGS = -j8

CFLAGS = -g3 -Ofast -m64 -ansi -std=c++11 -Wall -Wextra -Wpedantic -Werror -Wfatal-errors

LDFLAGS = -std=c++11
LDFLAGS+= -lcuda -lcudart -lcublas -lcurand -lcudnn
LDFLAGS+= -L/usr/local/cuda/lib64

LDLIBS = -lstdc++ -lrt
LDLIBS+= -ldarknet
LDLIBS+= -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs

INC = -I/usr/include/opencv4 -I/usr/local/cuda/include

all: $(TARGET)
	@nice -n 15 ./$(TARGET)

%.o: %.cpp Makefile
	g++ $(CFLAGS) $(INC) -o $@ -c $<

$(TARGET): $(OBJECTS) 
	g++ $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	@rm -f *.o $(TARGET)

.PHONY: all clean

.SECONDARY:


