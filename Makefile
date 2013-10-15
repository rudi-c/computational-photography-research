
LIB	= .
INCLUDE = .

CC	= g++

CPPFLAGS = -I$(INCLUDE) -O3 -Wall $(DEFINEFLAGS)
#CPPFLAGS = -I$(INCLUDE) -g -Wall
#CPPFLAGS = -I$(INCLUDE) -pg -Wall

SRCS  = main.cpp \
	focusMeasure.cpp \
	imageTools.cpp \

OBJS  =	$(SRCS:.cpp=.o) 

SRCS_RESIZE  = resize.cpp \
	focusMeasure.cpp \
	imageTools.cpp \
    lodepng.cpp \

OBJS_RESIZE  =	$(SRCS_RESIZE:.cpp=.o) 

all: apply resize

apply:	$(OBJS)
	$(CC) $(CPPFLAGS) -g -o apply.exe $(OBJS) -lm
	
resize: $(OBJS_RESIZE)
	$(CC) $(CPPFLAGS) -g -o resize.exe $(OBJS_RESIZE) -lm

clean:	;rm -f $(OBJS) $(OBJS_RESIZE) \
	apply \
	apply.exe \
	apply.exe.core \
	apply.exe.stackdump \
	resize \
	resize.exe \
	resize.exe.core \
	resize.exe.stackdump

###
# focusMeasure.o: focusMeasure.h
# main.o:         focusMeasure.h
