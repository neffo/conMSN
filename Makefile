#CC=icc
CC=gcc

VERSION=`cat VERSION`
BUILDDATE=`date +%Y%m%d`
BUILDTIME=`date +%H%M`
SRC=src/

PVER=v$(VERSION)_$(BUILDDATE)_$(BUILDTIME)

CCOPTS= -DHAVE_STRTOK_R -DDEBUG -DUSE_ERR_LOG \
-DPROG_VERSION=\"$(PVER)\" -g
#CCOPTS= -DHAVESTRTOK_R -DDEBUG -DUSER_ERR_LOG \
	-DPROG_VERSION=\"$(PVER)\" -g -tpp6 -O3
LIBS=-lncurses 

OBJS=	main.o \
	display.o \
	input.o \
	msn_shiz.o \
	md5.o \
	msn_commands.o \
	parse_utils.o \
	libmsn.o \
	chat_lists.o \
	alt_list_code.o \
	file.o \
	autoaway.o
HFILES=display.h input.h


all: console

console: $(OBJS) Makefile
	$(CC) $(CCOPTS) $(OBJS) $(LIBS) -o console

depend:

main.o: src/main.c display.o input.o 
	$(CC) src/main.c -c $(CCOPTS)

display.o: src/display.c 
	$(CC) -c $(CCOPTS) src/display.c

input.o: src/input.c 
	$(CC) -c $(CCOPTS) src/input.c

msn_shiz.o: src/msn_shiz.c
	$(CC) -c $(CCOPTS) src/msn_shiz.c

md5.o: src/md5.c
	$(CC) -c $(CCOPTS) src/md5.c

libmsn.o: src/libmsn.c
	$(CC) -c $(CCOPTS) src/libmsn.c

msn_commands.o: src/msn_commands.c
	$(CC) -c $(CCOPTS) src/msn_commands.c

parse_utils.o: src/parse_utils.c
	$(CC) -c $(CCOPTS) src/parse_utils.c

chat_lists.o: src/chat_lists.c
	$(CC) -c $(CCOPTS) src/chat_lists.c

alt_list_code.o: src/alt_list_code.c
	$(CC) -c $(CCOPTS) src/alt_list_code.c

file.o: src/file.c
	$(CC) -c $(CCOPTS) src/file.c

autoaway.o: src/autoaway.c
	$(CC) -c $(CCOPTS) src/autoaway.c

clean:
	rm -f *.o *~ console

targz: clean
	bash -c "cd ..;tar zcvf conmsn_$(VERSION)_`date +%Y_%m_%d`.tar.gz nefmsn-latest/"
