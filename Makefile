ARCH=$(shell uname -s)
TARGET = comtty
SRC = comtty.c
OBJS = _get_key.o com_op.o logging.o filedlg.o config.o commonFunc.o
LIBS = libcomtty.a
ifeq ($(ARCH), cygwin)
SYS_LIBS = /lib/w32api/libcomdlg32.a
else
SYS_LIBS=
endif
CC = gcc
CFLAGS = -Wall -g

all: $(TARGET)
$(TARGET): $(LIBS) $(OBJS) $(SRC)
	$(CC)  $(CFLAGS) $(SRC) $(LIBS) $(SYS_LIBS) -o $(TARGET)
$(LIBS): $(OBJS)
	ar ru $@ $?
.c .o:
	$(CC) $(CFLAGS) -c -o $*.o $*.c
clean:
	rm -f $(LIBS)
	rm -f *.o
	rm -f $(TARGET)
