
LIB	= .
INCLUDE = .

CC	= g++

CPPFLAGS = -I$(INCLUDE) -O3 -Wall $(DEFINEFLAGS)
#CPPFLAGS = -I$(INCLUDE) -g -Wall
#CPPFLAGS = -I$(INCLUDE) -pg -Wall

SRCS  = main.cpp \
	focusMeasure.cpp

OBJS  =	$(SRCS:.cpp=.o)

all:	$(OBJS)
	$(CC) $(CPPFLAGS) -o apply.exe $(OBJS) -lm

clean:	;rm -f $(OBJS) \
	apply \
	apply.exe \
	apply.exe.core \
	apply.exe.stackdump

###
focusMeasure.o: focusMeasure.h
main.o:         focusMeasure.h
