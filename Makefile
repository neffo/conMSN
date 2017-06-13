#CC=icc
CC=gcc

VERSION=`cat VERSION`
BUILDDATE=`date +%Y%m%d`
BUILDTIME=`date +%H%M`

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

main.o: main.c display.o input.o 
	$(CC) main.c -c $(CCOPTS)

display.o: display.c 
	$(CC) -c $(CCOPTS) display.c

input.o: input.c 
	$(CC) -c $(CCOPTS) input.c

msn_shiz.o: msn_shiz.c
	$(CC) -c $(CCOPTS) msn_shiz.c

md5.o: md5.c
	$(CC) -c $(CCOPTS) md5.c

libmsn.o: libmsn.c
	$(CC) -c $(CCOPTS) libmsn.c

msn_commands.o: msn_commands.c
	$(CC) -c $(CCOPTS) msn_commands.c

parse_utils.o: parse_utils.c
	$(CC) -c $(CCOPTS) parse_utils.c

chat_lists.o: chat_lists.c
	$(CC) -c $(CCOPTS) chat_lists.c

alt_list_code.o: alt_list_code.c
	$(CC) -c $(CCOPTS) alt_list_code.c

file.o: file.c
	$(CC) -c $(CCOPTS) file.c

autoaway.o: autoaway.c
	$(CC) -c $(CCOPTS) autoaway.c

clean:
	rm -f *.o *~ console

targz: clean
	bash -c "cd ..;tar zcvf nefmsn_$(VERSION)_`date +%Y_%m_%d`.tar.gz nefmsn-latest/"
