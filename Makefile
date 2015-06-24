EXE = comtty.exe
SRC = comtty.c
OBJS = _get_key.o com_op.o pipe_op.o logging.o filedlg.o config.o commonFunc.o
LIBS = libcomtty.a
SYS_LIBS = /lib/w32api/libcomdlg32.a
CC = gcc
CFLAGS = -Wall

all: $(EXE)
$(EXE): $(LIBS) $(OBJS)
	$(CC)  $(SRC) $(LIBS) $(SYS_LIBS) -o $(EXE)
$(LIBS) : $(OBJS)
	ar ru $@ $?
.c .o:
	$(CC) $(CFLAGS) -c -o $*.o $*.c
clean:
	rm -rf $(LIBS)
	rm -rf *.o
	rm -rf *.exe
