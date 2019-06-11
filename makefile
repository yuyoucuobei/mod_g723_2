
BASE=../../../..

TARGET = mod_g723_1.so

LIBS = -L./libs -lpthread -lrt -lm
INCLUDE = -I./g723 -I${BASE}/src/include -I${BASE}/libs/libteletone/src

CC = gcc
CPP = gcc

CFLAGS = -g -Wall -O0 -D__alpha -fPIC -shared
CPPFLAGS = -g -Wall -O0 -fPIC -shared

#########################################################
#DO NOT NEED MODIFY BELOW
#########################################################

MAKEDIR = $(PWD)

TMPLIST = $(shell find $(MAKEDIR) -name "*.h" -or -name "*.hpp")
TMPDIR = $(dir $(TMPLIST))
INCLIST = $(sort $(TMPDIR))
INCLUDE += $(foreach temp, $(INCLIST), -I$(temp))

CSRCLIST = $(shell find $(MAKEDIR) -name '*.c')
COBJS = $(CSRCLIST:%.c=%.o)

CPPSRCLIST = $(shell find $(MAKEDIR) -name '*.cpp')
CPPOBJS = $(CPPSRCLIST:%.cpp=%.o)

OBJS = $(COBJS) ${CPPOBJS}

all: $(TARGET)

$(COBJS): %.o: %.c 
	 @echo "##########COMPILE $<##########"
	 $(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@
	 @echo
	 
$(CPPOBJS): %.o: %.cpp 
	 @echo "##########COMPILE $<##########"
	 $(CPP) $(CPPFLAGS) $(INCLUDE) -c $< -o $@
	 @echo

$(TARGET): $(OBJS)
	 @echo "##########LINK $@##########"
	 $(CPP) $(CPPFLAGS) $(LIBS) $(INCLUDE) $(OBJS) -o $@ 
	 @echo
	 @echo "##########COMPILE OVER##########"
	 @echo

clean:
	 rm -f $(OBJS)
	 rm -f $(TARGET)

.PHONY: clean

install:
	 make
	 @echo "##########MOVE $(TARGET)##########"
	 /bin/cp -f $(TARGET) /usr/local/freeswitch/mod
	 @echo
	 @echo "##########INSTALL OVER##########"
	 @echo




