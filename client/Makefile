CC=g++ -std=c++11
OBJ = happychat_client
all:
	$(CC)  -o  $(OBJ)  -cpp   ./*.cpp    `pkg-config  --libs --cflags gtk+-2.0` -pthread -g
#	$(CC)  -o $(OBJ)  ./*.o -I `pkg-config --libs --cflags gtk+-2.0` -pthread -g
#	rm -rf *.o

clean:
	-rm -rf *.o
