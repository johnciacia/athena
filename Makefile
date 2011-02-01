CC = gcc
CFLAGS = -g -lm -O2 -D_REENTRANT -Wall -D__EXTENSIONS__

# 1. For SUNOS, uncomment the next line
# LIBS = -lresolv -lsocket -lnsl

# 2. For Linux (Anything that's RedHat compatible: RedHat, Mandrake)
# uncomment the next line; I don't guarantee this is going to work with 
# other variations of Linux
LIBS = -lresolv -lnsl -lpthread

# 3. For Mac OS X, don't need LIBS
# LIBS = 

INC   = includes

CLEAN = core *.o *.out typescript* *~ *dSYM

COBJS = network_utils.o

CD    = ${INC}/network_utils.h 

PROGS = server

all: ${PROGS}

server: server.c ${CD} ${COBJS}
	${CC} -I./${INC} ${CFLAGS} ${LIBS} server.c -o $@ ${COBJS}

network_utils.o: ${CD} network_utils.c
	${CC} -I./${INC} ${CFLAGS} -c network_utils.c

clean:
	rm -rf ${PROGS} ${CLEAN}
