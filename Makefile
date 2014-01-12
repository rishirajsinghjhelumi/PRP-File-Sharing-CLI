CC = g++
#CFLAGS = -Wall
PROG = PRP 

SRCS = main.c
LIBS = -lm -lcrypto

all: $(PROG)

$(PROG):	$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)

clean:
	rm -f $(PROG)
