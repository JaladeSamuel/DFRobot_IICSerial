# makefile for rc_mpu, builds example programs

# directories
SRCDIR		:= src
BINDIR		:= bin
INCLUDEDIR	:= ../library
LIBDIR		:= ../library

# basic definitions for rules
#EXAMPLES	:= $(shell find $(SRCDIR) -type f -name *.cpp)
#TARGETS		:= $(EXAMPLES:$(SRCDIR)/%.cpp=$(BINDIR)/%)
INCLUDES	:= $(shell find $(INCLUDEDIR) -name '*.h')
#SOURCES		:= $(shell find $(SRCDIR) -type f -name *.cpp)

# compiler and linker programs
CC		:= g++

# compile flags
WFLAGS		:= -Wall -Wextra -Werror=float-equal -Wuninitialized \
	-Wunused-variable -Wdouble-promotion -pedantic \
	-Werror=undef
CFLAGS		:= -g -pthread -I $(INCLUDEDIR)
LDFLAGS		:= -lm -lrt -pthread -L $(LIBDIR) -lDFRobot_IICSerial

# commands
RM		:= rm -rf

all : 
#	$(MAKE) -C ../library all
#	$(TARGETS) 
	@mkdir -p $(BINDIR)
	$(CC) -std=c++11 -I $(INCLUDEDIR) src/read_uart2.cpp $(LDFLAGS) -o bin/read_uart2
	$(CC) -std=c++11 -I $(INCLUDEDIR) src/test_rx_tx.cpp $(LDFLAGS) -o bin/test_rx_tx


clean :
	@$(RM) $(BINDIR)
	@echo "Examples Clean Complete"

