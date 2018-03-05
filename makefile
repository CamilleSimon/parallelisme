CC=mpicc
CFLAGS=-Wall

all: helloworld simple_msg ring pingpong

clean:
	rm -rf helloworld simple_msg ring pingpong
