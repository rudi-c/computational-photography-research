
LIB	= .
INCLUDE = .

CC	= g++

CPPFLAGS = -I$(INCLUDE) -O3 -Wall $(DEFINEFLAGS)
#CPPFLAGS = -I$(INCLUDE) -g -Wall
#CPPFLAGS = -I$(INCLUDE) -pg -Wall

SRCS  = focusMeasure.cpp \
	imageTools.cpp \
    lodepng.cpp

SRCS_APPLY = main.cpp $(SRCS)
OBJS_APPLY = $(SRCS_APPLY:.cpp=.o) 

SRCS_CONVOLVE = convolutions.cpp $(SRCS)
OBJS_CONVOLVE = $(SRCS_CONVOLVE:.cpp=.o) 

SRCS_RESIZE  = resize.cpp $(SRCS)
OBJS_RESIZE  =	$(SRCS_RESIZE:.cpp=.o) 

all: apply convolve resize

apply: $(OBJS_APPLY)
	$(CC) $(CPPFLAGS) -o apply.exe $(OBJS_APPLY) -lm

convolve: $(OBJS_CONVOLVE)
	$(CC) $(CPPFLAGS) -o convolve.exe $(OBJS_CONVOLVE) -lm
	
resize: $(OBJS_RESIZE)
	$(CC) $(CPPFLAGS) -o resize.exe $(OBJS_RESIZE) -lm

clean:	;rm -f $(OBJS) $(OBJS_RESIZE) \
	apply \
	apply.exe* \
	resize \
	resize.exe* \