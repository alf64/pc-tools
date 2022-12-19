# Copyright alf64.
# This Makefile shall be used when building under Linux OS.
# For build call: make all
# For clean call: make clean

# This Makefile builds the app for linux
OS_TYPE=linux
RM = rm -rf

# A name of the target
APP_NAME=SerialBinaryDumper

# a standard gcc is needed to build this app on linux
GCC = gcc

# for fragments of the code that is sensitive to the OS_TYPE.
# Also we need posix standard to be defined to use timespec functions like nanosleep from time.h
# Also we need to define _BSD_SOURCE and _DEFAULT_SOURCE to use com port specifics on linux (like CRTSCTS)
CUSTOM_DEFINES = -D OS_TYPE=$(OS_TYPE) -D_BSD_SOURCE -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=199309L

# Main path for .o files
OBJ_OUTDIR ?= .

# Main path for bin files
BIN_OUTDIR ?= .

# a path where .o files shall be placed
APP_OBJ_OUTDIR = $(OBJ_OUTDIR)/$(APP_NAME)

# a path where the resulting binary file shall be placed
APP_BIN_OUTDIR = $(BIN_OUTDIR)/$(APP_NAME)



SRCS = rs232.cpp \
    main.cpp \
    sbdop.cpp \

OBJS = $(APP_OBJ_OUTDIR)/rs232.o \
    $(APP_OBJ_OUTDIR)/sbdop.o \
    $(APP_OBJ_OUTDIR)/main.o


.PHONY: all clean
all: $(APP_NAME)


$(APP_OBJ_OUTDIR)/%.o: %.cpp
	mkdir -p $(APP_OBJ_OUTDIR)
	@$(GCC) $(CUSTOM_DEFINES) -std=c99 -O3 -Wall -c -o $@ $<

$(APP_NAME): $(OBJS)
	mkdir -p $(APP_BIN_OUTDIR)
	@$(GCC) $(CUSTOM_DEFINES) -std=c99 -O3 -Wall -o $(APP_BIN_OUTDIR)/$@ $^


clean:
	@$(RM) $(OBJS) $(APP_NAME)

