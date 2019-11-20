CC=gcc
CFLAGS = -Wall -pthread

objects = main.c

LuckyServer: $(objects)
	$(CC) -o $@ $(CFLAGS) $(objects)	\
	-I"/home/juli/eclipse-workspace/SRI/" \
	-L"/home/juli/eclipse-workspace/SRI/Debug/" \
	-l"SRI"
clean:
	rm LuckyServer
