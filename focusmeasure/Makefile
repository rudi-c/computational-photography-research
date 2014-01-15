
LIB	= .
INCLUDE = .

CC	= g++

CPPFLAGS = -I$(INCLUDE) -O3 -Wall $(DEFINEFLAGS)
#CPPFLAGS = -I$(INCLUDE) -g -Wall
#CPPFLAGS = -I$(INCLUDE) -pg -Wall

SRCS  = focusMeasure.cpp \
	imageTools.cpp \
    lodepng.cpp

SRCS_ADDLOWLIGHT = addlowlight.cpp $(SRCS)
OBJS_ADDLOWLIGHT = $(SRCS_APPLY:.cpp=.o) 

SRCS_APPLY = main.cpp $(SRCS)
OBJS_APPLY = $(SRCS_APPLY:.cpp=.o) 

SRCS_CONVOLVE = convolutions.cpp $(SRCS)
OBJS_CONVOLVE = $(SRCS_CONVOLVE:.cpp=.o) 

SRCS_MEDIAN = median.cpp $(SRCS)
OBJS_MEDIAN = $(SRCS_CONVOLVE:.cpp=.o) 

SRCS_RESIZE  = resize.cpp $(SRCS)
OBJS_RESIZE  =	$(SRCS_RESIZE:.cpp=.o) 

all: addlowlight apply convolve median resize

addlowlight: $(OBJS_APPLY)
	$(CC) $(CPPFLAGS) -o addlowlight.exe $(OBJS_ADDLOWLIGHT) -lm

apply: $(OBJS_APPLY)
	$(CC) $(CPPFLAGS) -o apply.exe $(OBJS_APPLY) -lm

convolve: $(OBJS_CONVOLVE)
	$(CC) $(CPPFLAGS) -o convolve.exe $(OBJS_CONVOLVE) -lm

median: $(OBJS_MEDIAN)
	$(CC) $(CPPFLAGS) -o median.exe $(OBJS_MEDIAN) -lm
	
resize: $(OBJS_RESIZE)
	$(CC) $(CPPFLAGS) -o resize.exe $(OBJS_RESIZE) -lm

clean:	;rm -f $(OBJS) $(OBJS_RESIZE) \
	addlowlight.exe*\
	apply.exe* \
	convolve.exe* \
	median.exe* \
	resize.exe* \